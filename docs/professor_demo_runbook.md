# Professor Demo Runbook

Goal: show that the Raspberry Pi and ESP32 can publish and subscribe to ROS 2 messages over a UART micro-ROS link.

## What To Say

This is a minimal proof before running the robot firmware:

```text
The Pi publishes an incrementing integer on /pi_count.
The ESP32 receives it and publishes the same value back on /esp32_ack.
When TX and RX match, the ROS 2 link is bidirectional.
```

## What To Wire

Use 3 jumper wires:

```text
Pi physical pin 8  / GPIO14 / TXD -> ESP32 RX0 / GPIO3
Pi physical pin 10 / GPIO15 / RXD <- ESP32 TX0 / GPIO1
Pi GND                             -> ESP32 GND
```

The ESP32 can be powered by laptop USB during the demo.

Do not connect Pi 5 V to ESP32 UART pins.

## What To Open

Open two SSH terminals to the Pi:

```powershell
ssh audix@172.20.10.3
```

Use:

```text
Terminal 1: micro_ros_agent
Terminal 2: topic checks and Pi Python node
```

## What To Run

Terminal 1:

```bash
cd ~/microros_ws
source /opt/ros/jazzy/setup.bash
source install/local_setup.bash

ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyAMA0 -b 115200 -v6
```

Press ESP32 reset once.

Show that the agent log contains:

```text
session established
publisher created
subscriber created
datawriter created
datareader created
```

Terminal 2:

```bash
source /opt/ros/jazzy/setup.bash
source ~/microros_ws/install/local_setup.bash

ros2 topic list
ros2 topic info /esp32_ack
ros2 topic info /pi_count
```

Point out:

```text
/esp32_ack has Publisher count: 1
/pi_count has Subscription count: 1
```

Then run:

```bash
cd ~/boudy/esp32-pi-microros-smoke-test
source /opt/ros/jazzy/setup.bash
/usr/bin/python3 pi_microros_pubsub_node.py
```

Expected screen:

```text
RX /esp32_ack: -1
TX /pi_count: 1
RX /esp32_ack: 1
TX /pi_count: 2
RX /esp32_ack: 2
```

## How To Explain The Result

- `TX /pi_count` is the Pi publishing a ROS 2 message.
- `RX /esp32_ack` is the Pi receiving a ROS 2 message published by the ESP32.
- Matching numbers prove that both directions work:
  - Pi to ESP32 subscription
  - ESP32 to Pi publication
- The first `-1` is normal because the ESP32 publishes a default value before the first Pi message arrives.

## If Something Fails Live

Check in this order:

1. ESP32 reset was pressed after the agent started.
2. TX/RX are crossed, not straight-through.
3. Pi UART is `/dev/ttyAMA0`.
4. PlatformIO Serial Monitor is closed.
5. Agent log shows session activity.
