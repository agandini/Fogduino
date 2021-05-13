/*
This file is part of fogduino which is released under GPLv3 license.
See file LICENSE  or go to https://github.com/agandini/fogduino/blob/main/LICENSE for full license details.
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 15

//gestione pwm ventola
#define PWM_FREQUENCY 25000
#define PWM_CHANNEL 0
#define PWM_RESOUTION 8 //duty cycle con 2^8 (0 - 255)

// variabili di conessione wifi/mqtt
const char * ssid = "***";
const char * password = "***";
const char * mqtt_server = "broker.hivemq.com";

//variabili gestione pin 
const int pinRelayCoil = 17; //relè resistenza
const int pinTemp = 15; //sensore b18 temperatura 
const int pinFanPWM = 27; //pwm verso il fan
const int pinFanRPM = 26; //rpm dal fan
const int pinLR = 35; //fotoresistore
const int pinFlame = 25; //flame sensor

//variabili gestione temp con onewire
OneWire oneWire(pinTemp);
DallasTemperature sensors( & oneWire);

unsigned long start_time;

WiFiClient espClient;
PubSubClient client(espClient);

char msg[100];
volatile int fanpwm, count, fanrpm = 0;
volatile float light, lightRef, temp, tempRef = 0;
volatile bool isOn, fire, mantenimento = false;
bool coldStart = true;

unsigned long last = 0;
unsigned long lastMsg = 0;
unsigned long lastdata = 0;
unsigned long lastOn = 0;

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
}

void callback(char * topic, byte * payload, unsigned int length) {
    Serial.print("Ricevuto messaggio da [");
    Serial.print(topic);
    Serial.print("] ");
    String messaggio = "";
    for (int i = 0; i < length; i++) {
        messaggio += (char) payload[i];
        Serial.print((char) payload[i]);
    }
    int mex = messaggio.toInt(); //mi aspetto messaggi numerici
    if (strcmp(topic, "fogduino/setpwm") == 0 && isOn) {
        fanpwm = mex;
    } else {
        switch (mex) {
        case -1: //spegni tutto
            isOn = false;
            digitalWrite(pinRelayCoil, HIGH);
            client.publish("fogduino/status", "Mi sono spento!");
            break;
        case 1: //accendi
            isOn = true;
            client.publish("fogduino/status", "Mi sono acceso!");
            break;
        case 2: //spegni mantenimento 
            mantenimento = false;
            client.publish("fogduino/status", "Modalità manuale");
            break;
        case 3: //accendi mantenimento
            mantenimento = true;
            client.publish("fogduino/status", "Modalità automatica");
            break;
        case 4: //accendi relay
            if (!mantenimento && isOn){
              digitalWrite(pinRelayCoil, LOW);
            }
            break;
        case 5: //spegni relay
            digitalWrite(pinRelayCoil, HIGH);
            break;
        default:
            Serial.println("comando non valido");
            break;
        }
    }
    Serial.println();
}

void reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connesione WiFi persa!");
        setup_wifi();
    }
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-Fogduino"; //crea id client mqtt
        //clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str(), NULL, NULL, "fogduino/status", 1, true, "Connessione persa!Mi riconnetterò a breve...")) {
            Serial.println("connected"); // Once connected, publish an announcement...
            client.publish("fogduino/status", "Connessione riuscita");
            client.subscribe("fogduino/ctrl"); //sottoscrizione, la callback si invoca alla ricezione di qualcosa su questo topic
            client.subscribe("fogduino/setpwm"); //sottoscrizione, la callback si invoca alla ricezione di qualcosa su questo topic
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" riprovo tra 5 secondi...");
            delay(5000);
        }
    }
}

void firstStart() {
    //se prima accensione, si ipotizza a freddo (non dopo un reset) allora riscaldo un pò la coil
    client.publish("fogduino/status", "Preriscaldamento in corso...");
    fanpwm = 30;
    setPWM(fanpwm);
    digitalWrite(pinRelayCoil, LOW);
    Serial.print("Preriscaldamento coil, fan al :");
    Serial.println(fanpwm);
    delay(2000);
    digitalWrite(pinRelayCoil, HIGH);
    Serial.println("Ho spento la coil dopo il preriscaldamento");
    client.publish("fogduino/status", "Modalità mantenimento avviata");

    mantenimento = true;
    coldStart = false;
}

void aggiornaHW() { //aggiorna i dati letti dai sensori, quindi rpm, temp, fotoresistenza.
    if (millis() - last > 100) {
        last = millis();
        light = getLight();
        temp = getAirTemp();
        fanrpm = getRpm(); //la getRPM "ruba" 250 ms circa di tempo, totale operaz. di aggiornamento HW circa 500ms
    }
}

float getLight() { //restituisce il valore letto in centesimi
    return map(analogRead(pinLR), 0, 4096, 0, 100);
}

float getAirTemp() {
    sensors.requestTemperatures();
    float C = sensors.getTempCByIndex(0); // questo tipo di sensore può comunicare come una sorta di I2C, io ne uso uno quindi chiamo il primo
    return C;
}

int getRpm() {
    start_time = millis();
    count = 0; //pulisco contatore prima di attesa
    while ((millis() - start_time) <= 125) {} //dopo 1/4 secondo
    return count * 240; //ho ricevuto 2 segnali per giro del fan, moltiplico per 120 e ho rpm
}

void setPWM(int pwmPerc) { // parametro come percentuale
    ledcWrite(PWM_CHANNEL, map(constrain(pwmPerc, 0, 100), 0, 100, 0, 255));
}

void pubblicaDati() { // invia tramite mqtt dati di stato ogni 3 secondi 
    if (millis() - lastdata > 2000) {
        lastdata = millis();
        //invia % pwm e rpm della ventola
        snprintf(msg, 100, "%d", fanpwm);
        Serial.print("Pub fan pwm: ");
        Serial.print(msg);
        client.publish("fogduino/fan/pwm", msg);

        snprintf(msg, 100, "%d", fanrpm);
        Serial.print(" Pub fan rpm: ");
        Serial.println(msg);
        client.publish("fogduino/fan/rpm", msg);

        //invia temperatura attuale e prima rilevazione
        snprintf(msg, 100, "%.1f", temp);
        Serial.print("Pub AirTemp: ");
        Serial.println(msg);
        client.publish("fogduino/coil", msg);

        snprintf(msg, 100, "%.1f", tempRef);
        Serial.print("Pub TempREF: ");
        Serial.println(msg);
        client.publish("fogduino/tempRef", msg);

        //invia luce attuale e prima rilevazione
        snprintf(msg, 100, "%.1f", lightRef);
        Serial.print("Pub lightRef: ");
        Serial.println(msg);
        client.publish("fogduino/lightRef", msg);

        snprintf(msg, 100, "%d", (int) light);
        Serial.print("Pub light: ");
        Serial.println(msg);
        client.publish("fogduino/light", msg);
    }

}

void mantieniErogazione(void * parameter) {
    //se attivo il mantenimento allora eseguo
    int msecOn, msecOff = 1000;
    int autopwm = 50;
    for (;;) {
        if (isOn && mantenimento) { //se arriva la disattivazione del mantenimento, mi autokillo
            //mantengo il fumo: ventola al massimo a meno che la temperatura non stia calando
            Serial.println(tempRef - temp);
            if (temp - tempRef < 2) {  // se siamo a meno di 2 gradi di differenza rispetto all'ambiente
                autopwm = 30;
                msecOn = 700;
                msecOff = 400;
            } else if (temp - tempRef < 4) { // se siamo a meno di 4 gradi di differenza rispetto all'ambiente
                autopwm = 50;
                msecOn = 700;
                msecOff = 600;
            } else if (temp - tempRef < 6) { // se siamo a meno di 6 ...
                autopwm = 70;
                msecOn = 700;
                msecOff = 1000;
            } else { // oltre 6 gradi di differenza rispetto all'ambiente dovrei aver raggiunto un punto abbastanza caldo da poter mantenere la temperatura con attivazioni piu lente
                autopwm = 100;
                msecOn = 700;
                msecOff = 1400;
            }
            fanpwm = autopwm;
            setPWM(fanpwm);
            digitalWrite(pinRelayCoil, LOW);
            vTaskDelay(msecOn);
            digitalWrite(pinRelayCoil, HIGH);
            vTaskDelay(msecOff);
        }
        setPWM(fanpwm);
        vTaskDelay(20); //necessario per watchdog di rtos altrimenti la task occupa tutte le risorse di cpu
    }
}

void counterRPM() {
    count++;
} //procedura chiamata da interrupt su tachometer della ventola
void flame() {
    fire = true;
} //interrupt su sensore fiamma

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    sensors.begin();

    //salvo variabili ambientali di riferimento
    lightRef = (getLight() + getLight() + getLight()) / 3;
    tempRef = getAirTemp();

    //Settaggio per lettura rpm e pwm ventola
    pinMode(pinFanRPM, INPUT_PULLUP); //oltre al pullup lato HW, se setto il pin a pullup interno i valori sembrano più precisi
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOUTION);
    ledcAttachPin(pinFanPWM, PWM_CHANNEL);
    attachInterrupt(digitalPinToInterrupt(pinFanRPM), counterRPM, RISING); //tramite interrupt rilevo segnale da "tachimetro" del fan

    //settaggio sensore fiamma
    pinMode(pinFlame, INPUT);
    attachInterrupt(digitalPinToInterrupt(pinFlame), flame, RISING); //tramite interrupt rilevo segnale da flame sensor

    //settaggio relay
    pinMode(pinRelayCoil, OUTPUT);
    digitalWrite(pinRelayCoil, HIGH); //usato in NO (normal open), il relay è low trigger quindi con segnale alto rimane interrotto.

    TaskHandle_t Task1; //creazione task pinnato al secondo core
    xTaskCreatePinnedToCore(
        mantieniErogazione, // Function that should be called
        "eroga fumo", // Name of the task (for debugging)
        1000, // Stack size (bytes)
        NULL, // Parameter to pass
        1, // Task priority
        &
        Task1, // Task handle
        0); // Core you want to run the task on (0 or 1)

    reconnect(); //connetti a mqtt
}

void loop() {

    if (!client.connected()) {
        mantenimento = false; //se perdo connessione smetto di erogare
        reconnect();
    }
    client.loop();
    aggiornaHW();

    if (isOn) {
       // if (coldStart) firstStart();
        pubblicaDati();
    }

    if (fire) { //in caso di fiamma rilevata
        isOn = false;
        setPWM(fanpwm = 100);
        digitalWrite(pinRelayCoil, HIGH);
        client.publish("fogduino/status", "Fiamma rilevata! Richiedo aiuto!");
        client.unsubscribe("fogduino/ctrl");
        for (;;) { // stallo fino a riavvio
            delay(4000);
            client.publish("fogduino/status", "Stall on max fan mode...");
            aggiornaHW();
            pubblicaDati();
        }
    }
    delay(10);
}
