import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32

class NumberPub(Node):
    def __init__(self):
        super().__init__('number_pub')
        self.pub = self.create_publisher(Int32, '/to_arduino_by_UDP', 10)
        self.n = 0
        self.create_timer(1.0, self.send)

    def send(self):
        self.pub.publish(Int32(data=self.n))
        self.get_logger().info(f'[Publisher]to_arduino_by_UDP: {self.n}')
        self.n += 1


def main():
    rclpy.init()
    node = NumberPub()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
