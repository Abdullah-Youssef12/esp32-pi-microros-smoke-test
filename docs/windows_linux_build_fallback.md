# Windows/Linux Build Fallback

The ESP32 firmware is a PlatformIO Arduino project using `micro_ros_platformio`.

On some Windows machines, `micro_ros_platformio` fails while building its Linux-style dev environment:

```text
Build dev micro-ROS environment failed:
'.' is not recognized as an internal or external command
```

If that happens, build the firmware on the Raspberry Pi or another Linux machine, then flash from the laptop.

## Build On Pi

Copy this repo to the Pi, for example:

```powershell
scp -r "C:\path\to\esp32-pi-microros-smoke-test" audix@172.20.10.3:~/boudy/
```

On the Pi:

```bash
sudo apt update
sudo apt install -y python3-venv python3-full python3-pip git

rm -rf ~/.platformio/penv
python3 -m venv ~/.platformio/penv
~/.platformio/penv/bin/python -m pip install -U pip setuptools wheel
~/.platformio/penv/bin/python -m pip install -U platformio

cd ~/boudy/esp32-pi-microros-smoke-test
~/.platformio/penv/bin/pio run
```

Expected output files:

```text
.pio/build/esp32dev/bootloader.bin
.pio/build/esp32dev/partitions.bin
.pio/build/esp32dev/firmware.bin
```

## Copy Binaries To Laptop

PowerShell:

```powershell
mkdir "C:\path\to\esp32-pi-microros-smoke-test\pi_build" -Force

scp audix@172.20.10.3:~/boudy/esp32-pi-microros-smoke-test/.pio/build/esp32dev/bootloader.bin "C:\path\to\esp32-pi-microros-smoke-test\pi_build\bootloader.bin"
scp audix@172.20.10.3:~/boudy/esp32-pi-microros-smoke-test/.pio/build/esp32dev/partitions.bin "C:\path\to\esp32-pi-microros-smoke-test\pi_build\partitions.bin"
scp audix@172.20.10.3:~/boudy/esp32-pi-microros-smoke-test/.pio/build/esp32dev/firmware.bin "C:\path\to\esp32-pi-microros-smoke-test\pi_build\firmware.bin"
```

## Flash From Laptop

Before flashing:

```text
Disconnect Pi TX/RX from ESP32 RX0/TX0
Keep ESP32 USB connected to laptop
```

PowerShell:

```powershell
python -m pip install esptool
python -m esptool --chip esp32 --port COM10 --baud 921600 --before default-reset --after hard-reset write-flash -z 0x1000 "pi_build\bootloader.bin" 0x8000 "pi_build\partitions.bin" 0x10000 "pi_build\firmware.bin"
```
