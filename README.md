# Fogduino

Fogduino è un progetto che vuole prototipare un generatore di fumo capace di interagire tramite MQTT. Applicando corrente sulla resistenza, essa si riscalda facendo evaporare il liquido contenuto nel cotone sulla quale è avvolta, generando fumo (Seguendo il principio di funzionamento di una sigaretta elettronica).
Una ventola investe con il suo flusso d'aria la resistenza, controllando la temperatura e evaporazione del liquido,convogliando il fumo generato verso un uscita dove una fotoresistenza legge la riduzione di luce rispetto allo standard ambientale.
Pubblicazione tramite mqtt di dati relativi alla riduzione di "visibilità" della fotoresistenza quando il fumo viene erogato, temperatura rilevata e parametri di funzionamento, oltre alla ricezione di comandi da remoto.


## Hardware

- ESP8266 (ESP32) NodeMCU
- PC Fan (7x7cm, 12v,0,4A; 4 pin: +,-,pwm,rpm)
- Resistenza in lega da 1.3 Ohm, in lega Ni80, avvolta su cotone imbevuto di liquido
- 1x Relè 5VDC JQC-3FF-S-Z 
- Sensore temperatura ds18b20
- Fotoresistore
- Sensore IR di fiamma KY-026
- Vecchio alimentatore da PC 

## Schema
![Disegno dello schema progetto](https://github.com/agandini/fogduino/blob/main/img/fogduino_bb.jpg?raw=true)

Note: la Coil è da circa 1.3 Ohm, applicando 5v otteniamo circa 4 A, quindi siamo sui 20W di potenza.\
Non sono riuscito a reperire informazioni online riguardo a "cosa esca" fisicamente dall'uscita per contare gli rpm dalla ventola, ne a misurarla con un multimetro. Facendo un pò di prove, e seguendo questo [schema](https://esp32.com/viewtopic.php?f=19&t=13679&sid=1309e637ad6137b20bef39852a5a2172&start=20) sembra arrivare un impulso a 12v e inserendo le resistenze per creare un piccolo partitore riesco a leggere i valori su un max di circa 3v, perfetti per l'esp32.\




## Links:
  - Board usata [ESP32](https://www.amazon.it/AZDelivery-sviluppo-successivo-versione-compatibile/dp/B08BTQ57ZV/ref=sr_1_5?__mk_it_IT=%C3%85M%C3%85%C5%BD%C3%95%C3%91&crid=7S1HMAXV0GP4&dchild=1&keywords=esp32+azdelivery&qid=1616601484&sprefix=esp32+azd%2Caps%2C245&sr=8-5) (Il venditore ora propone una versione "graficamente" diversa, ma dovrebbe essere la stessa)
  - MQTT - http://mqtt.org/
  - PubSubClient (MQTT) - https://pubsubclient.knolleary.net/
  - WiFi -https://www.arduino.cc/en/Reference/WiFi
  - [OneWire](https://www.arduino.cc/reference/en/libraries/onewire/) & [DallasTemperature](https://www.arduinolibraries.info/libraries/dallas-temperature)
 
