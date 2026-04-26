// ESP32-WROOM-32 + DFRobot Air780EU 4G CAT1 modem
// GitHub-ready demo: SIM unlock, network attach, MQTT subscribe + publish
#define MODEM_RX 16
#define MODEM_TX 17
HardwareSerial modem(2);
const char *SIM_PIN = "6030";
const char *APN = "web.vodafone.de";
const char *MQTT_HOST = "broker.emqx.io";
const char *MQTT_PORT = "1883";
const char *MQTT_CLIENT_ID = "esp32-air780eu-demo";
const char *MQTT_TOPIC_SUB = "esp32/air780eu/in";
const char *MQTT_TOPIC_PUB = "esp32/air780eu/out";
String sendAT(const String &cmd, uint32_t timeoutMs = 3000) {
  while (modem.available()) {
    modem.read();
  }
  modem.println(cmd);
  Serial.print("\n>> ");
  Serial.println(cmd);
  String response;
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    while (modem.available()) {
      response += (char)modem.read();
    }
  }
  Serial.print("<< ");
  Serial.println(response);
  return response;
}
bool responseContains(const String &response, const char *expected) {
  return response.indexOf(expected) >= 0;
}
bool sendAndExpect(const String &cmd, const char *expected, uint32_t timeoutMs = 3000) {
  String response = sendAT(cmd, timeoutMs);
  return responseContains(response, expected);
}
bool waitForSimReady(uint32_t totalMs = 20000) {
  uint32_t start = millis();
  while (millis() - start < totalMs) {
    String response = sendAT("AT+CPIN?", 1500);
    if (responseContains(response, "+CPIN: READY")) {
      return true;
    }
    delay(1000);
  }
  return false;
}
bool unlockSimIfNeeded() {
  String response = sendAT("AT+CPIN?", 2000);
  if (responseContains(response, "SIM PIN")) {
    Serial.println("SIM is locked. Sending PIN...");
    String unlockCmd = String("AT+CPIN=\"") + SIM_PIN + "\"";
    sendAT(unlockCmd, 5000);
    delay(5000);
  }
  return waitForSimReady();
}
bool configurePacketData() {
  String pdpCmd = String("AT+CGDCONT=1,\"IP\",\"") + APN + "\"";
  if (!sendAndExpect(pdpCmd, "OK", 3000)) {
    return false;
  }
  if (!sendAndExpect("AT+CGATT=1", "OK", 10000)) {
    return false;
  }
  if (!sendAndExpect("AT+CGACT=1,1", "OK", 10000)) {
    return false;
  }
  if (!sendAndExpect("AT+CGATT?", "+CGATT: 1", 3000)) {
    return false;
  }
  return true;
}
bool connectMqtt() {
  String configCmd = String("AT+MCONFIG=\"") + MQTT_CLIENT_ID + "\",\"\",\"\"";
  if (!sendAndExpect(configCmd, "OK", 3000)) {
    return false;
  }
  String tcpCmd = String("AT+MIPSTART=\"") + MQTT_HOST + "\",\"" + MQTT_PORT + "\"";
  if (!sendAndExpect(tcpCmd, "CONNECT OK", 12000)) {
    return false;
  }
  if (!sendAndExpect("AT+MCONNECT=1,60", "CONNACK OK", 12000)) {
    return false;
  }
  return true;
}
bool subscribeTopic() {
  String subCmd = String("AT+MSUB=\"") + MQTT_TOPIC_SUB + "\",0";
  return sendAndExpect(subCmd, "SUBACK", 12000);
}
bool publishStartupMessage() {
  String payload = "hello from esp32 + air780eu";
  String pubCmd = String("AT+MPUB=\"") + MQTT_TOPIC_PUB + "\",0,0,\"" + payload + "\"";
  return sendAndExpect(pubCmd, "OK", 8000);
}
void printBringupSummary() {
  sendAT("AT+CSQ", 2000);
  sendAT("AT+CREG?", 2000);
  sendAT("AT+CGATT?", 2000);
}
void setup() {
  Serial.begin(115200);
  delay(1000);
  modem.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(2000);
  Serial.println("\nAir780EU MQTT demo starting...");
  if (!sendAndExpect("AT", "OK", 2000)) {
    Serial.println("Modem did not answer AT. Check power, baud rate, and wiring.");
    return;
  }
  sendAT("ATI", 2000);
  // Disable unsolicited SMS dumps so the serial log stays readable.
  sendAT("AT+CNMI=0,0,0,0,0", 2000);
  if (!unlockSimIfNeeded()) {
    Serial.println("SIM did not become READY. Check SIM PIN and SIM card setting.");
    return;
  }
  printBringupSummary();
  if (!configurePacketData()) {
    Serial.println("Packet data setup failed.");
    return;
  }
  if (!connectMqtt()) {
    Serial.println("MQTT connection failed.");
    return;
  }
  if (!subscribeTopic()) {
    Serial.println("MQTT subscribe failed.");
    return;
  }
  if (!publishStartupMessage()) {
    Serial.println("MQTT publish failed.");
    return;
  }
  Serial.println("\nSystem online.");
  Serial.print("Subscribed to: ");
  Serial.println(MQTT_TOPIC_SUB);
  Serial.print("Published startup message to: ");
  Serial.println(MQTT_TOPIC_PUB);
  Serial.println("You can now send AT commands manually through the Serial Monitor.");
}
void loop() {
  if (Serial.available()) {
    modem.write(Serial.read());
  }
  if (modem.available()) {
    Serial.write(modem.read());
  }
}