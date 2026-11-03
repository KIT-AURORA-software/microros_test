# Teensy 4.1 micro-ROS / Ethernet

Teensy 4.1 の内蔵 Ethernet と、6 ピン Ethernet 端子に接続する Ethernet Adapter を使用します。
W5500 などの SPI Ethernet モジュール用ではありません。

## ビルドと書き込み

このフォルダーを PlatformIO で開きます。

```bash
pio run -e teensy41
pio run -e teensy41 -t upload
```

USB データケーブルで Teensy を PC に接続してください。書き込み時に待機した場合は、Teensy 本体の Program ボタンを押してください。
VS Code の PlatformIO の Upload からも書き込めます。

ROS 2 Humble を使用します。Teensy プラットフォームは Humble の C ソースとの互換性のため 5.0.0 に固定しています。
初回ビルドはインターネット接続が必要で、micro-ROS ライブラリの生成に時間がかかります。

## 通信

`src/main.cpp` の設定は以下のとおりです。PC の Ethernet インターフェースを同じサブネットに設定してください。

| 設定 | 値 |
| --- | --- |
| PC / Agent IP | `192.168.10.10` |
| Teensy IP | `192.168.10.20` |
| Agent UDP ポート | `8888` |
| ノード名 | `teensy41` |
| 受信トピック | `/to_arduino_by_UDP` (`std_msgs/msg/Int32`) |
| 返信トピック | `/from_arduino_by_UDP` (`std_msgs/msg/Int32`) |

PC で Humble の micro-ROS Agent を起動します。

```bash
ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
```

別のターミナルで受信と送信を確認できます。

```bash
ros2 topic echo /from_arduino_by_UDP
ros2 topic pub --once /to_arduino_by_UDP std_msgs/msg/Int32 '{data: 42}'
```

Agent の応答後にノードを作成し、受信した整数を返信します。Agent 再起動時は Teensy も再起動してください。

参考: [micro_ros_platformio](https://github.com/micro-ROS/micro_ros_platformio)

## ビルドパス

micro-ROS Humble のメッセージ生成処理は日本語パスで失敗するため、依存ライブラリは `~/microros_test/ArduinoUnoR4/.pio/libdeps` で生成します。場所を変更するときは `platformio.ini` の `libdeps_dir` を英数字のパスに変更してください。

`~/microros_test/ArduinoUnoR4` からも直接ビルド・書き込みができます。

書き込み時は `teensy_upload.py` でローダーに `--mcu=TEENSY41` を渡します。Teensy プラットフォーム 5.0.0 の既定値 `imxrt1062` による機種判定の違いを避けるため、このスクリプトもプロジェクトと一緒に保持してください。

実機で再起動直後の初回転送が失敗する現象を確認したため、書き込み前に2秒待機し、失敗時は再起動を挟まず1回だけ再試行します。途中で `error writing to Teensy` が表示されても、最後が `[SUCCESS]` なら書き込みは完了しています。2回とも失敗した場合はエラーとして終了します。
