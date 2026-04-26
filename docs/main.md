# Setup


## General

You should clone repo with the `--recursive` flag to ensure that all submodules are also cloned.
If you have already cloned the repo without the `--recursive` flag, you can initialize and update the submodules with the following command:

```bash
git submodule update --init --recursive
```

**Python** is generally required for various scripts and tools in the codebase. It is recommended to use *Python 3.10* or later. You can download Python from the official website: https://www.python.org/downloads/.


## DataLink

DataLink is a lightweight protocol used by the whole project to communicate between different components, e.g. the flight firmware and the ground station app. It has generic schemes under the `datalink/schemes` directory in XML format. It has a custom generator script that generates code for different languages. To generate code you should use the script `datalink/gen.py` with specified language (--lang) and output directory (--outdir). For example, to generate C/C++ code for the flight firmware, you can run the following command:

```bash
python datalink/gen.py --lang c --outdir .
```

But all components have this integrated into their build process, so you don't need to run it manually unless you are developing new features for the datalink.

DataLink also has a a few test suites for different languages, which can be found in the `datalink/tests` directory. You can run these tests to ensure that the generated code is correct and that the protocol is working as expected.

To run those tests, first download required dependencies.

- For Python tests, you can install the required dependencies with the following command:
```bash
python -m pip install -r tests/python/requirements.txt
```

- For C# tests you need to have .NET SDK installed, which you can download from the official website: https://dotnet.microsoft.com/download.
- For C/C++ you need to have GCC installed.

Then you can run all tests with the following command:

```bash
python datalink/run_tests.py
```


## Firmware


## Apps

Both apps are made in Unity 2021.3.2f1 and can be found in the `apps` directory.

Install Unity Hub and add the Unity version 2021.3.2f1 to your list of Unity versions. Then, open the `apps` directory in Unity Hub and open either the `obc_app` or `telemetry` project.

Unity natively uses C# as its scripting language so you will need Visual Studio with the Unity workload installed to edit the code for the apps. You can download Visual Studio from the official website: https://visualstudio.microsoft.com/downloads/.

### Structure of the apps

- `obc_app` is the on-board computer control application, mainly for downloading data from the internal storage.
- `telemetry` is the ground station application, which receives telemetry data from the rocket and displays it in a user-friendly interface.
- `shared` contains code that is shared between the apps, e.g. datalink utilities.


## Simulations

### Hub
`hub` contains the main simulation environment, behaving as a central hub, connecting all other simulation components together and facilitating communication between them.

To setup the hub you need to have Python 3.10 or later installed. You can download Python from the official website: https://www.python.org/downloads/.

Then, you can install the required dependencies with the following command:

```bash
pip install -e .
```

After that you can run the hub with the following command:

```bash
make run CONFIG=<config_name>
```

Configs are found in the `hub/src/sim/configs` directory and they specify which components to run and how to connect them together. They are in the format `cfg_<config_name>.py`, so if you want to run the config `cfg_fm2024.py`, you should use `CONFIG=fm2024` in the command above.

### MATLAB Model
`matlab_model` contains the Simulink model of the rocket, which is used for system-level simulations and testing of control algorithms.

To run the Simulink model, you need to have MATLAB and Simulink installed. You can download MATLAB from the official website: https://www.mathworks.com/products/matlab.html. 

Required toolboxes include *Simulink*, *Aerospace Blockset*, and *Control System Toolbox*.
