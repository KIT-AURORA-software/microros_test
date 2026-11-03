#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>

byte mac[] = {0x02, 0, 0, 0, 0, 0x42};

IPAddress pc_ip(192, 168, 10, 15);
IPAddress board_ip(192, 168, 10, 20);

rcl_node_t node;
rcl_publisher_t pub;
rcl_subscription_t sub;
rclc_support_t support;
rclc_executor_t exec;
rcl_allocator_t alloc;
std_msgs__msg__Int32 msg;

void callback(const void *data) {
    rcl_publish(&pub, data, NULL);
}

void setup() {
    set_microros_native_ethernet_transports(mac, board_ip, pc_ip, 8888);

    while (rmw_uros_ping_agent(1000, 1) != RMW_RET_OK)
        delay(500);

    alloc = rcl_get_default_allocator();
    rclc_support_init(&support, 0, NULL, &alloc);
    rclc_node_init_default(&node, "teensy41", "", &support);

    rclc_publisher_init_default(
        &pub, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "/from_arduino_by_UDP"
    );

    rclc_subscription_init_default(
        &sub, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "/to_arduino_by_UDP"
    );

    rclc_executor_init(&exec, &support.context, 1, &alloc);
    rclc_executor_add_subscription(
        &exec, &sub, &msg, &callback, ON_NEW_DATA
    );
}

void loop() {
    rclc_executor_spin_some(&exec, RCL_MS_TO_NS(10));
}
