# impcount

## Short description
impcount is a simple Arduino impulse counter. Connect S0 counters to Arduino's digital pins and this sketch will print the duration between two signals to the serial port. Interrupts are used when the pin supports it, so the `loop()` function may do other things.

## Project status
This project is no longer maintained. It was developed on the FHEM forum in 2013 and moved to GitHub in 2015.

## Releases
### 1.0 (24.06.2013)
#### Arduino
Hier läuft hier das kleine Programm impcount, das auf digitalen Ports lauscht, beim Anliegen eines Signals die Dauer seit dem vorigen Signal berechnet und diese auf die serielle Schnittstelle schreibt.

Das Lauschen könnte über Polling ("Busy Waiting") implementiert werden, d.h. das Programm prüft regelmäßig, ob ein Signal anliegt. Der Arduino kennt aber auch das Konzept von Interrupts: damit wird automatisch beim Anliegen eines Signals eine Funktion aufgerufen. Interrupts gibt es nur für ausgewählte Pins, die von der Arduino-Hardware abhängen. Ich habe mich für einen Mega2560 entschieden, dieser erlaubt 6 Interrupt-Pins. Die kleineren Modelle haben 2, beim Modell Due sind alle Pins Interrupt-fähig.

Das Programm impcount verwendet Interrupts, sofern der Zähler an einem dafür geeigneten Pin angeschlossen ist. An allen anderen Pins wird per Busy Waiting verfahren. Wenn man nicht mehr Zähler anschließen möchte als Interrupt-Pins zur Verfügung stehen, kann man natürlich die `loop()`-Funktion auch anderweitig nutzen.

#### fhem
Jetzt müssen die Werte von der seriellen Schnittstelle noch in fhem eingelesen werden. Ich habe folgende Module gefunden, die mir hilfreich erscheinen.

#####ECMD
Mit [ECMD](http://www.fhemwiki.de/wiki/ECMD) habe ich es geschafft, die Daten vom Arduino abzufragen. Ich war damit aber nicht ganz zufrieden, weil ECMD für "Request/Response-like communication" gedacht ist. Das heißt, fhem muss regelmäßig aktiv Werte vom Arduino abfragen. Dieses Vorgehen ist völlig in Ordnung, wenn ein Sensor dauerhaft einen Wert liefert, wie z.B. ein Thermometer. Für Impulssignale passt es aber leider nicht, weil man zur richtigen Zeit abfragen müsste. Man kann natürlich die Daten auch auf dem Arduino zwischenspeichern; das Programm dort wird dadurch aber komplexer, und man muss sich dann auch um Pufferüberläufe kümmern, auf dem seriellen Port lauschen, etc.

#####WHR962
Im Foreneintrag zum Modul [WHR962](http://forum.fhem.de/index.php/topic,10290.msg57862.html#msg57862) habe ich gelernt, dass man wohl auch in FHEM automatisch auf das Eintreffen von Daten über die serielle Schnittstelle reagieren kann. Ich habe das Modul als Vorlage genommen und möchte an dieser Stelle herzlich beim Autor, Joachim, für die Veröffentlichung bedanken.

#####IMPCOUNT
Mit Hilfe von WHR962 habe ich ein Modul namens IMPCOUNT geschrieben, das auf meinen Anwendungsfall passt; es ist so weit verallgemeinert, dass es auch für andere impuls-basierte Zähler verwendet werden kann. Es passiert eigentlich nicht viel: die Werte des Arduino werden gepuffert, auf Wunsch umgerechnet und gespeichert. Details zur Implementierung sind im Modul dokumentiert.

####Konfiguration
Was jetzt noch fehlt, sind ein paar Einträge in der `fhem.cfg` und ein Skript zum Zeichnen von Diagrammen namens `my_impcount.gplot`.

Der Arduino-Sketch namens `s0_dummy_sender` ist zum Testen gedacht. Er gibt in regelmäßigen Abständen zufällige Werte aus, sodass das fhem-Modul `IMPCOUNT` auch ohne S0-Zähler getestet werden kann.

### 2.0 (28.09.2013)
Erweiterung von `impcount` um die Möglichkeit, Signale mit einer konfigurierbaren Dauer als ungültig zu markieren und herauszufiltern.

####Change Log
* Ausgabe der Impulsdauer (eines einzelnen Impulses) zusätzlich zur Impulsdistanz (zwischen zwei Impulsen)
* Angabe von minimal und maximal erlaubter Dauer je Pin
* Trennung von Impulserfassung und Ausgabe; das reduziert die Zeit, in der aufgrund von Ausgaben keine Impulse erfasst werden können
* Performance- und Interrupt-Optimierungen
* (optional) Ausgabe einer Statistik über die Dauer zwischen zwei Poll-Abfragen

## Performance
Tests mit Stromzählern haben gezeigt, dass die Verwendung von Interrupts in der Praxis selten notwendig sein sollte. Die vorliegenden Stromzähler liefern 1000 Impulse pro Wh, die Impulsdauer beträgt ca. 60ms. Der Arduino ist mit 16 MHz getaktet.
Die `loop()`-Methode des Arduino-Sketches liest nacheinander alle digitalen Eingänge ein und sendet im Fall eines Signalwechsels eine Nachricht über die serielle Schnittstelle. Wie lange dauert ein Durchlauf? Ich habe das bei der Entwicklung mal gemessen: ca. 80 µs im Leerlauf und ca. 600 µs, falls ein Signal gemessen und übermittelt wird (das entspricht also etwa 1280 bzw. 9600 Maschinenbefehlen).
Lassen wir jetzt mal je 1 Impuls von 20 Zählern direkt nacheinander eintreffen. Wenn die einfach stumpf nacheinander abgearbeitet werden, dauert das also 12 ms. Man könnte natürlich noch optimieren, indem man erst alle Signale eines Durchlaufs sammelt und gemeinsam über die serielle Schnittstelle schickt. Interrupts sind eine weitere Möglichkeit. Bei einer Impulsdauer von 60ms ist das aber nicht notwendig.

## Discussion on the FHEM forum
* [Wie kann ich einige S0-Zähler mit fhem auf einer Fritz!Box 7390 auslesen?](http://forum.fhem.de/index.php?topic=13155.0)
* [Stromzähler mit S0 Schnittstelle nochmals](http://forum.fhem.de/index.php?topic=19285.0)
* [Firmata UND Impcount and einem Arduino?](http://forum.fhem.de/index.php?topic=15245.0)

