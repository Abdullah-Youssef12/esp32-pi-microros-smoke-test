# Test Commands

These commands assume:

```text
Pi SSH: audix@172.20.10.3
Pi UART: /dev/ttyAMA0
ESP32 upload port example: COM10
```

Adjust the COM port if `pio device list` shows a different ESP32 port.

## 1. Flash ESP32 From Laptop

Before flashing:

```text
Disconnect Pi TX/RX from ESP32 RX0/TX0
Keep ESP32 USB connected to laptop
Pi GND to ESP32 GND is okay
Close PlatformIO Serial Monitor
```

PowerShell:

```powershell
cd "C:\path\to\esp32-pi-microros-smoke-test"
pio device list
pio run -t upload --upload-port COM10
```

If the board does not enter bootloader:

```text
Hold ESP32 BOOT
Run the upload command
Release BOOT after Connecting... changes to Writing...
```

## 2. Runtime Wiring

After flashing:

```text
Pi TX / GPIO14 / physical pin 8  -> ESP32 RX0 / GPIO3
Pi RX / GPIO15 / physical pin 10 <- ESP32 TX0 / GPIO1
Pi GND                           -> ESP32 GND
```

## 3. Start Agent On Pi

Terminal 1:

```bash
ssh audix@172.20.10.3
```

```bash
pkill -f 'micro_ros_agent.*ttyAMA0' 2>/dev/null || true

cd ~/microros_ws
source /opt/ros/jazzy/setup.bash
source install/local_setup.bash

ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyAMA0 -b 115200 -v6
```

Press the ESP32 reset button once while the agent is running.

Expected agent log should include lines like:

```text
create_client
session established
create_publisher
create_subscriber
create_datawriter
create_datareader
```

## 4. Verify ROS Graph

Terminal 2:

```bash
ssh audix@172.20.10.3
```

```bash
source /opt/ros/jazzy/setup.bash
source ~/microros_ws/install/local_setup.bash

ros2 topic list
ros2 topic info /esp32_ack
ros2 topic info /pi_count
```

Expected:

```text
/esp32_ack
Publisher count: 1

/pi_count
Subscription count: 1
```

## 5. Run Pi Pub/Sub Node

Terminal 2:

```bash
cd ~/boudy/esp32-pi-microros-smoke-test
source /opt/ros/jazzy/setup.bash

/usr/bin/python3 pi_microros_pubsub_node.py
```

Expected:

```text
RX /esp32_ack: -1
TX /pi_count: 1
RX /esp32_ack: 1
TX /pi_count: 2
RX /esp32_ack: 2
```

Stop with `Ctrl+C`.

## 6. Optional Topic Echo

```bash
source /opt/ros/jazzy/setup.bash
source ~/microros_ws/install/local_setup.bash

ros2 topic echo /esp32_ack --once
```

Expected after the Pi node runs:

```text
data: <latest_count>
```

## Windows micro-ROS Build Fallback

If Windows PlatformIO fails with:

```text
Build dev micro-ROS environment failed:
'.' is not recognized as an internal or external command
```

build on Linux/Pi, copy the three `.bin` files back to the laptop, and flash with `esptool`:

```powershell
python -m esptool --chip esp32 --port COM10 --baud 921600 --before default-reset --after hard-reset write-flash -z 0x1000 "pi_build\bootloader.bin" 0x8000 "pi_build\partitions.bin" 0x10000 "pi_build\firmware.bin"
```
