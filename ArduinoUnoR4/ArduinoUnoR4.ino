
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <micro_ros_arduino.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>

byte mac[] = {0x02, 0, 0, 0, 0, 0x42};

IPAddress pc_ip(192, 168, 10, 10);
IPAddress uno_ip(192, 168, 10, 20);

EthernetUDP udp;

rcl_node_t node;
rcl_publisher_t pub;
rcl_subscription_t sub;
rclc_support_t support;
rclc_executor_t exec;
rcl_allocator_t alloc;
std_msgs__msg__Int32 msg;

extern "C" bool udp_open(uxrCustomTransport *) {
    return udp.begin(8889) == 1;
}

extern "C" bool udp_close(uxrCustomTransport *) {
    udp.stop();
    return true;
}

extern "C" size_t udp_write(
    uxrCustomTransport *, const uint8_t *buf,
    size_t len, uint8_t *) {
    if (!udp.beginPacket(pc_ip, 8888)) return 0;
    size_t n = udp.write(buf, len);
    return udp.endPacket() ? n : 0;
}

extern "C" size_t udp_read(
    uxrCustomTransport *, uint8_t *buf,
    size_t len, int timeout, uint8_t *) {
    uint32_t start = millis();
    do {
        int n = udp.parsePacket();
        if (n > 0) {
            if ((size_t)n > len) {
                while (udp.available()) udp.read();
                return 0;
            }
            return udp.read(buf, n);
        }
        delay(1);
    } while (millis() - start < (uint32_t)timeout);
    return 0;
}

void callback(const void *data) {
    rcl_publish(&pub, data, NULL);
}

void setup() {
    Ethernet.init(10);
    Ethernet.begin(mac, uno_ip);

    rmw_uros_set_custom_transport(
        false, NULL,
        udp_open, udp_close, udp_write, udp_read
    );

    while (rmw_uros_ping_agent(1000, 1) != RMW_RET_OK)
        delay(500);

    alloc = rcl_get_default_allocator();
    rclc_support_init(&support, 0, NULL, &alloc);
    rclc_node_init_default(&node, "uno_r4", "", &support);

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
