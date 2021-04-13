#include <WiFiType.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <WiFiSTA.h>
#include <ETH.h>
#include <WiFiAP.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiServer.h>
#include <WiFiScan.h>


/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "Redmi 8T di Andrea";
const char* password = "A123456789";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[100];
int value = 0;
bool fan=false;
bool coil=false;
bool isOn=false;
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messaggio="";
  for (int i = 0; i < length; i++) {
    messaggio += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  int mex=messaggio.toInt();
  Serial.println("Messaggio salvato in stringa: "); Serial.println(messaggio);
  switch(mex){
      case 0: //spegni
                    isOn=false;
                    break;
      case 1: //accendi
                    isOn=true;
                    break;
      case 2: //accendi fan
                    if(isOn)fan=true;
                    break;
      case 3: //spegni fan
                    fan=false;
                    if(isOn) coil=false; //se spengo ventola quando sono on, spengo anche la coil !! danger  flame !!
                    break;
      case 4: //accendi coil
                    if (fan) coil=true;
                    break;
      case 5:  //spegni coil
                  coil=false;
                  break;
    }

    
  Serial.println();


}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += "espfogduino";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("fogduino/test", "Connessione riuscita");
      // ... and resubscribe
      client.subscribe("fogduino/testsub"); //sottoscrizione, la callback si invoca alla ricezione di qualcosa su questo topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000 ) {
    lastMsg = now;
    ++value;
    snprintf (msg, 100, "Status: isOn: %d, fan: %d, coil: %d" ,isOn,fan,coil, value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("fogduino/test", msg);
  }
}
