# Raspberry Pi Setup

Tested target:

```text
Raspberry Pi 5
Ubuntu 24.04
ROS 2 Jazzy
Pi UART device: /dev/ttyAMA0
```

## Enable UART

Disable serial console services that may hold the UART:

```bash
sudo systemctl disable --now serial-getty@serial0.service || true
sudo systemctl disable --now serial-getty@ttyAMA0.service || true
sudo systemctl disable --now serial-getty@ttyS0.service || true
```

Enable UART in the boot config:

```bash
grep -q '^enable_uart=1' /boot/firmware/config.txt || echo 'enable_uart=1' | sudo tee -a /boot/firmware/config.txt
sudo reboot
```

After reboot:

```bash
ls -l /dev/ttyAMA0
```

## micro-ROS Agent

If the package is available through apt:

```bash
source /opt/ros/jazzy/setup.bash
sudo apt update
sudo apt install -y ros-jazzy-micro-ros-agent
```

If apt cannot find the package, build from source:

```bash
source /opt/ros/jazzy/setup.bash

sudo apt update
sudo apt install -y git build-essential cmake python3-colcon-common-extensions python3-rosdep python3-numpy python3-dev

sudo rosdep init 2>/dev/null || true
rosdep update

mkdir -p ~/microros_ws/src
cd ~/microros_ws
git clone -b jazzy https://github.com/micro-ROS/micro_ros_setup.git src/micro_ros_setup

rosdep install --from-paths src --ignore-src -y --rosdistro jazzy
colcon build
source install/local_setup.bash

ros2 run micro_ros_setup create_agent_ws.sh
ros2 run micro_ros_setup build_agent.sh
```

Verify:

```bash
source /opt/ros/jazzy/setup.bash
source ~/microros_ws/install/local_setup.bash
ros2 pkg list | grep micro_ros_agent
```

Expected:

```text
micro_ros_agent
```

## Python NumPy Fix

If ROS Python fails with:

```text
ModuleNotFoundError: No module named 'numpy'
```

install and use system Python:

```bash
sudo apt install -y python3-numpy python3-dev
/usr/bin/python3 -c "import numpy; print(numpy.__version__)"
```

If `python3` points to a PlatformIO venv, clean the terminal:

```bash
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:$HOME/.local/bin"
unset VIRTUAL_ENV
unset PYTHONHOME
unset PYTHONPATH
hash -r
```

Run the Pi smoke node with:

```bash
/usr/bin/python3 pi_microros_pubsub_node.py
```
