# ESP32 to Raspberry Pi micro-ROS Pub/Sub Smoke Test

Standalone UART0 smoke test for proving bidirectional ROS 2 communication between a Raspberry Pi and an ESP32 before using the full robot firmware.

This repo does one focused test:

```text
Pi publishes /pi_count      std_msgs/msg/Int32
ESP32 subscribes /pi_count

ESP32 publishes /esp32_ack  std_msgs/msg/Int32
Pi subscribes /esp32_ack
```

When the test passes, the Pi prints matching transmit and receive values:

```text
TX /pi_count: 1
RX /esp32_ack: 1
TX /pi_count: 2
RX /esp32_ack: 2
```

The first ESP32 ACK may be `-1`. That is normal: it means the ESP32 publisher is alive before it has received the first Pi message.

## Hardware

- Raspberry Pi with Ubuntu 24.04 and ROS 2 Jazzy
- ESP32 DevKit
- Jumper wires
- Common ground between Pi and ESP32
- Laptop for ESP32 flashing

## Runtime Wiring

Use 3.3 V UART only.

![UART0 wiring diagram](docs/assets/uart0_wiring_diagram.svg)

```text
Pi TX / GPIO14 / physical pin 8  -> ESP32 RX0 / GPIO3
Pi RX / GPIO15 / physical pin 10 <- ESP32 TX0 / GPIO1
Pi GND                           -> ESP32 GND
```

Do not connect Pi 5 V to ESP32 UART pins.

## Quick Test

Full copy-ready steps are in [docs/test_commands.md](docs/test_commands.md).

On the Pi, terminal 1:

```bash
cd ~/microros_ws
source /opt/ros/jazzy/setup.bash
source install/local_setup.bash

ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyAMA0 -b 115200 -v6
```

Press the ESP32 reset button once.

On the Pi, terminal 2:

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

Then run:

```bash
cd ~/boudy/esp32-pi-microros-smoke-test
source /opt/ros/jazzy/setup.bash
/usr/bin/python3 pi_microros_pubsub_node.py
```

## Known Passed Result

This was the successful output captured during the hardware bring-up:

```text
RX /esp32_ack: -1
TX /pi_count: 1
RX /esp32_ack: 1
TX /pi_count: 2
RX /esp32_ack: 2
TX /pi_count: 3
RX /esp32_ack: 3
TX /pi_count: 4
RX /esp32_ack: 4
TX /pi_count: 5
RX /esp32_ack: 5
TX /pi_count: 6
RX /esp32_ack: 6
TX /pi_count: 7
RX /esp32_ack: 7
TX /pi_count: 8
RX /esp32_ack: 8
TX /pi_count: 9
RX /esp32_ack: 9
TX /pi_count: 10
RX /esp32_ack: 10
TX /pi_count: 11
```

Topic echo sample after the node ran:

```text
data: 11
---
```

## Important UART0 Rule

ESP32 RX0/TX0 are shared with USB upload and serial monitor.

During flashing:

```text
ESP32 USB -> laptop
Pi TX/RX  -> disconnected from ESP32 RX0/TX0
Pi GND    -> ESP32 GND is okay
```

During runtime:

```text
Close PlatformIO Serial Monitor
Reconnect Pi TX/RX to ESP32 RX0/TX0
Start micro_ros_agent first
Press ESP32 reset once
```

## Professor Demo

Use [docs/professor_demo_runbook.md](docs/professor_demo_runbook.md) for the live presentation sequence: what to wire, what terminals to open, what commands to run, and how to explain the result.

## Troubleshooting

See [docs/troubleshooting.md](docs/troubleshooting.md) for fixes to the exact issues hit during bring-up:

- `Package 'micro_ros_agent' not found`
- Python `numpy` missing inside ROS 2
- `/esp32_ack` missing
- `/pi_count` subscription count is zero
- reversed TX/RX
- ESP32 not entering bootloader
- Windows `micro_ros_platformio` build failure
