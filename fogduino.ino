#include <WiFi.h>
#include <PubSubClient.h>

// variabili di conessione wifi/mqtt
const char* ssid = "Redmi 8T di Andrea";
const char* password = "A123456789";
const char* mqtt_server = "broker.hivemq.com";

//variabili gestione pin 
const int pinRelayCoil=17; //relè resistenza
const int pinTemp=15;     //sensore b18 temperatura 
const int pinFanPWM=27;   //pwm verso il fan
const int pinFanRPM=34;   //rpm dal fan
const int pinLDR=35;      //fotoresistore

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg,last,lastdata= 0;
char msg[100];
int fanpwm,fanrpm,coiltemp=0;
bool fan=false;
bool coil=false;
bool isOn=false;
bool firstStart=true;
bool mantenimento=false;
int touch=0;
int temp=0; 

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Mi connetto a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { //loppa finche non connesso
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
    String clientId = "ESP32Client-"; //crea id client mqtt
    clientId += "espfogduino";      //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected"); // Once connected, publish an announcement...
      client.publish("fogduino/status", "Connessione riuscita");
      client.subscribe("fogduino/testsub"); //sottoscrizione, la callback si invoca alla ricezione di qualcosa su questo topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);      
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  TaskHandle_t Task1;
   xTaskCreatePinnedToCore(
    mantieniErogazione,      // Function that should be called
    "eroga fumo",    // Name of the task (for debugging)
    1000,               // Stack size (bytes)
    NULL,               // Parameter to pass
    1,                  // Task priority
    &Task1,               // Task handle
    0);         // Core you want to run the task on (0 or 1)
}

void pubblicaDati(){  // invia tramite mqtt dati di stato
  
  if (millis()- lastdata > 4000 ) {
    lastdata = millis();
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
    Serial.print("Pub coil activation: ");
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

void aggiornaHW(){ //aggiorna i dati letti dai sensori, quindi rpm, temp, fotoresistenza.
    //inseriee il codice per sapere rpm
    Serial.println("sto aggiornando i valori letti dai sensori!");
    //lettura da sens temperatura e luce
    
  }

void flood(){ //uso il servo per "pucciare" la coil e rifornirla di liquido da evaporare
      //spostati di 90 gradi, aspetta 1 secondo, poi raddrizza
  }

void mantieniErogazione(void * parameter){
        //se attivo il mantenimento allora eseguo
      for(;;){
          if(mantenimento){ //se arriva la disattivazione del mantenimento, mi autokillo
           //mantengo il fumo: ventola al massimo a meno che la temperatura non stia calando, ogni tot tempo flood sulla coil (che deve essere prima spenta)
          
          //#### debug

          Serial.println("Sto mantenendo la coil!!");
          Serial.print("Task1 running on core ");
          Serial.println(xPortGetCoreID());
           vTaskDelay(20); //necessario per watchdog di rtos altrimenti la task occupa tutte le risorse di cpu
          }
          vTaskDelay(20);
      }
  }
  
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //####################  simulazione 1#######  
  if(firstStart){ //se prima accensione, si ipotizza a freddo (non dopo un reset) allora riscaldo un pò di piu la coil
        fanpwm=125;
        //comando per scrittura pwm su fan
        digitalWrite(pinRelayCoil, HIGH);
        Serial.println("riscaldo la coil la prima volta, circa 8 secondi");
        delay(8000);
        digitalWrite(pinRelayCoil,LOW);
        Serial.println("Ho spento la coil dopo il preriscaldamento");
        mantenimento=true;
        firstStart=false;
    }

  if (millis()- last > 1000 ) {
    last=millis();
    aggiornaHW();
  }
  
  pubblicaStatus();
  pubblicaDati();
  
}
