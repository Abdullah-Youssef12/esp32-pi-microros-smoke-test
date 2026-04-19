#!/usr/bin/env python3
"""Pi-side ROS 2 node for the ESP32 micro-ROS pub/sub smoke test."""

import argparse

import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32


class PiMicroRosSmokeNode(Node):
    def __init__(self, period_sec: float):
        super().__init__("pi_microros_smoke_node")
        self.counter = 0
        self.pub = self.create_publisher(Int32, "/pi_count", 10)
        self.sub = self.create_subscription(Int32, "/esp32_ack", self._ack_cb, 10)
        self.timer = self.create_timer(max(0.1, period_sec), self._timer_cb)
        self.get_logger().info("Pi micro-ROS smoke node started.")

    def _timer_cb(self) -> None:
        self.counter += 1
        msg = Int32()
        msg.data = self.counter
        self.pub.publish(msg)
        print(f"TX /pi_count: {msg.data}", flush=True)

    def _ack_cb(self, msg: Int32) -> None:
        print(f"RX /esp32_ack: {msg.data}", flush=True)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Pi micro-ROS pub/sub smoke test node.")
    parser.add_argument("--period-sec", type=float, default=1.0)
    return parser


def main() -> int:
    args = build_parser().parse_args()
    rclpy.init()
    node = PiMicroRosSmokeNode(args.period_sec)
    try:
        rclpy.spin(node)
        return 0
    except KeyboardInterrupt:
        print("\nStopped by operator.", flush=True)
        return 130
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    raise SystemExit(main())
