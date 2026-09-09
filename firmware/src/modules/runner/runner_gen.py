import argparse
import json
import sys
from pathlib import Path


def check_profile(profile_path):
    sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
    from bus_ownership import check_board

    violations = check_board(profile_path.parent)
    if violations:
        for message in violations:
            print(message)
        sys.exit(1)


MAX_RATE_HZ = 1000


# Emitted once per generated runner, above the loop functions.
SCHEDULING_HELPERS = """
// Millisecond timestamps are uint32_t and wrap roughly every 49.7 days, so deadlines are
// compared as signed differences rather than as absolute values. Correct as long as no real
// interval exceeds ~24.8 days.
static inline bool time_reached(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}

static inline bool time_before_or_equal(uint32_t a, uint32_t b)
{
    return (int32_t)(a - b) <= 0;
}

// Advance a module past the tick it just served. If run() overran far enough that the next
// deadline has already passed, drop the missed ticks instead of running back-to-back: catching
// up would pin the pool at 100% CPU and permanently starve every lower-priority pool.
static inline uint32_t schedule_next(uint32_t due, uint32_t period, uint32_t now)
{
    due += period;

    if (time_reached(now, due))
    {
        due = now + period;
    }

    return due;
}

"""


def round_up_pow2(n):
    if n <= 1:
        return 1

    return 1 << (n - 1).bit_length()


def is_power_of_two(n):
    return n > 0 and (n & (n - 1)) == 0


def loop_stack_size(loop):
    MIN_MODULE_STACK_SIZE = 1024

    for module in loop["modules"]:
        if "stack_size" not in module:
            print(f"Module '{module['name']}' in loop '{loop['name']}' is missing 'stack_size'")
            sys.exit(1)

        stack_size = module["stack_size"]

        if stack_size < MIN_MODULE_STACK_SIZE:
            print(f"Module '{module['name']}' in loop '{loop['name']}' has stack_size {stack_size}, below the minimum of {MIN_MODULE_STACK_SIZE}")
            sys.exit(1)

        if not is_power_of_two(stack_size):
            print(f"Module '{module['name']}' in loop '{loop['name']}' has stack_size {stack_size}, which is not a power of two")
            sys.exit(1)

    return round_up_pow2(max(module["stack_size"] for module in loop["modules"]))


def validate_loop_rates(loop):
    rated = [module for module in loop["modules"] if "rate" in module]
    rateless = [module for module in loop["modules"] if "rate" not in module]

    if rated and rateless:
        names = ", ".join(module["name"] for module in rateless)
        print(f"Loop '{loop['name']}' mixes rated and rateless modules ({names} have no 'rate'); a loop must be either fully rated or a single rateless module")
        sys.exit(1)

    if rateless and len(loop["modules"]) > 1:
        print(f"Loop '{loop['name']}' has {len(loop['modules'])} modules but none declare 'rate'; a rateless loop must contain exactly one module")
        sys.exit(1)

    # The scheduler tick is 1 kHz, so a period is an integer number of milliseconds. A rate above
    # 1000 Hz would round to a 0 ms period, which leaves the module permanently due and turns the
    # pool into a busy loop that starves every lower-priority pool.
    for module in rated:
        rate = module["rate"]

        if not isinstance(rate, int) or isinstance(rate, bool) or rate <= 0:
            print(f"Module '{module['name']}' in loop '{loop['name']}' has rate {rate!r}; rate must be a positive integer in Hz")
            sys.exit(1)

        if rate > MAX_RATE_HZ:
            print(f"Module '{module['name']}' in loop '{loop['name']}' has rate {rate} Hz, above the {MAX_RATE_HZ} Hz scheduler tick")
            sys.exit(1)

        if 1000 % rate != 0:
            actual = 1000 / (1000 // rate)
            print(f"Warning: module '{module['name']}' in loop '{loop['name']}' requests {rate} Hz, which is not a divisor of the {MAX_RATE_HZ} Hz tick; it will actually run at {actual:.1f} Hz")


def gen_source(profile):
    names_cache = {}

    for loop in profile:
        validate_loop_rates(loop)

    loop_stack_sizes = {loop["name"]: loop_stack_size(loop) for loop in profile}

    def get_module_include_name(module_name):
        if module_name in names_cache:
            return names_cache[module_name]

        module_path = Path(__file__).parent.parent.resolve() / module_name

        if not module_path.exists():
            print(f"Module '{module_name}' does not exist (path: {module_path})")
            sys.exit(1)

        for file in module_path.iterdir():
            if file.is_file() and file.suffix in ['.h', '.hpp'] and file.stem.endswith("Module"):
                names_cache[module_name] = file.stem
                return file.stem

        print(f"Module '{module_name}' has no header file ending with 'Module'")
        sys.exit(1)

    def gen_header(profile):
        s = ""
        s += "#include <osal/task.h>\n"
        s += "#include <hw_info.h>\n"
        s += "#include <cstdint>\n"

        for loop in profile:
            for module in loop["modules"]:
                s += "#include \"modules/{module}/{module_include}.h\"\n".format(module=module["name"], module_include=get_module_include_name(module["name"]))

        s += "\n"

        for loop in profile:
            s += f"// Loop '{loop['name']}'\n"

        total_stack = sum(loop_stack_sizes[loop["name"]] for loop in profile)
        s += f"alignas(8) static uint8_t g_stackBuffer[{total_stack}]; // Aligned to 64-bit words\n"
        s += "static size_t g_stackBufferOffset = 0;\n"

        s += SCHEDULING_HELPERS

        for loop in profile:
            for module in loop["modules"]:
                module_include = get_module_include_name(module["name"])
                args = ", ".join(module.get("args", []))

                if args:
                    s += "static {mi} {mi}Instance({args});\n".format(mi=module_include, args=args)
                else:
                    s += "static {mi} {mi}Instance;\n".format(mi=module_include)

        s += "\n"

        return s

    def gen_loop(data):
        print(f"Generating loop '{data['name']}'...")

        modules = data["modules"]
        rateless = all("rate" not in module for module in modules)

        s = "static void main_{name}(void *arg)\n{{\n".format(name=data["name"])
        s += "    (void)arg;\n\n"

        for module in modules:
            s += "    {module_include}Instance.init();\n".format(module_include=get_module_include_name(module["name"]))

        if rateless:
            s += "\n    while (osal_task_should_run())\n    {\n"

            for module in modules:
                s += "        {module_include}Instance.run();\n".format(module_include=get_module_include_name(module["name"]))

            s += "    }\n}\n"

            return s

        periods = [int(1000 / module["rate"]) for module in modules]
        n = len(modules)

        s += "\n    const uint32_t startTime = osal_task_get_ms();\n"
        s += "    uint32_t nextDue[{n}] = {{ {values} }};\n".format(n=n, values=", ".join(f"startTime + {p}" for p in periods))
        s += "    const uint32_t period[{n}] = {{ {values} }};\n".format(n=n, values=", ".join(str(p) for p in periods))

        s += "\n    while (osal_task_should_run())\n    {\n"
        s += "        uint32_t soonest = nextDue[0];\n\n"
        s += "        for (size_t i = 1; i < {n}; i++)\n        {{\n".format(n=n)
        s += "            if (time_before_or_equal(nextDue[i], soonest)) soonest = nextDue[i];\n"
        s += "        }\n\n"
        s += "        osal_task_delay_until(soonest);\n"

        for i, module in enumerate(modules):
            module_include = get_module_include_name(module["name"])
            s += "\n        {decl}now = osal_task_get_ms();\n".format(decl="uint32_t " if i == 0 else "")
            s += "        if (time_reached(now, nextDue[{i}]))\n        {{\n".format(i=i)
            s += "            {mi}Instance.run();\n".format(mi=module_include)
            s += "            nextDue[{i}] = schedule_next(nextDue[{i}], period[{i}], osal_task_get_ms());\n".format(i=i)
            s += "        }\n"

        s += "    }\n}\n"

        return s

    def gen_spawn(profile):
        s = """
static void spawnTask(void (*taskFunc)(void *), const char *name, size_t stack_size, osal_task_priority_t priority)
{
    osal_task_create(name, taskFunc, nullptr, g_stackBuffer + g_stackBufferOffset, stack_size, priority);

    g_stackBufferOffset += stack_size;
}

"""

        s += "static void start_tasks()\n{\n"

        priority_mapping = {
            "high": "OSAL_TASK_PRIORITY_HIGH",
            "normal": "OSAL_TASK_PRIORITY_NORMAL",
            "low": "OSAL_TASK_PRIORITY_LOW",
        }

        for loop in profile:
            s += "    spawnTask(main_{name}, \"{name}\", {stack_size}, {priority});\n".format(name=loop["name"], stack_size=loop_stack_sizes[loop["name"]], priority=priority_mapping[loop["priority"]])

        s += "\n    osal_task_start_scheduler();\n"

        s += "}\n"

        return s

    def gen_main():
        s = ""
        s += "\n"
        s += "void core_main()\n{\n"
        s += "    hw_init();\n"
        s += "    start_tasks();\n"
        s += "}\n"

        return s

    print("Generating code...")

    total = gen_header(profile)

    for loop in profile:
        total += gen_loop(loop)

    total += gen_spawn(profile)
    total += gen_main()

    return total


def gen_cmake(profile):
    print("Generating CMake...")

    s = ""

    for loop in profile:
        for module in loop["modules"]:
            s += "add_subdirectory(../{module} ${{CMAKE_CURRENT_BINARY_DIR}}/../{module})\n".format(module=module["name"])

    s += "\n"
    s += "target_link_libraries(app_main PUBLIC\n"

    for loop in profile:
        for module in loop["modules"]:
            s += "    app_modules_{module}\n".format(module=module["name"])

    s += ")\n"

    return s


def main():
    parser = argparse.ArgumentParser(description="Generate code from JSON profile.")
    parser.add_argument("--profile", help="Path to the JSON profile file.")
    parser.add_argument("--output-source", help="Path to the output C++ source file.")
    parser.add_argument("--output-cmake", help="Path to the output C++ CMake file.")
    args = parser.parse_args()

    profile_path = Path(args.profile)

    check_profile(profile_path)

    with open(profile_path, "r") as f:
        profile = json.load(f)

    with open(args.output_source, "w") as f:
        f.write(gen_source(profile))

    with open(args.output_cmake, "w") as f:
        f.write(gen_cmake(profile))


if __name__ == "__main__":
    main()
