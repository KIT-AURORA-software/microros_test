import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32

class NumberSub(Node):
    def __init__(self):
        super().__init__('number_sub')
        self.sub = self.create_subscription(
            Int32, '/from_arduino_by_UDP', self.receive, 10
        )

    def receive(self, msg):
        self.get_logger().info(f'[Subscriber]from_arduino_by_UDP: {msg.data}')


def main():
    rclpy.init()
    node = NumberSub()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()