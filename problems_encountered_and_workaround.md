# problemi, difficoltà e soluzioni alternative riscontrate durante sviluppo

## Lato Hardware
- Inizialmente, avrei voluto poter calcolare quanto la coil sia stata calda misurando la sua variazione di resistenza rispetto alla temperatura ambiente dalla quale si parte (coefficiente di temperatura).
Purtoppo dato che la variazione è davvero minima ( millesimi di ohm) e l'idea che prevedeva di usare l'esp32 come un [ohmmetro](https://create.arduino.cc/projecthub/iasonas-christoulakis/how-to-make-an-arduino-ohm-meter-90fda8) non è in grado di leggere sia varazioni così piccole che leggere proprio resistenze dal valore così basso (ho testato con una resistenza da circa 1 ohm ottendo scarsi risultati, e il surriscaldamento della stessa), è molto difficile poter calcolare la temperatura approssimativa in questo modo.
-> come workaround solution intendo usare un sensore di temperatura ds18b20 posto dopo la resistenza, calcolando quindi la temperatura dell'aria che lo investe.

- Ventola: uso una vecchia ventola per processori desktop, con 4 pin (+, - , pwm, rpm) che controllo con il pin pwm e leggo come "feedback" dal pin rpm le pulsazioni di quando un giro della ventola viene effettuato(non sono sicuro sia 1 giro, alcuni modelli di ventole mandano 2 impulsi ogni rotazione).
[Ho preso spunto da qui](https://www.youtube.com/watch?v=UJK2JF8wOu8), nel video viene presentato su un arduino (a 5v dunque)e da una mia prova sembra funzionare perfettamente, ho provato a adattarlo per leggere da un esp32 (a 3.3v):in particolare ho messo in serie 2 resitenze ( una da 10k e una da 3k) in pullup su 3.3v e leggendo il segnale sul piede della resistenza da 3k (sembrano valori sensati quelli letti)

- Alimentazione liquido: la coil andrà inevitabilmente a esaurire il liquido da evaporare nel tempo, quindi per bagnarla avevo inizialmente pensato e costruito una piccola pompa a partire da un motorino a 5v.
Purtoppo mi è difficile gestirlo e manderebbe troppo liquido sulla resistenza, quindi l'idea per bagnare la coil è quella di "legarla" a un servomotore, che ogni tot tempo ruota di 90 gradi la resistenza "pucciandola" in una vaschettina contenete liquido sotto di essa.
//da implementare

## Lato Software
- dual core: ho pensato di sfruttare i due core dell'esp32 per poter far girare sul secondo core la routine che mantiene il fumo in erogazione, ovvero setta la ventola a una certa velocita, mantiene la corrente sulla resistenza e ogni tot secondi la bagna. Ho un problema nella gestione del watchdog, mi obbliga a inserire un dealy per evitare di fermare la board.
In questo modo l'altro core eseguira tutto il resto: comunicazione mqtt, lettura sensori. //in testing 
- altra opzione sarebbe usare la libreria task scheduler, ma non l'ho provata (sarà possibile usarlo su 2 core?)
