import json
import re


BUS_DECL_RE = re.compile(r'extern\s+const\s+(hal_\w+_bus_t)\s+(g_cfg_\w+)\s*;')


def parse_bus_symbols(hw_info_path):
    if not hw_info_path.exists():
        return {}

    content = hw_info_path.read_text(encoding="utf-8")

    return {name: bus_type for bus_type, name in BUS_DECL_RE.findall(content)}


def find_violations(profile, bus_symbols):
    pools_by_bus = {}

    for loop in profile:
        for module in loop.get("modules", []):
            for arg in module.get("args", []):
                if arg in bus_symbols:
                    pools_by_bus.setdefault(arg, set()).add(loop["name"])

    return [(bus, bus_symbols[bus], sorted(pools)) for bus, pools in pools_by_bus.items() if len(pools) > 1]


def check_board(board_dir):
    profile_path = board_dir / "run.json"
    hw_info_path = board_dir / "include" / "hw_info.h"

    if not profile_path.exists():
        return []

    # Boards with no declared bus symbols (e.g. host SITL targets) have nothing to share.
    bus_symbols = parse_bus_symbols(hw_info_path)
    if not bus_symbols:
        return []

    with open(profile_path, "r") as f:
        profile = json.load(f)

    return [
        f"{board_dir.name}/run.json: bus '{bus}' ({bus_type}) is used by modules in multiple pools ({', '.join(pools)}) "
        "-- each physical bus must be driven from a single RTOS task"
        for bus, bus_type, pools in find_violations(profile, bus_symbols)
    ]
