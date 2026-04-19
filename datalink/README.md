Python template runtime tests are optional and intended for local development and CI.
They do not affect submodule consumers unless explicitly installed.

Install dev dependencies (only needed for Python tests):

```powershell
python -m pip install -r tests/python/requirements.txt
```

Run all language tests with one command:

```powershell
python run_tests.py
```