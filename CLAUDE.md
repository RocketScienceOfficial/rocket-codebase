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
| `make test` | Build and run all firmware tests |
| `make audit` | Run code audit (detects unsafe constructs: malloc, new, STL, exceptions) |
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

**Platform abstraction**: `firmware/platform/` provides HAL and OSAL layers per target (`pico/`, `esp32/`, `host/` for SITL). Board-specific pin/peripheral configs are in `firmware/boards/{obc,radio_module,gcs}/`. Module code only calls HAL/OSAL — never platform-specific headers.

**Module system**: All application logic lives in `firmware/src/modules/`. Each module implements `init()` / `run()` and is registered into an execution pool defined by a JSON profile (e.g. `obc_flight.json`). The runner drives pools as RTOS tasks at fixed rates: `fast` (500 Hz), `com` (200 Hz), `slow` (100 Hz).

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

## CI

GitHub Actions (`.github/workflows/`) run on push/PR to `main`:
- `firmware-build.yml` — builds all firmware targets and runs ctest
- `firmware-audit.yml` — unsafe-construct audit on all pushes
- `datalink-python-tests.yml`, `datalink-c-tests.yml`, `datalink-csharp-tests.yml` — per-language DataLink tests
