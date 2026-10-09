
# micro-ROS UDP Communication Test

## 松尾が真面目に話す部分

Teensy4.1にはmicro-ROSの標準ライブラリがあり、platformIOでスムーズに開発できます。しかしArduino UNO R4には標準ライブラリがないためサードパーティでなんとかしてます。本番ではTeensyを使うので実際に動かす価値はあまり感じません。松尾のPCなら動きます。
agentのパッケージ内部に.githubが残っています。いつかワンチャン苦しめられるかもしれないです。そのときは消してください。
micro-ROSコンパイラの仕様なのか勝手に他のワークスペースからライブラリを呼んでコンパイルしました。コンパイラってそういうものなのかも。
焼き込んだ.inoファイルにおいてSPIのライブラリをincludeしたのは、EthernetシールドとArduinoはSPI通信をしているからです。Ethernetライブラリがどの道SPIライブラリを呼ぶので消しても大丈夫だと思います(たぶん)。
Arduino CLIで焼き込みをしました。PlatformIOにライブラリでエンチャントするのが嫌だったからです。
次はTeensyのテストか、Jetson-ArduinoのUDP通信テストか、２号機のコードをmicro-ROS用にアレンジして遊びます。主担当にバレるまで。

※追記
はじめCodexに「PlatformIOで開発できるようにして」と指示したらVScodeをグチャグチャにされました。open AI 許すまじ。

## ここからChatGPT

Arduino UNO R4とPC（ROS 2）間で、micro-ROSを利用した双方向UDP通信を行うテストプログラムです。

PCから1秒ごとに整数を送信し、Arduinoが受信した値をそのままPCへ返信します。

## 1. システム構成

### 使用環境

| 項目 | 使用環境 |
|---|---|
| PC OS | Ubuntu 24.04 |
| ROS 2 | Jazzy |
| マイコン | Arduino UNO R4 |
| 通信方式 | Ethernet / UDP (IPv4) |
| ミドルウェア | micro-ROS |
| Arduino開発環境 | Arduino IDE |
| PC側言語 | Python |
| Arduino側言語 | C++ |

Arduino側ではSPI接続のEthernetインターフェースを使用します。EthernetのCSピンは10番に設定しています。

### 通信の流れ

```text
               PC (Ubuntu 24.04)
                  ROS 2 Jazzy
                        |
             +----------+----------+
             |                     |
       number_pub.py         number_sub.py
             |                     ^
             v                     |
    /to_arduino_by_UDP   /from_arduino_by_UDP
             |                     ^
             v                     |
             +--- micro-ROS Agent--+
                        |
                    UDP 8888
                        |
                  Ethernet
                        |
                  Arduino UNO R4
                        |
                  micro-ROS Node
                    "uno_r4"
                        |
                  受信値を返信
```

## 2. ネットワーク設定

本プログラムでは以下の固定IPアドレスを使用します。

| デバイス | IPアドレス | ポート |
|---|---|---|
| PC / micro-ROS Agent | 192.168.10.10 | UDP 8888 |
| Arduino UNO R4 | 192.168.10.20 | UDP 8889 |

Arduino側の設定は `ArduinoUnoR4/ArduinoUnoR4.ino` に記述されています。

```cpp
IPAddress pc_ip(192, 168, 10, 10);
IPAddress uno_ip(192, 168, 10, 20);
```

異なるネットワーク環境で実行する場合は、これらのアドレスを変更してください。

PCとArduinoは同一ネットワーク上で通信できる必要があります。

## 3. ディレクトリ構成

```text
microros_test/
├── ArduinoUnoR4/
│   └── ArduinoUnoR4.ino
│
└── PCtopic_ws/
    └── src/
        ├── counter/
        │   ├── counter/
        │   │   ├── __init__.py
        │   │   ├── number_pub.py
        │   │   └── number_sub.py
        │   ├── resource/
        │   ├── test/
        │   ├── package.xml
        │   ├── setup.py
        │   └── setup.cfg
        │
        ├── micro-ROS-Agent/
        └── micro_ros_msgs/
```

各ファイルの役割：

- `ArduinoUnoR4.ino`：UDP通信・micro-ROSノード・受信データの返信
- `number_pub.py`：PCから整数を1秒ごとに送信
- `number_sub.py`：Arduinoから返信された整数を表示
- `micro-ROS-Agent/`：micro-ROS Agentのソースコード
- `micro_ros_msgs/`：micro-ROS関連のメッセージ定義

## 4. セットアップ

### 4.1 リポジトリの取得

```bash
git clone https://github.com/KIT-AURORA-software/microros_test.git
cd microros_test
```

### 4.2 ROS 2ワークスペースの準備

ROS 2 Jazzy、colcon、rosdepがインストールされていることを前提とします。

```bash
source /opt/ros/jazzy/setup.bash

cd PCtopic_ws
```

`src/` 内に `micro-ROS-Agent` と `micro_ros_msgs` が存在しない場合は、以下で取得します。

```bash
git clone -b jazzy \
  https://github.com/micro-ROS/micro-ROS-Agent.git \
  src/micro-ROS-Agent

git clone -b jazzy \
  https://github.com/micro-ROS/micro_ros_msgs.git \
  src/micro_ros_msgs
```

必要な依存関係をインストールします。

```bash
rosdep install --from-paths src --ignore-src -r -y
```

ワークスペースをビルドします。

```bash
colcon build --symlink-install
source install/setup.bash
```

### 4.3 Arduino側の準備

Arduino IDEで以下のスケッチを開きます。

```text
ArduinoUnoR4/ArduinoUnoR4.ino
```

必要なライブラリ：

- SPI
- Ethernet
- EthernetUdp（Ethernetライブラリに含まれる）
- micro_ros_arduino

micro-ROSに対応したArduino環境を準備し、使用するArduino UNO R4にスケッチを書き込みます。

EthernetのCSピンは以下のように設定されています。

```cpp
Ethernet.init(10);
```

使用するEthernetハードウェアに応じて設定を変更してください。

## 5. 実行方法

PCでは複数のターミナルを使用します。

### Terminal 1：micro-ROS Agentの起動

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros/microros_test/PCtopic_ws/install/setup.bash

ros2 run micro_ros_agent micro_ros_agent \
  udp4 --port 8888 -v4
```

UDPポート8888でArduinoからの接続を待ち受けます。

### Terminal 2：Subscriberの起動

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros/microros_test/PCtopic_ws/install/setup.bash

ros2 run counter number_sub
```

Arduinoから送られた値を表示します。

### Terminal 3：Publisherの起動

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros/microros_test/PCtopic_ws/install/setup.bash

ros2 run counter number_pub
```

1秒ごとに整数を送信します。

### Arduinoの起動

ArduinoをEthernetネットワークに接続して起動します。

Arduinoはmicro-ROS Agentへの接続に成功すると、PublisherとSubscriberを初期化します。

## 6. 通信仕様

### 使用トピック

| トピック名 | 型 | 通信方向 |
|---|---|---|
| `/to_arduino_by_UDP` | `std_msgs/msg/Int32` | PC → Arduino |
| `/from_arduino_by_UDP` | `std_msgs/msg/Int32` | Arduino → PC |

### 動作内容

1. `number_pub.py` が整数0から送信を開始します。
2. 1秒ごとに整数を1ずつ増加させます。
3. Arduinoが `/to_arduino_by_UDP` を購読します。
4. Arduinoは受信した整数を `/from_arduino_by_UDP` にPublishします。
5. `number_sub.py` が受信結果を表示します。

### 出力例

Publisher：

```text
[Publisher]to_arduino_by_UDP: 0
[Publisher]to_arduino_by_UDP: 1
[Publisher]to_arduino_by_UDP: 2
[Publisher]to_arduino_by_UDP: 3
```

Subscriber：

```text
[Subscriber]from_arduino_by_UDP: 0
[Subscriber]from_arduino_by_UDP: 1
[Subscriber]from_arduino_by_UDP: 2
[Subscriber]from_arduino_by_UDP: 3
```

## 7. 通信確認

### トピック一覧

```bash
ros2 topic list
```

### Arduinoからの受信データ確認

```bash
ros2 topic echo /from_arduino_by_UDP
```

### PCからArduinoへの手動送信

```bash
ros2 topic pub --once \
  /to_arduino_by_UDP \
  std_msgs/msg/Int32 \
  "{data: 100}"
```

Arduinoとの通信が正常であれば、以下のコマンドで100が返信されます。

```bash
ros2 topic echo --once /from_arduino_by_UDP
```

※ 手動送信試験では `number_pub` を停止しておくと確認しやすくなります。

## 8. トラブルシューティング

### ArduinoがAgentに接続できない

以下を確認してください。

- PCとArduinoのIPアドレスが正しいか
- Ethernetケーブルが接続されているか
- PCとArduinoが互いに通信可能なネットワークに存在するか
- micro-ROS AgentがUDPポート8888で起動しているか
- ファイアウォールがUDP通信を遮断していないか

ArduinoはAgentへの接続が成功するまで、以下の処理で待機します。

```cpp
while (rmw_uros_ping_agent(1000, 1) != RMW_RET_OK)
    delay(500);
```

### ROS 2トピックが見つからない

各ターミナルでROS 2環境を読み込んでください。

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros/microros_test/PCtopic_ws/install/setup.bash
```

### Pythonノードが見つからない

ワークスペースを再ビルドしてください。

```bash
cd ~/ros/microros_test/PCtopic_ws

colcon build --symlink-install
source install/setup.bash
```

## 9. 参考資料

- [micro-ROS公式サイト](https://micro.ros.org/)
- [micro-ROS Arduino](https://github.com/micro-ROS/micro_ros_arduino)
- [micro-ROS Agent](https://github.com/micro-ROS/micro-ROS-Agent)
- [ROS 2 Jazzy Documentation](https://docs.ros.org/en/jazzy/)

## 10. リポジトリ

GitHub Organization：KIT-AURORA-software

Repository：microros_test

本リポジトリは、ArduinoとROS 2間のmicro-ROS通信を検証するためのテスト環境です。
