## TTGO T-Beam WIZnet W5500 Lite: Ethernet + LoRa + MQTT

![TTGO T-Beam WIZnet W5500 Lite: Ethernet + LoRa + MQTT](/ttgo-t-beam-wiz5500-lite.png)

PoC: how to use the Arduino `Ethernet`, `arduino-LoRa` and `PubSubClient` library simultaneously. The Device sends a test packet every 15 seconds via LoRa and it listens to the topic `inTopic`. MQTT packets received on the `inTopic` topic gets transmitted via LoRa and a copy of the content is send back to the topic `outTopic`.

✍️ Blog: https://www.technopolis.tv/PlatformIO-Ethernet-LoRa-TTGO-T-Beam-WIZnet-W5500-Lite/