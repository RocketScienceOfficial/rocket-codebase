# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

Monorepo for a rocket system stack: embedded firmware (flight computer, radio module, ground station), Unity apps (telemetry and on-board control), a cross-language communication protocol (DataLink), and Python-based simulation.

Clone with `--recursive` to pull all submodules (FreeRTOS-Kernel, pico-sdk, RadioLib, u8g2).

## Project Structure

```
rocket-codebase/
├── apps/
│   ├── obc_app/          # Unity app — OBC data download
│   ├── telemetry/        # Unity app — ground station display
│   └── shared/           # Shared C# / DataLink utilities
├── datalink/
│   ├── schemas/          # XML message definitions
│   ├── templates/        # Code generation templates
│   ├── tests/            # Per-language test suites (c, csharp, python)
│   ├── gen.py            # Code generator
│   └── run_tests.py      # Test runner
├── docs/
│   └── main.md           # Full setup and architecture docs
├── firmware/
│   ├── boards/           # Board pin configs (obc, radio_module, gcs)
│   ├── platform/         # HAL + OSAL per target (pico, esp32, host)
│   ├── src/
│   │   ├── lib/          # Shared libraries (math, geo, drivers, battery)
│   │   ├── modules/      # Application modules (one directory per module)
│   │   └── pubsub/       # Publish-subscribe message bus
│   ├── tools/            # Build scripts and utilities
│   └── Makefile
└── sim/
    ├── hub/              # Python SITL orchestration hub
    └── matlab_model/     # Simulink model
```

## Critical Constraints

**No heap allocation.** The firmware forbids `malloc`, `new`, STL containers, and exceptions across all targets. All memory must be statically allocated. `make audit` enforces this — run it when touching firmware code.

**Do not hand-edit generated files.** `firmware/src/modules/ekf/derivation/generated/` is produced by `derivation.py`. Edit the derivation script, not the output.

## Build Commands

### Firmware (`firmware/`)

All targets are driven by `make` from the `firmware/` directory.

| Command | Description |
|---|---|
| `make obc` | Build OBC flight firmware (RP2040/Pico) |
| `make obc_sitl` | Build OBC software-in-the-loop (host simulation) |
| `make radio_module` | Build radio module firmware |
| `make radio_module_sitl` | Build radio module SITL |
| `make gcs` | Build ground station firmware (ESP32/ESP-IDF) |
| `make gcs_flash PORT=/dev/ttyUSB0` | Flash GCS to device |
| `make test` | Build and run all firmware tests (configures `host` with `BUILD_TESTS=ON`, which activates the `add_sys_test()` CMake helper used across `pubsub`/`lib`/several `modules`; without it those calls are no-ops) |
| `make audit` | Run code audit (detects unsafe constructs: malloc, new, STL, exceptions; also flags any bus (SPI/I2C/UART) referenced by modules in more than one execution pool, since that means two RTOS tasks would drive the same physical bus) |
| `make clean` | Remove build artifacts |

Optional build variables: `BUILD_TYPE=Debug`, `LOG_LEVEL=<level>`, `SITL_FREERUN=ON`.

### DataLink (`datalink/`)

Code generation is integrated into each component's build — run manually only when developing the protocol itself.

```bash
python datalink/gen.py --lang {c|csharp|python} --outdir <output>
python datalink/run_tests.py   # run all language test suites
```

### Simulation (`sim/hub/`)

```bash
pip install -e .          # one-time setup inside sim/hub/
make run CONFIG=fm2024    # run with a named config
```

Config files: `sim/hub/src/sim/configs/cfg_<name>.py`.

## Architecture

### Firmware

Three hardware targets sharing a common codebase:

- **OBC** (RP2040) — flight computer
- **Radio Module** (RP2040) — telemetry radio
- **GCS** (ESP32) — ground control station

**Platform abstraction**: `firmware/platform/` provides HAL and OSAL layers per target (`pico/`, `esp32/`, `host/` for SITL). Each board (`firmware/boards/{obc,radio_module,gcs}[_sitl]/`) supplies concrete pin/bus numbers via `hw_info.h` + `hw_init.c`, selected at CMake configure time via `board_hw.cmake`. Module code only calls HAL/OSAL — never platform-specific headers. Full contract: [docs/hal_boards.md](docs/hal_boards.md).

**Module system**: All application logic lives in `firmware/src/modules/`. Each module implements `init()` / `run()` and is registered into an execution pool (RTOS work queue) defined by the board's `run.json`. Pools are per-board and named for the bus/resource they own (e.g. `wq_spi`, `wq_com`, `wq_slow` on OBC), not a fixed set — each module in a pool has its own configurable rate.

**Pub/Sub message bus**: Modules communicate exclusively through `firmware/src/pubsub/` (topics defined in `Topics.h`). No direct module coupling. Lock-free circular buffers with atomic sequence numbers — no mutexes.

**EKF**: `firmware/src/modules/ekf/derivation/` contains symbolic derivation (Python/SymPy) that generates C files in `generated/`.

### DataLink Protocol

XML schemas in `datalink/schemas/` define messages shared across all components. The generator produces C, C#, and Python implementations. Shared enums and state machine states are in `datalink/schemas/common.xml`.

### Apps (`apps/`)

Unity 2021.3.45f2. Requires Unity Hub + Visual Studio with Unity workload.

- `obc_app` — on-board computer control (data download)
- `telemetry` — ground station display
- `shared/datalink-unity-utils/` — shared DataLink C# utilities

### Simulation (`sim/hub/`)

Python hub connecting firmware SITL processes over sockets. Physics and sensor models in `sim/hub/src/sim/env/`. The MATLAB Simulink model (`sim/matlab_model/`) requires Simulink, Aerospace Blockset, and Control System Toolbox.

## Naming Conventions

| Context | Convention | Example |
|---|---|---|
| C++ classes | PascalCase | `EKFModule`, `BuzzerModule` |
| C++ private members | `m_` prefix + PascalCase | `m_IMUSubscriber`, `m_CurrentTone` |
| C++ methods | camelCase | `init()`, `updateGPSData()` |
| Macros / constants | UPPER\_SNAKE\_CASE | `LORA_BUFFER_SIZE`, `EKF_NUM_STATES` |
| C structs (typedef) | snake\_case + `_t` | `vec3_t`, `geo_position_wgs84_t` |
| C functions | snake\_case | `vec3_add()`, `hal_pwm_init_pin()` |
| PubSub topic structs | PascalCase | `SensorsIMU`, `EKFState` |
| PubSub topic instances | snake\_case | `sensors_imu_1`, `ekf_state` |
| Python | PEP 8 (snake\_case functions, PascalCase classes) | |

**No leading underscores.** Never prefix a function or variable with `_` or `__` (e.g. `_get_ctx`, `__chipModel`). Both are reserved for the implementation by the C/C++ standard (C11 §7.1.3, C++ `[lex.name]`) — using them is technically undefined behavior, and in practice risks silently colliding with a macro defined by a vendor SDK header (pico-sdk, ESP-IDF, FreeRTOS, newlib). Use a plain name instead (`get_ctx`, `chipModel`); mark C helpers `static` and C++ members `private`/`protected` for the same file-local intent.

## CI

GitHub Actions (`.github/workflows/`) run on push/PR to `main`:
- `firmware-build.yml` — builds `obc`, `obc_sitl`, `radio_module`, `radio_module_sitl` and runs `make test` (does **not** build `gcs`; ESP-IDF isn't installed in that CI job)
- `firmware-audit.yml` — unsafe-construct audit on all pushes
- `datalink-python-tests.yml`, `datalink-c-tests.yml`, `datalink-csharp-tests.yml` — per-language DataLink tests
