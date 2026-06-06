# Image assets

Images referenced by the root [README.md](../../README.md) and the documentation.

| Filename | Shown in | Description |
|---|---|---|
| `telemetry.png` | README (Applications) | Telemetry ground-station Unity app |
| `obc_app.png` | README (Applications) | OBC data-download Unity app |
| `sitl_main_or.png` | README (Simulation) | Trajectory and altitude plot from an OpenRocket SITL run |
| `sitl_errors_or.png` | README (Simulation) | EKF estimate versus ground-truth error plot (OpenRocket SITL) |
| `sitl_plots_or.png` | (spare) | Combined SITL plot panel |
| `attitude_animation_px4.gif` | README (Simulation) | EKF attitude convergence from a wrong initial estimate (PX4 replay) |

## Regenerating the SITL plots

```bash
# Terminal A
cd firmware
make obc_sitl
make obc_sitl_run

# Terminal B
cd sim/hub
pip install -e .
make run CONFIG=or_taipan
```

When the run completes, the hub opens Matplotlib windows (trajectory, per-axis errors, predicted apogee, and a
3-D attitude animation). The PX4 attitude animation comes from the `px4_example` config.

## Tips

- The README sizes app screenshots at about 50 percent width and plots at about 48 percent, so PNGs around
  1200 to 1600 px wide stay crisp without bloating the repo.
- Prefer PNG for UI and plots (sharp text). Optimize large files before committing.
