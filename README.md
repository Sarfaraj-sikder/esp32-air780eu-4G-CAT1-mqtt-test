**ESP32 + Air780EU 4G CAT1 MQTT Demo**
A practical embedded systems project showing how to connect an ESP32-WROOM-32 to a DFRobot Air780EU 4G CAT1 modem over UART, unlock a SIM card with a PIN, attach to the cellular data network, and perform two-way MQTT communication over the internet.

**Why This Project Matters**
This project demonstrates hands-on embedded and IoT engineering skills:

-Hardware UART integration on ESP32
-AT-command based modem control
-SIM PIN unlock flow
-Cellular registration and packet data attach
-MQTT publish and subscribe over 4G
-Serial debugging and response handling
This is not a simulated test. The modem successfully:

-unlocked the SIM
-registered on the Vodafone network
-attached to packet data
-connected to a public MQTT broker
-published an MQTT message

**Hardware Used**
ESP32-WROOM-32
DFRobot Air780EU 4G CAT1 modem
Vodafone SIM card with PIN enabled
External LTE antenna
Jumper wires

**Software Used**
Arduino IDE

**Wiring**
Air780EU TX -> ESP32 GPIO16 (RX2)
Air780EU RX -> ESP32 GPIO17 (TX2)
Air780EU GND -> ESP32 GND
Important notes:

TX and RX must be crossed
The modem should use a stable external power source
The antenna should be connected before powering the modem

**Code Structure**
The ESP32 uses UART2 to send AT commands to the Air780EU modem.

Main flow:

Start serial communication with the modem
Check modem communication with AT
Query SIM state with AT+CPIN?
Unlock the SIM if it requests a PIN
Check signal quality with AT+CSQ
Check network registration with AT+CREG?
Attach to packet data with AT+CGATT=1
Configure APN and activate the PDP context
Configure the modem's built-in MQTT client
Open a TCP connection to a public broker
Start an MQTT session
Subscribe to a test topic
Publish a test message
Example Successful Output
Successful runs include responses like:

+CPIN: READY
+CSQ: 21,0
+CREG: 0,1
+CGATT: 1
CONNECT OK
CONNACK OK
SUBACK
OK after publish
These responses indicate that the modem completed cellular registration, packet data attach, and MQTT messaging over the internet.
    
**Configuration**
Before uploading the sketch, update these values:

SIM_PIN
APN
MQTT_HOST
MQTT_TOPIC_SUB
MQTT_TOPIC_PUB
Default APN in this example:

web.vodafone.de
This worked in the tested setup, but APN values can vary depending on country, plan, and SIM type.

**How To Run**
Connect the hardware
Open the sketch in Arduino IDE
Select your ESP32 board
Update the SIM PIN and any MQTT topic names
Upload the code
Open Serial Monitor at 115200
Watch the modem register, attach, subscribe, and publish

**How To Test Subscribe + Publish**
The sketch subscribes to a topic and publishes a startup message.

You can verify bidirectional MQTT behavior by:

Connecting to the same broker with any MQTT client
Publishing a message to the subscribe topic used by the ESP32
Watching the modem forward incoming MQTT data to the Serial Monitor


