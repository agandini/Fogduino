#include <WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "Redmi 8T di Andrea";
const char* password = "A123456789";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[100];
int fanpwm,fanrpm,coiltemp=0;
bool fan=false;
bool coil=false;
bool isOn=false;
int touch=0;
int temp=0; 
  
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Mi connetto a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("Connessione riuscita!");
 // Serial.println("IP address: ");
 // Serial.println(WiFi.localIP());
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
  
  int mex=messaggio.toInt(); //mi aspetto messaggi numerici

  switch(mex){
      case -1: //spegni tutto
                    isOn=false;
                    fan=false;
                    coil=false;
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
      case 4: //accendi coil solo se anche fan è accesa (evitare surriscaldamenti)
                    if (fan) coil=true;
                    break;
      case 5:  //spegni coil 
                  coil=false;
                  break;
      default: Serial.println("comando non valido");
                  break;
    }
  Serial.println();
}

void reconnect() {

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Connesione WiFi persa!");
    setup_wifi();
    }
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
      client.publish("fogduino/status", "Connessione riuscita");
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

void pubblicaDati(){
  long now = millis();
  if (now - lastMsg > 4000 ) {
    lastMsg = now;
     //invia % pwm e rpm della ventola
    snprintf (msg, 100, "%d" ,fanpwm);
    Serial.print("Pub fan pwm: ");
    Serial.println(msg);
    client.publish("fogduino/fan/pwm", msg);

    snprintf (msg, 100, "%d" ,fanrpm);
    Serial.print("Pub fan rpm: ");
    Serial.println(msg);
    client.publish("fogduino/fan/rpm", msg);
    //invia resistenza della coil
    snprintf (msg, 100, "%d" ,coiltemp);
    Serial.print("Pub coil resistance: ");
    Serial.println(msg);
    client.publish("fogduino/coil", msg);
    }
    
  }
void pubblicaStatus(){
  long now = millis();
  if (now - lastMsg > 4000 ) {
    lastMsg = now;
    snprintf (msg, 100, "Status: isOn: %d, fan: %d, coil: %d" ,isOn,fan,coil);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("fogduino/status", msg);
    }
  }

void aggiornaHW(){
    coilres=touchRead(4);
    //ottenere rpm da fan e calcolare resistenza coil
    // check liquido rimasto //check fiamma // check fumo
    
  }
  
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  aggiornaHW();
 // pubblicaStatus();
  pubblicaDati();
  
}
