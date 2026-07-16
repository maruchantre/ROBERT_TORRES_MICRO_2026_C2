# ESP-IDF Firmware

- This is a single-component ESP-IDF project: `main/main.c` provides the firmware entrypoint `app_main`; register additional application sources in `main/CMakeLists.txt`.
- Use ESP-IDF 5.5.4. Initialize its environment before invoking the project tools (`. $IDF_PATH/export.sh`); the dev container does this from `~/.bashrc`.
- Build from the repository root with `idf.py build`. Flash and view serial output with `idf.py -p <port> flash monitor`.
- The operative target is `esp32`, using a 2 MB DIO flash and 115200 baud monitor. Do not switch targets based only on `hardware/README.md`, which names an ESP32-S3.
- `sdkconfig` and `build/` are generated local artifacts and are ignored. Change configuration through `idf.py menuconfig` (or add a tracked `sdkconfig.defaults` when defaults must be shared), then rebuild.
- `hardware/` contains the KiCad schematic and production material for the KACATA RC433 board; it is separate from the firmware component.
