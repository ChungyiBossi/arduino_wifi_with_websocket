// 0 – WStype_ERROR
// 1 – WStype_DISCONNECTED
// 2 – WStype_CONNECTED
// 3 – WStype_TEXT
// 4 – WStype_BIN
// 5 – WStype_FRAGMENT_TEXT_START
// 6 – WStype_FRAGMENT_BIN_START
// 7 – WStype_FRAGMENT
// 8 – WStype_FRAGMENT_FIN
// 9 – WStype_PING
// 10- WStype_PONG

// 0 : WL_IDLE_STATUS when Wi-Fi is in process of changing between statuses
// 1 : WL_NO_SSID_AVAILin case configured SSID cannot be reached
// 3 : WL_CONNECTED after successful connection is established
// 4 : WL_CONNECT_FAILED if connection failed
// 6 : WL_CONNECT_WRONG_PASSWORD if password is incorrect
// 7 : WL_DISCONNECTED if module is not configured in station mode

char* ssid = "YOUR_WIFI_NAME";
char* pwd = "YOUR_WIFI_PASSWORD";
char* serverUrl = "YOUR_SOCKIO_SERVER";
int port = 80;

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

ESP8266WiFiMulti WiFiMulti;
SocketIOclient socketIO;

#define USE_SERIAL Serial

// 13 = D7
// 16 = D0
// 15 = D8
// 14 = D5
// 12 = D6
// 4 = D2
// 3 = D9
// 2 = D4
// 1 = D10
// 0 = D3
int PIN_OUT_A = 13;
int PIN_OUT_B = 12;

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    DynamicJsonDocument doc(1024);
    String eventName;
    String eventMessage;
    DeserializationError error;
    switch(type) {
        case sIOtype_DISCONNECT:
            USE_SERIAL.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
            USE_SERIAL.printf("[IOc] get event: %s\n", payload);
      
            error = deserializeJson(doc, payload, length);
            if(error) {
                USE_SERIAL.print(F("deserializeJson() failed: "));
                USE_SERIAL.println(error.c_str());
                return;
            }
            else
            {
              serializeJson(doc[0], eventName);
              USE_SERIAL.print("Event: "); USE_SERIAL.println(eventName);
              // Car rotate event
              if (strcmp(doc[0], "car_rotate") == 0){
                // https://www.runoob.com/cprogramming/c-function-strcmp.html
                serializeJson(doc[1]["rotationSide"], eventMessage);
                USE_SERIAL.print("Event Message: "); USE_SERIAL.println(eventMessage);
                if (atof(eventMessage.c_str()) == 1.0){
                  setRotateSignal(true);
                }
                else {
                  setRotateSignal(false);
                }
              }
              else {
                serializeJsonPretty(doc, USE_SERIAL);
                USE_SERIAL.println("Other event");
              }
            }
            break;
        case sIOtype_ACK:
            USE_SERIAL.printf("[IOc] get ack: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_ERROR:
            USE_SERIAL.printf("[IOc] get error: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_EVENT:
            USE_SERIAL.printf("[IOc] get binary: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_ACK:
            USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
            hexdump(payload, length);
            break;
    }
}

void setRotateSignal(bool isClockwise){
  // PIN_A = which side ?
  // PIN_B = is rotating ?
  if (isClockwise == true) {
    USE_SERIAL.println("Car rotate: Rotate Clockwise");
    digitalWrite(PIN_OUT_A, LOW);
    digitalWrite(PIN_OUT_B, HIGH);
  }
  else {
    USE_SERIAL.println("Car rotate: Rotate Counter-Clockwise");
    digitalWrite(PIN_OUT_A, HIGH);
    digitalWrite(PIN_OUT_B, HIGH);
  }

  delay(500);
  digitalWrite(PIN_OUT_B, LOW);
}

void resetSerial() {

    USE_SERIAL.begin(115200);
    USE_SERIAL.setDebugOutput(true);
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] Resetting Serial Port: %d...\n", t);
        USE_SERIAL.flush();
    }

}

void setup() {
    // output data
    pinMode(PIN_OUT_A, OUTPUT);
    pinMode(PIN_OUT_B, OUTPUT);
    digitalWrite(PIN_OUT_A, LOW);
    digitalWrite(PIN_OUT_B, LOW);

    resetSerial();
    delay(1000);


    // disable AP
    if(WiFi.getMode() & WIFI_AP) {
        WiFi.softAPdisconnect(true);
    }
    WiFiMulti.addAP(ssid, pwd);
    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }
    String ip = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());
    // server address, port and URL
    socketIO.begin(serverUrl, port, "/socket.io/?transport=polling&EIO=4");
    // event handler
    socketIO.onEvent(socketIOEvent);
}

unsigned long messageTimestamp = 0;
void loop() {
    socketIO.loop();
    uint64_t now = millis();
    if(now - messageTimestamp > 2000) {
        messageTimestamp = now;

        // creat JSON message for Socket.IO (event)
        DynamicJsonDocument doc(1024);
        JsonArray array = doc.to<JsonArray>();
        
        // add evnet name
        // Hint: socket.on('event_name', ....
        array.add("event_name");

        // add payload (parameters) for the event
        JsonObject param1 = array.createNestedObject();
        param1["now"] = (uint32_t) now;

        String output;
        serializeJson(doc, output);
        socketIO.sendEVENT(output);
    }
}