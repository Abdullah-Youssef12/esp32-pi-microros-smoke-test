# Troubleshooting

## `Package 'micro_ros_agent' not found`

The micro-ROS agent is not installed or its workspace is not sourced.

```bash
source /opt/ros/jazzy/setup.bash
source ~/microros_ws/install/local_setup.bash
ros2 pkg list | grep micro_ros_agent
```

If it does not appear, install or build the agent using [pi_setup.md](pi_setup.md).

## `ModuleNotFoundError: No module named 'numpy'`

Use system Python, not a PlatformIO venv:

```bash
sudo apt install -y python3-numpy python3-dev
/usr/bin/python3 -c "import numpy; print(numpy.__version__)"
```

Run the node with:

```bash
/usr/bin/python3 pi_microros_pubsub_node.py
```

## `/esp32_ack` Is Missing

The ESP32 is not connected to the micro-ROS agent.

Check:

```text
Agent is running before ESP32 reset
ESP32 has this firmware flashed
Pi TX/RX are connected for runtime
TX/RX are crossed correctly
PlatformIO Serial Monitor is closed
```

## `/pi_count` Subscription Count Is Zero

The ESP32 subscriber did not appear. This usually means the ESP32 did not create its micro-ROS entities.

Check the agent log for:

```text
create_client
session established
create_subscriber
create_datareader
```

If these are absent, the ESP32 is not reaching the agent.

## Agent Shows No Activity After ESP32 Reset

Most likely causes:

```text
TX/RX reversed
wrong Pi UART device
ESP32 still running old firmware
USB serial monitor holding UART0
no common ground
```

Correct runtime wiring:

```text
Pi TX pin 8  -> ESP32 RX0/GPIO3
Pi RX pin 10 <- ESP32 TX0/GPIO1
Pi GND       -> ESP32 GND
```

## ESP32 Upload Fails: Wrong Boot Mode

If `esptool` says:

```text
Wrong boot mode detected
The chip needs to be in download mode
```

retry upload manually:

```text
Hold BOOT
Run the upload command
Release BOOT when writing starts
```

Some boards need:

```text
Hold BOOT
Tap RESET
Release RESET
Keep BOOT held until Connecting... changes
Release BOOT
```

## Windows `micro_ros_platformio` Build Fails

Known failure:

```text
Build dev micro-ROS environment failed:
'.' is not recognized as an internal or external command
```

Use the Linux/Pi fallback:

```text
Build on Pi/Linux
Copy bootloader.bin, partitions.bin, firmware.bin back to laptop
Flash with esptool
```

Flash command:

```powershell
python -m esptool --chip esp32 --port COM10 --baud 921600 --before default-reset --after hard-reset write-flash -z 0x1000 "pi_build\bootloader.bin" 0x8000 "pi_build\partitions.bin" 0x10000 "pi_build\firmware.bin"
```

## Pi Node Prints Only TX Lines

Example:

```text
TX /pi_count: 60
TX /pi_count: 61
```

This means the Pi publisher is running, but it is not receiving `/esp32_ack`.

Check:

```bash
ros2 topic info /esp32_ack
```

If unknown, the ESP32 is not connected. If it exists, restart the Pi node.
