# ChaosUhr
ChaosBlinkiBuntiUhr für ChaosUhr (https://www.thingiverse.com/thing:4152063 by BishobMarthen)

Hallo zusammen, 

Anleitung zur Uhr siehe Thingiverse Link oben.

Ich habe die Uhr um ein paar Animationen und Bilder erweitert.
Zudem habe ich den Intervall zum Anzeigen der Uhrzeit angepasst. 

Intervall ist nun: 
Uhrzeit - random Animation im Wechsel.

In Arduino werden folgende Bibliotheken benötigt:

FastLED;
FastLED_NeoMatrix;

Zudem muss das ESP8266 Board hinzugefügt werden. 

Eine kurze Anleitung als PDF ist in den Dateien zu finden.

WebServer incl. OTA hab ich mit übernommen, somit muss die Uhr zum Flashen nicht immer an USB angeschlossen werden. 
Einfach http://<hostname>.local im Browser eingeben und dann die entsprechende Datei wählen. 
In Arduino kann man unter Sketch - Kompilierte Binärdatei exportieren wird eine *.bin in den Sketch Ordner erstellt die dann über OTA hochgeladen werden kann.

Der erste Flash muss jedoch noch über USB geflasht werden, um OTA zu aktivieren. 

Wer testen will, nur zu.
Ebenfalls kann das Sketch auch gern bearbeitet und überarbeitet werden.

Ich bin kein Programmiere und daher sieht der Code aus wie er aussieht.
