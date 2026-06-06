# Setup


## General

You should clone repo with the `--recursive` flag to ensure that all submodules are also cloned.
If you have already cloned the repo without the `--recursive` flag, you can initialize and update the submodules with the following command:

```bash
git submodule update --init --recursive
```

**Python** is generally required for various scripts and tools in the codebase. It is recommended to use *Python 3.10* or later. You can download Python from the official website: https://www.python.org/downloads/.


## DataLink

> **Design doc:** for the schema format, code generator, and wire framing, see **[datalink.md](datalink.md)**.

DataLink is a lightweight protocol used by the whole project to communicate between different components, e.g. the flight firmware and the ground station app. It has generic schemes under the `datalink/schemas` directory in XML format. It has a custom generator script that generates code for different languages. To generate code you should use the script `datalink/gen.py` with specified language (--lang) and output directory (--outdir). For example, to generate C/C++ code for the flight firmware, you can run the following command:

```bash
python datalink/gen.py --lang c --outdir .
```

But all components have this integrated into their build process, so you don't need to run it manually unless you are developing new features for the datalink.

DataLink also has a a few test suites for different languages, which can be found in the `datalink/tests` directory. You can run these tests to ensure that the generated code is correct and that the protocol is working as expected.

To run those tests, first download required dependencies.

- For Python tests, you can install the required dependencies with the following command:
```bash
python -m pip install -r datalink/tests/python/requirements.txt
```

- For C# tests you need to have .NET SDK installed, which you can download from the official website: https://dotnet.microsoft.com/download.
- For C/C++ you need to have GCC installed.

Then you can run all tests with the following command:

```bash
python datalink/run_tests.py
```


## Firmware

> **No heap allocation.** The firmware forbids dynamic memory allocation (`malloc`, `new`, STL containers, exceptions) across all targets. All memory must be statically allocated at compile time. The `make audit` target enforces this automatically.

All firmware targets are built from the `firmware/` directory using `make`. The repository contains three hardware targets and their SITL counterparts:

| Target | Hardware | Chip |
|---|---|---|
| OBC (On-Board Computer) | Flight computer | RP2040 (Raspberry Pi Pico) |
| Radio Module | Telemetry radio | RP2040 (Raspberry Pi Pico) |
| GCS (Ground Control Station) | Ground station | ESP32 |

---

### Toolchain Setup

#### Pico (OBC & Radio Module) — arm-none-eabi

The OBC and Radio Module targets require the ARM GNU Embedded Toolchain, CMake, and Ninja.

**Windows**

Download the MSI installer directly from [developer.arm.com](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and install manually. The installer can optionally add the toolchain to `PATH` — enable this during setup.

**Linux (Ubuntu / Debian)**

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi cmake ninja-build
```

---

#### GCS — ESP-IDF

The GCS target requires ESP-IDF v6.0. Download and install it from the [Espressif ESP-IDF Tools page](https://dl.espressif.com/dl/esp-idf/). The installer handles the compiler, CMake, Python environment, and all dependencies automatically.

After installation, activate the environment in each new terminal before building. On Windows this is done via the *ESP-IDF Command Prompt* shortcut installed by the wizard. On Linux, source the activation script:

```bash
. $HOME/esp/esp-idf/export.sh
```

Once the environment is active, `idf.py` is available in that shell session and `make gcs` / `make gcs_flash` will work.

---

### Building

All targets are driven by `make` from inside the `firmware/` directory.

```bash
cd firmware/
```

| Command | Description |
|---|---|
| `make obc` | Build OBC flight firmware (.uf2 for Pico drag-drop flashing) |
| `make obc_sitl` | Build OBC software-in-the-loop simulation binary |
| `make radio_module` | Build Radio Module flight firmware |
| `make radio_module_sitl` | Build Radio Module SITL binary |
| `make gcs` | Build GCS firmware (requires ESP-IDF environment active) |
| `make gcs_flash PORT=/dev/ttyUSB0` | Build and flash GCS to a connected ESP32 |
| `make test` | Build and run all unit tests |
| `make audit` | Run static audit detecting unsafe constructs (malloc, new, STL, exceptions) |
| `make clean` | Remove all build artifacts |

Optional build variables can be appended to any target:

```bash
make obc BUILD_TYPE=Debug LOG_LEVEL=DEBUG
```

| Variable | Values | Default |
|---|---|---|
| `BUILD_TYPE` | `Release`, `Debug` | `Release` |
| `LOG_LEVEL` | `OFF`, `ERROR`, `WARN`, `INFO`, `DEBUG` | `INFO` |

**Flashing OBC / Radio Module:** after a successful build, `firmware/build/obc/firmware.uf2` (or `radio_module`) is produced. Hold the BOOTSEL button on the Pico while connecting USB, then copy the `.uf2` file to the mass-storage drive that appears.

**Flashing GCS:** the `make gcs_flash` target invokes `idf.py flash` internally. Ensure the ESP-IDF environment is activated and the correct `PORT` is supplied.

---

### Running SITL (no hardware)

Software-in-the-loop runs the *real* firmware as a host process that communicates with the Python simulation hub (`sim/hub/`) over TCP sockets. The firmware host binary is the socket **server**, so it must be started **before** the hub.

```bash
# Terminal A: build and run the firmware SITL process
cd firmware
make obc_sitl
make obc_sitl_run

# Terminal B: run the simulation hub against a flight scenario
cd sim/hub
pip install -e .            # one-time setup
make run CONFIG=or_taipan
```

The hub connects, arms the vehicle, streams synthesized sensor data through the firmware, and on completion plots the on-board EKF estimate against ground truth. Available configs live in `sim/hub/src/sim/configs/`; see [Simulations](#simulations) below. The `obc_sitl_freerun` target (or the `SITL_FREERUN=ON` CMake option) builds the same binary in free-run mode, running as fast as possible instead of pacing to real time.

---

### EKF Derivation

> **Theory:** for the mathematical background (state representation, error injection, prediction, and measurement updates), see **[ekf.md](ekf.md)**.

The EKF covariance and fusion equations are derived symbolically using Python/SymPy. The derivation script generates the C source files in `firmware/src/modules/ekf/derivation/generated/` — **do not hand-edit those files**, changes will be overwritten on the next run.

You only need to re-run the derivation when modifying the EKF equations themselves. Install the single dependency:

```bash
pip install sympy
```

Then run the script from the repo root:

```bash
python firmware/src/modules/ekf/derivation/derivation.py
```

This regenerates `covariance_prediction.c` and `fusion.c` in the `generated/` directory.

---

### Architecture

#### Platform Abstraction (HAL / OSAL)

All hardware access is isolated behind two abstraction layers in `firmware/platform/`:

```
firmware/platform/
├── include/
│   ├── hal/      # Hardware Abstraction Layer — driver interfaces
│   └── osal/     # OS Abstraction Layer — task and timing interfaces
├── pico/         # RP2040 implementations (Pico SDK)
├── esp32/        # ESP32 implementations (ESP-IDF)
└── host/         # Linux/host implementations (SITL)
```

**HAL** (`platform/include/hal/`) defines C-callable driver interfaces. Each interface is a header-only contract; the implementation is compiled only for the target being built.

| Driver | Responsibility |
|---|---|
| `gpio_driver.h` | Pin direction, state, pull-ups, external interrupts |
| `uart_driver.h` | UART init, write, FIFO read |
| `i2c_driver.h` | I2C master blocking transfer |
| `spi_driver.h` | SPI master blocking transfer |
| `adc_driver.h` | Analog voltage measurement |
| `pwm_driver.h` | PWM frequency and duty cycle |
| `time_driver.h` | Millisecond/microsecond timestamps, sleep |
| `flash_driver.h` | Flash read, page write, sector erase |
| `ws2812b_driver.h` | RGB LED bit-banging protocol |

**OSAL** (`platform/include/osal/`) provides a minimal, RTOS-backed threading and timing API:

- `task.h` — `create`, `start_scheduler`, `should_run`: create RTOS tasks and start the scheduler.
- `systime.h` — `get_ms`, `delay_ms`, `delay_until`: system time and periodic delay (used by the runner to enforce task rates).

**Board configurations** (`firmware/boards/{obc,gcs,radio_module}/board_config.h`) map physical pin numbers to logical roles (SPI bus, UART, igniter channels, etc.) for each hardware target.

The result is that all module code is fully portable: it calls only HAL/OSAL APIs and never includes platform-specific headers. Swapping a target means selecting a different platform implementation directory at CMake configure time.

---

#### Pub-Sub Message Bus

> **Design doc:** for ring buffers, sequence numbers, the RPC layer, and scheduler integration, see **[pubsub.md](pubsub.md)**.

Modules communicate exclusively through a lock-free publish-subscribe bus (`firmware/src/pubsub/`). There is no direct function-call coupling between modules.

**Topics** are statically declared in `Topics.h` with a type, a name, and a queue depth. The global topic registry is backed by a 32 KB static memory buffer; there is no heap allocation at runtime.

**API overview:**

```cpp
// Publishing
PubSub::Publisher<SensorImuMsg> imuPublisher{PUBSUB_ID(sensors_imu_1)};
imuPublisher.publish(data);

// Subscribing — poll() returns true when new data is available
PubSub::Subscriber<SensorImuMsg> imuSubscriber{PUBSUB_ID(sensors_imu_1)};
if (imuSubscriber.poll()) {
    auto& data = imuSubscriber.get();
}

// pollLatest() skips stale messages and returns the most recent one
imuSubscriber.pollLatest();
```

Each subscriber tracks its own read position as a sequence number against the circular write buffer. A slow subscriber that falls behind gets a warning but does not block the publisher. The bus is thread-safe through `std::atomic` sequence numbers (acquire/release memory ordering) — no mutexes.

An **RPC pattern** (`RPCRequest` / `RPCHandler`) is layered on top for command-response interactions such as arming the igniter or enabling a power rail.

**Key topics** cover the full flight system:

| Domain | Examples |
|---|---|
| Sensor data | `sensors_imu_1`, `sensors_baro_1`, `sensors_gps_1`, `sensors_battery` |
| State estimation | `ekf_state` (orientation, position, velocity), `sm_state` (flight phase) |
| Communication | `uart_rx/tx`, `lora_rx/tx`, `telemetry_tx/rx` |
| Commands (RPC) | `command_arm`, `command_ignite`, `command_set_voltage` |
| Diagnostics | `ign_continuity`, `voltage_state`, `gcs_radio_state` |

---

#### Module System

All application logic lives in `firmware/src/modules/`. Each module is a self-contained C++ class that implements two methods:

```cpp
class ExampleModule {
    void init();  // one-time setup (subscribe/advertise topics, init hardware)
    void run();   // called periodically by the runner
};
```

Modules declare their pub-sub relationships in their constructor by instantiating `Publisher` and `Subscriber` members. They never call other modules directly.

**The runner** (`modules/runner/`) executes modules according to a JSON profile that specifies execution pools, rates, and priorities. A profile is selected at build time via the `RUNNER_PROFILE` variable:

*`obc_flight.json` — nominal flight profile:*

| Pool | Rate | Priority | Modules |
|---|---|---|---|
| `fast` | 500 Hz | high | sensors, ekf, state_machine, ign |
| `com` | 200 Hz | normal | com_serial, com_uart |
| `slow` | 100 Hz | low | commander_obc, database, telemetry, buzzer, led, voltage |

Each pool maps to an RTOS task. Modules in the same pool run sequentially within that task's time slot; pools run concurrently as separate tasks with their configured priorities. The OSAL `delay_until` primitive enforces the fixed rates.

SITL profiles (`obc_sitl.json`, `gcs_sitl.json`) add simulation bridge modules (`sim_bridge`, `sim_uart`, `sim_lora`, `sim_serial`) that replace physical drivers with UDP socket connections to the Python simulation hub.

**Available modules:**

| Module | Description |
|---|---|
| `sensors` | Reads IMU, barometer, GPS, and battery ADC; publishes raw sensor topics |
| `ekf` | Extended Kalman Filter fusing IMU + baro + GPS into orientation, position, and velocity estimates |
| `state_machine` | Flight state machine (standing → armed → accelerating → free\_flight → free\_fall → landed) |
| `ign` | Igniter continuity monitoring and fire command execution |
| `database` | Flight data logging to internal flash storage |
| `telemetry` | Assembles telemetry packets from EKF and sensor data; publishes to radio TX topic |
| `com_uart` | UART framing and forwarding |
| `com_serial` | Serial (USB) framing and forwarding |
| `com_lora` | LoRa radio framing and forwarding via RadioLib |
| `commander_obc` | Handles arm, ignite, and voltage RPC commands on OBC |
| `commander_gcs` | Handles commands on GCS side |
| `commander_rm` | Handles commands on Radio Module |
| `voltage` | Monitors power rails and publishes health state |
| `buzzer` | Audio feedback tied to flight state transitions |
| `led` | Status LED patterns |
| `oled` | GCS OLED display (ESP32 only) |
| `pmu` | Power management unit readout (GCS, ESP32 only) |
| `gps_simple` | Lightweight GPS NMEA parser (GCS) |
| `sim_bridge` / `sim_uart` / `sim_serial` / `sim_lora` | SITL UDP socket bridges replacing physical drivers |

**Adding a new module** involves creating the class in `firmware/src/modules/<name>/`, registering it in the target's runner profile JSON, and adding it to the CMake module list. The build system uses an `add_module()` CMake macro to compile each module as a static library linked into the final firmware binary.


## Apps

Both apps are made in Unity 2021.3.45f2 and can be found in the `apps` directory.

Install Unity Hub and add the Unity version 2021.3.45f2 to your list of Unity versions. Then, open the `apps` directory in Unity Hub and open either the `obc_app` or `telemetry` project.

Unity natively uses C# as its scripting language so you will need Visual Studio with the Unity workload installed to edit the code for the apps. You can download Visual Studio from the official website: https://visualstudio.microsoft.com/downloads/.

### Structure of the apps

- `obc_app` is the on-board computer control application, mainly for downloading data from the internal storage.
- `telemetry` is the ground station application, which receives telemetry data from the rocket and displays it in a user-friendly interface.
- `shared` contains code that is shared between the apps, e.g. datalink utilities.


## Simulations

### Hub
`sim/hub/` contains the main simulation environment, behaving as a central hub, connecting all other simulation components together and facilitating communication between them.

To setup the hub you need to have Python 3.10 or later installed. You can download Python from the official website: https://www.python.org/downloads/.

Then, you can install the required dependencies with the following command (inside the `sim/hub/` directory):

```bash
pip install -e .
```

After that you can run the hub with the following command:

```bash
make run CONFIG=<config_name>
```

Configs are found in the `sim/hub/src/sim/configs/` directory and they specify which components to run and how to connect them together. They are in the format `cfg_<config_name>.py`, so if you want to run the config `cfg_fm2024.py`, you should use `CONFIG=fm2024` in the command above.

### MATLAB Model
`sim/matlab_model/` contains the Simulink model of the rocket, which is used for system-level simulations and testing of control algorithms.

To run the Simulink model, you need to have MATLAB and Simulink installed. You can download MATLAB from the official website: https://www.mathworks.com/products/matlab.html. 

Required toolboxes include *Simulink*, *Aerospace Blockset*, and *Control System Toolbox*.


## Conventions

### C++ (Firmware)

| Construct | Convention | Example |
|---|---|---|
| Classes | PascalCase | `EKFModule`, `BuzzerModule` |
| Private member variables | `m_` prefix, PascalCase | `m_IMUSubscriber`, `m_CurrentTone` |
| Methods | camelCase | `init()`, `run()`, `updateGPSData()` |
| Macros and constants | UPPER\_SNAKE\_CASE | `LORA_BUFFER_SIZE`, `EKF_NUM_STATES` |
| Enums | `enum class`, PascalCase values | `enum class Tone { START, ARM }` |
| Template classes | PascalCase | `Publisher<T>`, `Subscriber<T>` |

### C (HAL / drivers / shared libs)

| Construct | Convention | Example |
|---|---|---|
| Structs (typedef) | snake\_case with `_t` suffix | `vec3_t`, `geo_position_wgs84_t` |
| Functions | snake\_case | `vec3_add()`, `hal_pwm_init_pin()` |
| Constants / macros | UPPER\_SNAKE\_CASE | `HAL_UART_BUFFER_SIZE` |

### Pub-Sub Topics

Topic message structs use PascalCase (`SensorsIMU`, `EKFState`). Topic instance identifiers registered in `Topics.h` use snake\_case and, where applicable, a numeric suffix for multiple instances (`sensors_imu_1`, `lora_tx`). Message fields use camelCase (`gpsFix`, `baroHeight`, `batVolts`).

### Python

Standard PEP 8: functions and variables in `snake_case`, classes in `PascalCase`. No project-specific deviations observed.
