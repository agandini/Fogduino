#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 15

// variabili di conessione wifi/mqtt
const char* ssid = "FASTWEB- AG";
const char* password = "A123456789";
const char* mqtt_server = "broker.hivemq.com";

//variabili gestione pin 
const int pinRelayCoil=17; //relè resistenza
const int pinTemp=15;     //sensore b18 temperatura 
const int pinFanPWM=27;   //pwm verso il fan
const int pinFanRPM=26;   //rpm dal fan
const int pinLR=35;      //fotoresistore

//variabili gestione temp con onewire
OneWire oneWire(pinTemp);
DallasTemperature sensors (&oneWire);

unsigned long start_time;
//gestione pwm ventola
int PWM_FREQUENCY = 25000; 
int PWM_CHANNEL = 0; 
int PWM_RESOUTION = 8; //duty cycle con 2^8 (0 - 255)

WiFiClient espClient;
PubSubClient client(espClient);

char msg[100];
int fanpwm,fanrpm,light,lightRef,count=0;
float temp,tempRef=0;
bool isOn=false;
bool coldStart=true;
volatile bool mantenimento=false;
unsigned long last = 0;
unsigned long lastMsg = 0;
unsigned long lastdata = 0;
unsigned long lastOn =0;


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
  if(strcmp(topic,"fogduino/setpwm")==0){
    fanpwm=mex;
   }
  else{
    switch(mex){
        case -1: //spegni tutto
                      isOn=false;
                      digitalWrite(pinRelayCoil,LOW);
                      break;
        case 1: //accendi
                      isOn=true;
                      break;                   
        default: Serial.println("comando non valido");
                    break;
      }
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
    if (client.connect(clientId.c_str(),NULL,NULL,"fogduino/status",1,true,"Connessione persa!Mi riconnetterò a breve...")) {
      Serial.println("connected"); // Once connected, publish an announcement...
      client.publish("fogduino/status", "Connessione riuscita");
      client.subscribe("fogduino/ctrl"); //sottoscrizione, la callback si invoca alla ricezione di qualcosa su questo topic
      client.subscribe("fogduino/setpwm"); //sottoscrizione, la callback si invoca alla ricezione di qualcosa su questo topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);      
    }
  }
}

void firstStart(){
     //se prima accensione, si ipotizza a freddo (non dopo un reset) allora riscaldo un pò di piu la coil
        fanpwm=10;
        setPWM(fanpwm);
        digitalWrite(pinRelayCoil, HIGH);
        Serial.print("Preriscaldamento coil, fan al :");Serial.println(fanpwm);
        delay(5000);
        digitalWrite(pinRelayCoil,LOW);
        Serial.println("Ho spento la coil dopo il preriscaldamento");
        mantenimento=true;
  }

void aggiornaHW(){ //aggiorna i dati letti dai sensori, quindi rpm, temp, fotoresistenza.
   if (millis()- last > 200 ) {
    last=millis();
    light=getLight();
    temp=getAirTemp();
    fanrpm=getRpm();
    //Serial.println("Ho aggiornato i valori letti dai sensori!");
    }
  }
  
int getLight(){ //restituisce il valore letto in centesimi
      return map(analogRead(pinLR),0,4096,0,100);
  }

float getAirTemp(){
    sensors.requestTemperatures();
    float C=sensors.getTempCByIndex(0);// questo tipo di sensore può comunicare come una sorta di I2C, io ne uso uno quindi chiamo il primo
    return C;
  }
  
int getRpm(){
    start_time=millis();
    count=0; //pulisco contatore prima di attesa
    while((millis() - start_time)<=250){} //dopo 1/4 secondo
    return count*120 ;//ho ricevuto 2 segnali per giro del fan, moltiplico per 120 e ho rpm
  }
 
void setPWM(int pwmPerc){ // parametro come percentuale
      if(pwmPerc>=0 && pwmPerc<=100)
       ledcWrite(PWM_CHANNEL,map(pwmPerc,0,100,0,255));
  }

void pubblicaDati(){  // invia tramite mqtt dati di stato ogni 4 secondi 
  if (millis()- lastdata > 3000 ) {
    lastdata = millis();
     //invia % pwm e rpm della ventola
    snprintf (msg, 100, "%d" ,fanpwm);
    Serial.print("Pub fan pwm: ");
    Serial.print(msg);
    client.publish("fogduino/fan/pwm", msg);

    snprintf (msg, 100, "%d" ,fanrpm);
    Serial.print(" Pub fan rpm: ");
    Serial.println(msg);
    client.publish("fogduino/fan/rpm", msg);
    //invia resistenza della coil
    snprintf (msg, 100, "%.1f" ,temp);
    Serial.print("Pub AirTemp: ");
    Serial.println(msg);
    client.publish("fogduino/coil", msg);
    }
    
  }
void pubblicaStatus(){
  if (millis() - lastMsg > 4000 ) {
    lastMsg = millis();
    snprintf (msg, 100, "Status: isOn: %d, fan: %d, coil: %d" ,isOn,fan,coil);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("fogduino/status", msg);
    }
  }


void flood(){ //uso il servo per "pucciare" la coil e rifornirla di liquido da evaporare
      //spostati di 90 gradi, aspetta 1 secondo, poi raddrizza
  }

void mantieniErogazione(void * parameter){
        //se attivo il mantenimento allora eseguo
      for(;;){
          if(isOn && mantenimento){ //se arriva la disattivazione del mantenimento, mi autokillo
           //mantengo il fumo: ventola al massimo a meno che la temperatura non stia calando, ogni tot tempo flood sulla coil (che deve essere prima spenta)
          
          //#### debug
          setPWM(fanpwm);
          digitalWrite(pinRelayCoil,HIGH);
          Serial.print("#######FAN SET TO !!");Serial.println(fanpwm);
          delay(3000);
          digitalWrite(pinRelayCoil,LOW);
          vTaskDelay(500);
          /*Serial.print("Task1 running on core ");
          Serial.println(xPortGetCoreID());*/
            //necessario per watchdog di rtos altrimenti la task occupa tutte le risorse di cpu
          }
          vTaskDelay(20);
      }
  }

void counterRPM(){count++;} //procedura chiamata da interrupt su tachometer della ventola

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  sensors.begin();
  
  //salvo variabili ambientali di riferimento
  lightRef=getLight();
  tempRef=getAirTemp();
  
  //Settaggio per lettura rpm e pwm ventola
  pinMode(26,INPUT_PULLUP); //oltre al pullup lato HW, se setto il pin a pullup interno i valori sembrano più precisi
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOUTION);
  ledcAttachPin(pinFanPWM, PWM_CHANNEL);
  attachInterrupt(digitalPinToInterrupt(26),counterRPM,RISING); //tramite interrupt rilevo segnale da "tachimetro" del fan

  pinMode(pinRelayCoil, OUTPUT);

  TaskHandle_t Task1; //creazione task pinnato al secondo core
   xTaskCreatePinnedToCore(
    mantieniErogazione,      // Function that should be called
    "eroga fumo",    // Name of the task (for debugging)
    1000,               // Stack size (bytes)
    NULL,               // Parameter to pass
    1,                  // Task priority
    &Task1,               // Task handle
    0);         // Core you want to run the task on (0 or 1)
    
    reconnect();
    firstStart(); //moved to is on changing
}

void loop() {
  if (!client.connected()) {
    mantenimento=false; //se perdo connessione smetto di erogare
    reconnect();
    mantenimento=true; //quando riconesso, ricomincio (check se da fare preriscaldamento)
  }
  client.loop(); 
  if(isOn){  
    aggiornaHW();
    pubblicaStatus();
    pubblicaDati();
  }
}
