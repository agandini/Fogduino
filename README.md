# Fogduino

Fogduino è un progetto che vuole prototipare un generatore di fumo capace di interagire tramite MQTT. Applicando corrente sulla resistenza, essa si riscalda facendo evaporare il liquido contenuto nel cotone sulla quale è avvolta, generando fumo (Seguendo il principio di funzionamento di una sigaretta elettronica). Una ventola investe con il suo flusso d'aria la resistenza, mantenendo "costante" la temperatura e evaporazione del liquido,convogliando il fumo generato verso un uscita.
-> In test calcolo densità del fumo tramite fotoresistenza (per ora difficoltà in quanto fumo non sufficientemente denso per ottenere valori analogici consistenti)
MQTT: gestione del prototipo da remoto ( o una serie di essi ), trasmissione dati su qualità del fumo generato, temperature, ecc..) //da implementare

## Hardware

- ESP8266 (ESP32) NodeMCU
- PC Fan (7x7cm, 12v,0,4A; in test)
- Resistenza in lega (in testing diversi valori di Ohm, leghe di metalli, lunghezze e dimensioni per ottimale riscaldamento del liquido)
- 3x Relè 5VDC JQC-3FF-S-Z
- Pompa liquido (in testing, forse non necessaria)
- in agiornamento

## Links:
  - Board usata [ESP32](/https://www.amazon.it/AZDelivery-sviluppo-successivo-versione-compatibile/dp/B08BTQ57ZV/ref=sr_1_5?__mk_it_IT=%C3%85M%C3%85%C5%BD%C3%95%C3%91&crid=7S1HMAXV0GP4&dchild=1&keywords=esp32+azdelivery&qid=1616601484&sprefix=esp32+azd%2Caps%2C245&sr=8-5) (Il venditore ora propone una versione "graficamente" diversa, ma dovrebbe essere la stessa)
  - MQTT - http://mqtt.org/
  - PubSubClient (MQTT) - https://pubsubclient.knolleary.net/
  - WiFi -https://www.arduino.cc/en/Reference/WiFi
  - ESP8266WiFi - https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
  - TaskScheduler - https://github.com/arkhipenko/TaskScheduler
  - da inserire
