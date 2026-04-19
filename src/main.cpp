#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rmw_microros/rmw_microros.h>
#include <std_msgs/msg/int32.h>

namespace {

constexpr uint32_t kSerialBaud = 115200;
constexpr unsigned long kReconnectPeriodMs = 1000;
constexpr unsigned long kAgentPingTimeoutMs = 100;
constexpr uint8_t kAgentPingAttempts = 1;
constexpr uint32_t kAckPeriodMs = 1000;
constexpr uint8_t kExecutorHandles = 2;

struct MicroRosState {
    bool entities_created = false;
    unsigned long last_connect_attempt_ms = 0;

    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support = {};
    rcl_node_t node = rcl_get_zero_initialized_node();
    rcl_publisher_t ack_pub = rcl_get_zero_initialized_publisher();
    rcl_subscription_t pi_count_sub = rcl_get_zero_initialized_subscription();
    rcl_timer_t ack_timer = rcl_get_zero_initialized_timer();
    rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();

    std_msgs__msg__Int32 pi_count_msg;
    std_msgs__msg__Int32 ack_msg;
};

MicroRosState g_ros;
int32_t g_latest_pi_count = -1;

void resetHandles() {
    g_ros.support = {};
    g_ros.node = rcl_get_zero_initialized_node();
    g_ros.ack_pub = rcl_get_zero_initialized_publisher();
    g_ros.pi_count_sub = rcl_get_zero_initialized_subscription();
    g_ros.ack_timer = rcl_get_zero_initialized_timer();
    g_ros.executor = rclc_executor_get_zero_initialized_executor();
    g_ros.pi_count_msg = {};
    g_ros.ack_msg = {};
}

void piCountCallback(const void* msg_in) {
    const auto* msg = static_cast<const std_msgs__msg__Int32*>(msg_in);
    g_latest_pi_count = msg->data;
}

void ackTimerCallback(rcl_timer_t* timer, int64_t) {
    if (timer == nullptr) {
        return;
    }

    g_ros.ack_msg.data = g_latest_pi_count;
    (void)rcl_publish(&g_ros.ack_pub, &g_ros.ack_msg, nullptr);
}

void destroyEntities() {
    if (!g_ros.entities_created) {
        resetHandles();
        return;
    }

    rmw_context_t* rmw_context = rcl_context_get_rmw_context(&g_ros.support.context);
    if (rmw_context != nullptr) {
        (void)rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);
    }

    (void)rcl_timer_fini(&g_ros.ack_timer);
    (void)rcl_subscription_fini(&g_ros.pi_count_sub, &g_ros.node);
    (void)rcl_publisher_fini(&g_ros.ack_pub, &g_ros.node);
    (void)rclc_executor_fini(&g_ros.executor);
    (void)rcl_node_fini(&g_ros.node);
    (void)rclc_support_fini(&g_ros.support);

    g_ros.entities_created = false;
    resetHandles();
}

bool createEntities() {
    if (rmw_uros_ping_agent(kAgentPingTimeoutMs, kAgentPingAttempts) != RMW_RET_OK) {
        return false;
    }

    resetHandles();
    g_ros.allocator = rcl_get_default_allocator();

    if (rclc_support_init(&g_ros.support, 0, nullptr, &g_ros.allocator) != RCL_RET_OK) {
        destroyEntities();
        return false;
    }

    if (rclc_node_init_default(&g_ros.node, "esp32_microros_smoke_node", "", &g_ros.support) != RCL_RET_OK) {
        destroyEntities();
        return false;
    }

    if (rclc_publisher_init_default(
            &g_ros.ack_pub,
            &g_ros.node,
            ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
            "/esp32_ack") != RCL_RET_OK) {
        destroyEntities();
        return false;
    }

    if (rclc_subscription_init_default(
            &g_ros.pi_count_sub,
            &g_ros.node,
            ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
            "/pi_count") != RCL_RET_OK) {
        destroyEntities();
        return false;
    }

    if (rclc_timer_init_default(
            &g_ros.ack_timer,
            &g_ros.support,
            RCL_MS_TO_NS(kAckPeriodMs),
            ackTimerCallback) != RCL_RET_OK) {
        destroyEntities();
        return false;
    }

    if (rclc_executor_init(
            &g_ros.executor,
            &g_ros.support.context,
            kExecutorHandles,
            &g_ros.allocator) != RCL_RET_OK) {
        destroyEntities();
        return false;
    }

    if (rclc_executor_add_subscription(
            &g_ros.executor,
            &g_ros.pi_count_sub,
            &g_ros.pi_count_msg,
            &piCountCallback,
            ON_NEW_DATA) != RCL_RET_OK) {
        destroyEntities();
        return false;
    }

    if (rclc_executor_add_timer(&g_ros.executor, &g_ros.ack_timer) != RCL_RET_OK) {
        destroyEntities();
        return false;
    }

    g_latest_pi_count = -1;
    g_ros.entities_created = true;
    return true;
}

void spinMicroRos() {
    const unsigned long now_ms = millis();

    if (!g_ros.entities_created) {
        if ((now_ms - g_ros.last_connect_attempt_ms) >= kReconnectPeriodMs) {
            g_ros.last_connect_attempt_ms = now_ms;
            (void)createEntities();
        }
        return;
    }

    if (rmw_uros_ping_agent(kAgentPingTimeoutMs, kAgentPingAttempts) != RMW_RET_OK) {
        destroyEntities();
        return;
    }

    const rcl_ret_t spin_result = rclc_executor_spin_some(&g_ros.executor, RCL_MS_TO_NS(20));
    if ((spin_result != RCL_RET_OK) && (spin_result != RCL_RET_TIMEOUT)) {
        destroyEntities();
    }
}

}  // namespace

void setup() {
    Serial.begin(kSerialBaud);
    delay(1000);
    set_microros_serial_transports(Serial);
    resetHandles();
}

void loop() {
    spinMicroRos();
    delay(10);
}
