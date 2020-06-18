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
WiFiManager;
Zudem muss das ESP8266 Board hinzugefügt werden. 

Um es denjenigen einfacher zu machen, habe ich eine fertige .bin erstellt, die einfach nur geflasht werden muss. 
Dazu einfach die zip Datei herunterladen. Danach einfach die ZIP entpacken, FlashESP8266 im Ordner ausführen, COM und Image wählen und auf flashen klicken. 

Danach sollte ein neues WLAN Netzwerk "ChaosUhr" angezeigt werden.
Mit diesem verbinden und es sollte direkt die Landingpage laden. 
Unter Configure WiFi das WLAN wählen und das Passwort eintragen. Danach auf speichern. 

Es kann sein dass man den Wemos danach nochmal kurz vom Strom nehmen muss, danach sollte er sich dann aber mit dem WLAN verbinden und das Sketch loslegen. 

Sobald das Sketch beim laden kein WLAN findet sich nicht verbinden kann oder man die Daten falsch eingegeben hat, lädt er wieder den AP Modus und man kann die WLAN Daten erneut eingeben. 

Ich weiß auch nicht so ganz, wie er sich verhält, wenn zuvor schon WLAN Daten auf dem ESP gespeichert wurden. 
Evtl. muss man dann erst mal den Flash komplett löschen

WebServer incl. OTA hab ich mit übernommen, somit muss die Uhr zum Flashen nicht immer an USB angeschlossen werden. 
Einfach http://<hostname>.local im Browser eingeben und dann die entsprechende Datei wählen. 
In Arduino kann man unter Sketch - Kompilierte Binärdatei exportieren wird eine *.bin in den Sketch Ordner erstellt die dann über OTA hochgeladen werden kann.

Der erste Flash muss jedoch noch über USB geflasht werden, um OTA zu aktivieren. 

Wer testen will, nur zu.
Ebenfalls kann das Sketch auch gern bearbeitet und überarbeitet werden.

Ich bin kein Programmiere und daher sieht der Code aus wie er aussieht.
