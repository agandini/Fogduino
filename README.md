# Fogduino

Fogduino è un progetto che vuole prototipare un generatore di fumo capace di interagire tramite MQTT. Applicando corrente sulla resistenza, essa si riscalda facendo evaporare il liquido contenuto nel cotone sulla quale è avvolta, generando fumo (Seguendo il principio di funzionamento di una sigaretta elettronica). Una ventola investe con il suo flusso d'aria la resistenza, mantenendo "costante" la temperatura e evaporazione del liquido,convogliando il fumo generato verso un uscita.
-> In test calcolo densità del fumo tramite fotoresistenza (per ora difficoltà in quanto fumo non sufficientemente denso per ottenere valori analogici consistenti)
MQTT: gestione del prototipo da remoto ( o una serie di essi ), trasmissione dati su qualità del fumo generato, temperature, ecc..) //da implementare

## Hardware

- ESP8266 (ESP32) NodeMCU
- PC Fan (7x7cm, 12v,0,4A; in test)
- Resistenza in lega (in testing diversi valori di Ohm, lunghezze e dimensioni per ottimale riscaldamento del liquido. Leghe di metallo in considerazione: Accialo 316L, khantal A1, Ni80 in diversi AWG)
- 3x Relè 5VDC JQC-3FF-S-Z
- Pompa liquido (in testing, forse non necessaria)

## Links:

  - da inserire
