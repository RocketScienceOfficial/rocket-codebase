import argparse
import json
import sys
from pathlib import Path


ROOT = Path(__file__).parent.parent.resolve()


def gen_source(profile):
    names_cache = {}

    def get_module_include_name(module):
        if module in names_cache:
            return names_cache[module]

        module_path = ROOT / module

        if not module_path.exists():
            print(f"Module '{module}' does not exist")
            sys.exit(1)

        for file in module_path.iterdir():
            if file.is_file() and file.suffix in ['.h', '.hpp'] and file.stem.endswith("Module"):
                names_cache[module] = file.stem
                return file.stem

        print(f"Module '{module}' has no header file ending with 'Module'")
        sys.exit(1)

    def gen_header(profile):
        s = ""
        s += "#include \"ModulesRunner.h\"\n"
        s += "#include <lib/debug/obc_assert.h>\n"
        s += "#include <osal/task.h>\n"
        s += "#include <osal/systime.h>\n"

        for loop in profile:
            for module in loop["modules"]:
                s += "#include \"modules/{module}/{module_include}.h\"\n".format(module=module, module_include=get_module_include_name(module))

        s += "\n"

        s += "static uint8_t g_stackBuffer[16 * 1024];\n"
        s += "static size_t g_stackBufferOffset = 0;\n"

        s += "\n"

        for loop in profile:
            for module in loop["modules"]:
                s += "static {module_include} {module_include}Instance;\n".format(module_include=get_module_include_name(module))

        s += "\n"

        return s

    def gen_loop(data):
        print(f"Generating loop '{data['name']}'...")

        rateless = "rate" not in data

        s = "static void main_{name}(void *arg)\n{{\n".format(name=data["name"])
        s += "    (void)arg;\n\n"

        for module in data["modules"]:
            s += "    {module_include}Instance.init();\n".format(module_include=get_module_include_name(module))

        if not rateless:
            s += "\n    uint32_t lastWakeTime = osal_systime_get_ms();\n\n"

        s += "    while (osal_task_should_run())\n    {\n"

        for module in data["modules"]:
            s += "        {module_include}Instance.run();\n".format(module_include=get_module_include_name(module))

        if not rateless:
            s += "\n        osal_task_delay_until(&lastWakeTime, {rate});\n".format(rate=int(1000 / data["rate"]))

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

        s += "void ModulesRunner::startAllModules()\n{\n"

        priority_mapping = {
            "high": "OSAL_TASK_PRIORITY_HIGH",
            "normal": "OSAL_TASK_PRIORITY_NORMAL",
            "low": "OSAL_TASK_PRIORITY_LOW",
        }

        for loop in profile:
            s += "    spawnTask(main_{name}, \"{name}\", 4096, {priority});\n".format(name=loop["name"], priority=priority_mapping[loop["priority"]])

        s += "\n    osal_task_start_scheduler();\n"

        s += "}\n"

        return s

    print("Generating ModulesRunner code...")

    total = gen_header(profile)

    for loop in profile:
        total += gen_loop(loop)

    total += gen_spawn(profile)

    return total


def gen_cmake(profile):
    print("Generating CMake code for ModulesRunner...")

    s = ""

    for loop in profile:
        for module in loop["modules"]:
            s += "add_subdirectory(../{module} ${{CMAKE_CURRENT_BINARY_DIR}}/../{module})\n".format(module=module)

    s += "\n"
    s += "target_link_libraries(app_modules_runner PUBLIC\n"

    for loop in profile:
        for module in loop["modules"]:
            s += "    app_modules_{module}\n".format(module=module)

    s += "    platform_osal\n"
    s += ")\n"

    return s


def main():
    parser = argparse.ArgumentParser(description="Generate ModulesRunner code from JSON profile.")
    parser.add_argument("--profile", help="Path to the JSON profile file.")
    parser.add_argument("--output-source", help="Path to the output C++ source file.")
    parser.add_argument("--output-cmake", help="Path to the output C++ CMake file.")
    args = parser.parse_args()

    with open(args.profile, "r") as f:
        profile = json.load(f)

    with open(args.output_source, "w") as f:
        f.write(gen_source(profile))

    with open(args.output_cmake, "w") as f:
        f.write(gen_cmake(profile))


if __name__ == "__main__":
    main()
