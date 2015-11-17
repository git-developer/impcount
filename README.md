# impcount

## Short description
impcount is a simple Arduino impulse counter. Connect S0 counters to Arduino's digital pins and this sketch will print the duration between two signals to the serial port. Interrupts are used when the pin supports it, so the `loop()` function may do other things.

## Project status
This project is no longer maintained. It was developed on the FHEM forum in 2013 and moved to GitHub in 2015.

## Releases
### 1.0 (24.06.2013)
#### Arduino
Hier l�uft hier das kleine Programm impcount, das auf digitalen Ports lauscht, beim Anliegen eines Signals die Dauer seit dem vorigen Signal berechnet und diese auf die serielle Schnittstelle schreibt.

Das Lauschen k�nnte �ber Polling ("Busy Waiting") implementiert werden, d.h. das Programm pr�ft regelm��ig, ob ein Signal anliegt. Der Arduino kennt aber auch das Konzept von Interrupts: damit wird automatisch beim Anliegen eines Signals eine Funktion aufgerufen. Interrupts gibt es nur f�r ausgew�hlte Pins, die von der Arduino-Hardware abh�ngen. Ich habe mich f�r einen Mega2560 entschieden, dieser erlaubt 6 Interrupt-Pins. Die kleineren Modelle haben 2, beim Modell Due sind alle Pins Interrupt-f�hig.

Das Programm impcount verwendet Interrupts, sofern der Z�hler an einem daf�r geeigneten Pin angeschlossen ist. An allen anderen Pins wird per Busy Waiting verfahren. Wenn man nicht mehr Z�hler anschlie�en m�chte als Interrupt-Pins zur Verf�gung stehen, kann man nat�rlich die `loop()`-Funktion auch anderweitig nutzen.

#### fhem
Jetzt m�ssen die Werte von der seriellen Schnittstelle noch in fhem eingelesen werden. Ich habe folgende Module gefunden, die mir hilfreich erscheinen.

#####ECMD
Mit [ECMD](http://www.fhemwiki.de/wiki/ECMD) habe ich es geschafft, die Daten vom Arduino abzufragen. Ich war damit aber nicht ganz zufrieden, weil ECMD f�r "Request/Response-like communication" gedacht ist. Das hei�t, fhem muss regelm��ig aktiv Werte vom Arduino abfragen. Dieses Vorgehen ist v�llig in Ordnung, wenn ein Sensor dauerhaft einen Wert liefert, wie z.B. ein Thermometer. F�r Impulssignale passt es aber leider nicht, weil man zur richtigen Zeit abfragen m�sste. Man kann nat�rlich die Daten auch auf dem Arduino zwischenspeichern; das Programm dort wird dadurch aber komplexer, und man muss sich dann auch um Puffer�berl�ufe k�mmern, auf dem seriellen Port lauschen, etc.

#####WHR962
Im Foreneintrag zum Modul [WHR962](http://forum.fhem.de/index.php/topic,10290.msg57862.html#msg57862) habe ich gelernt, dass man wohl auch in FHEM automatisch auf das Eintreffen von Daten �ber die serielle Schnittstelle reagieren kann. Ich habe das Modul als Vorlage genommen und m�chte an dieser Stelle herzlich beim Autor, Joachim, f�r die Ver�ffentlichung bedanken.

#####IMPCOUNT
Mit Hilfe von WHR962 habe ich ein Modul namens IMPCOUNT geschrieben, das auf meinen Anwendungsfall passt; es ist so weit verallgemeinert, dass es auch f�r andere impuls-basierte Z�hler verwendet werden kann. Es passiert eigentlich nicht viel: die Werte des Arduino werden gepuffert, auf Wunsch umgerechnet und gespeichert. Details zur Implementierung sind im Modul dokumentiert.

####Konfiguration
Was jetzt noch fehlt, sind ein paar Eintr�ge in der `fhem.cfg` und ein Skript zum Zeichnen von Diagrammen namens `my_impcount.gplot`.

Der Arduino-Sketch namens `s0_dummy_sender` ist zum Testen gedacht. Er gibt in regelm��igen Abst�nden zuf�llige Werte aus, sodass das fhem-Modul `IMPCOUNT` auch ohne S0-Z�hler getestet werden kann.

### 2.0 (28.09.2013)
Erweiterung von `impcount` um die M�glichkeit, Signale mit einer konfigurierbaren Dauer als ung�ltig zu markieren und herauszufiltern.

####Change Log
* Ausgabe der Impulsdauer (eines einzelnen Impulses) zus�tzlich zur Impulsdistanz (zwischen zwei Impulsen)
* Angabe von minimal und maximal erlaubter Dauer je Pin
* Trennung von Impulserfassung und Ausgabe; das reduziert die Zeit, in der aufgrund von Ausgaben keine Impulse erfasst werden k�nnen
* Performance- und Interrupt-Optimierungen
* (optional) Ausgabe einer Statistik �ber die Dauer zwischen zwei Poll-Abfragen

## Discussion on the FHEM forum
* [Wie kann ich einige S0-Z�hler mit fhem auf einer Fritz!Box 7390 auslesen?](http://forum.fhem.de/index.php?topic=13155.0)
* [Stromz�hler mit S0 Schnittstelle nochmals](http://forum.fhem.de/index.php?topic=19285.0)
* [Firmata UND Impcount and einem Arduino?](http://forum.fhem.de/index.php?topic=15245.0)

