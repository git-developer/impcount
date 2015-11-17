################################################################################
# FHEM-Modul f�r impulsbasierte Z�hler
# ====================================
#
# Autor:   fhem-user ( http://forum.fhem.de/index.php?t=email&toi=1713 )
# Version: 1.0
# Datum:   24.06.2013
# 
#
# Beschreibung
# ============
#
# Szenario
# --------
#
#   +------+   Signal   +----------+    ID:Dauer   +-----------------+
#   |Z�hler| ---------> |Controller|   ----------> | Ger�t ---> fhem |
#   +------+            +----------+               +-----------------+
#
# Ein oder mehrere impulsbasierte Z�hler (z.B. S0-Stromz�hler) sind an einem
# Mikrocontroller (z.B. Arduino) angeschlossen; der Controller nimmt den
# Impuls auf und bestimmt die Dauer seit dem vorigen Impuls. Die Dauer wird
# zusammen mit einer ID f�r den Z�hler �ber ein Ger�t (z.B. die serielle
# Schnittstelle) an dieses FHEM-Modul gesendet. Hier wird die Dauer in eine
# Z�hler-abh�ngige Einheit (z.B. kWh) umgerechnet und gespeichert.
#
# Eine Besonderheit dieses Moduls ist, dass es den Controller nicht aktiv
# abfragt (sog. "Polling"), sondern auf das Eintreffen von Daten wartet und dann
# automatisch t�tig wird. Das ist bei impulsbasierten Z�hlern sinnvoll, weil sie
# nicht dauerhaft einen Wert zur Verf�gung stellen.
#
# Anwendung
# ---------
# Die Default-Werte in diesem Modul sind ausgelegt f�r
#
# - S0-Stromz�hler mit 1000 Impulsen pro Kilowattstunde
# - �bertragung der Dauer in Millisekunden
# - Speicherung der Werte in Kilowattstunden
#
# Das kann aber durch Ver�nderung der Modul-Parameter angepasst werden.
# Beispiele:
#
# - Z�hler mit 2000 Impulsen / kWh:           signals_per_unit := 2000
# - Auswertung in Wh statt kWh:               unit_factor := 0.001
# - Der Sender liefert die Dauer in Sekunden: sender_time_scale := 1
# - Umwandlung der Werte in Kilowatt-Tage:    user_time_scale := 86400
#
# Diese Konfigurierbarkeit erlaubt auch die Anbindung anderer Z�hler
# (z.B. Wasser oder Gas).
#
#
# Implementierungsdetails
# =======================
# 
# Datensatz-Format
# ----------------
# Ein Datensatz hat folgende Struktur:
#
#   <ID><Trenner><Dauer><Terminator>
#
#  ID:         Beliebige Zeichenfolge
#  Trenner:    Regul�rer Ausdruck, Default: Doppelpunkt (':')
#  Dauer:      Dauer als positive Zahl
#  Terminator: Regul�rer Ausdruck, Default: Whitespace ('\s')
#
# Beispiel f�r 5 Datens�tze (Trenner: Doppelpunkt, Terminator: Zeilenumbruch):
#   1:1000
#   2:750
#   1:900
#   1:650
#   2:1150
#
# Parameter
# ---------
# Die Parameter dieses Moduls k�nnen in FHEM per 'get' ausgelesen
# und per 'set' ver�ndert werden.
#
# * key_prefix:        Pr�fix f�r die ID.
#                      Default-Wert: 'id-'
#    Dieses Pr�fix wird der ID in den FHEM-Readings und im Log vorangestellt.
# 
# * value_separator:   Trenner.
#                      Default-Wert: ':'
#    Details: siehe 'Datensatz-Format'
#
# * record_separator:  Terminator.
#                      Default-Wert: '\s'
#    Details: siehe 'Datensatz-Format'
# 
# * sender_time_scale: Zeiteinheit des Senders
#                      Default-Wert: 0.001 (Millisekunden)
#    Gibt die Einheit der "Dauer"-Werte des Senders an; der Wert wird
#    als Faktor zu 1 Sekunde interpretiert. "1" entspricht also einer
#    Sekunde, der Default-Wert "0.001" entspricht einer Millisekunde.
#
# * user_time_scale:   Zeiteinheit f�r die Weiterverarbeitung.
#                      Default-Wert: 3600
#    Gibt die Einheit der "Dauer"-Werte in diesem Modul und FHEM an; der Wert
#    wird als Faktor zu 1 Sekunde interpretiert. "1" entspricht also einer
#    Sekunde, der Default-Wert "3600" entspricht einer Stunde.
# 
# * signals_per_unit:  Signale pro Einheit des Z�hlers
#                      Default-Wert: 1000
#    Gibt die Anzahl der Signale an, die der Z�hler sendet, bis 1 Einheit
#    erreicht ist. Der Default-Wert "1000" entspricht also 1000 Signalen f�r
#    1 Einheit (z.B. 1 kWh).
# 
# * unit_factor:       Skalierung der Einheit f�r die Weiterverarbeitung
#                      Default-Wert: 1
#    Gibt einen Faktor zur Skalierung der Einheit des Senders an.
#    Der Default-Wert "1" f�hrt zur �bernahme der Werte vom Sender;
#    der Wert "0.001" entspricht z.B. einer Skalierung von kWh auf Wh.
# 
# * max_value:        Maximum f�r g�ltige Werte.
#                     Default-Wert: 3.68
#    Werte, die �ber 'max_value' liegen, werden verworfen. 
#    Hintergrund: Hin und wieder kann es vorkommen, dass fehlerhafte Werte
#    ausgelesen werden, z.B. beim Anschlie�en eines Ger�tes. Sehr gro�e Werte
#    f�hren zu Problemen beim Zeichnen von Diagrammen, weil eine unpassende
#    Skala verwendet wird. Die Angabe eines Maximalwertes erlaubt es,
#    solche Fehler zu entdecken.
#    Der Default-Wert 3.68 ist f�r einphasige S0-Stromz�hler vorgesehen
#    (Maximal 16A * 230V = 3.68 kW).
# 
# * buffer_size:      Anzahl der Zeichen des Lese-Puffers.
#                     Default-Wert: 20
#    Je nach Sender und Ger�t wird ein Datensatz in mehreren Fragmenten
#    �bertragen; dieses Modul wird bei jedem Eintreffen eines Fragmentes
#    benachrichtigt. Deshalb werden alle Fragmente zun�chst in einem Puffer
#    gespeichert. Die Verarbeitung erfolgt erst, wenn der Puffer einen
#    vollst�ndigen Datensatz enth�lt. Der Puffer muss deshalb mindestens so gro�
#    wie ein (maximal gro�er) Datensatz sein.
#
#
# Referenzen
# ----------
# Als Grundlage diente das Modul 'WHR962' von Joachim, Details unter
#  http://forum.fhem.de/index.php?t=msg&th=10290&goto=57862&rid=0#msg_57862
#
# Weitere Informationen finden sich im FHEM-Forum unter
#  http://forum.fhem.de/index.php?t=rview&goto=83400
#
#
# Copyright notice
# ================
#
#  (c) 2013
#  Copyright: fhem-user ( http://forum.fhem.de/index.php?t=email&toi=1713 )
#  All rights reserved
#
#  This script free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  The GNU General Public License can be found at
#  http://www.gnu.org/copyleft/gpl.html.
#
#  This script is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  This copyright notice MUST APPEAR in all copies of the script!
#
################################################################################

package main;

use strict;
use warnings;

# Parameter dieses Moduls
my %module_parameters = (
  key_prefix                => "id-",
  value_separator           => ":",
  record_separator          => "\\s",
  sender_time_scale         => 0.001,
  user_time_scale           => 3600,
  signals_per_unit          => 1000,
  unit_factor               => 1,
  max_value                 => 3.68,
  buffer_size               => 20
);

my $buffer = "";

##
# Wird beim Laden des Moduls von FHEM aufgerufen
##
sub IMPCOUNT_Initialize {
  my ($hash) = @_;
  require "$attr{global}{modpath}/FHEM/DevIo.pm";
  
  # Verbindung der Funktionen dieses Moduls mit FHEM
  $hash->{DefFn}   = "define_module";
  $hash->{UndefFn} = "undefine_module";
  $hash->{GetFn}   = "get_parameter";
  $hash->{SetFn}   = "set_parameter";
  $hash->{ReadFn}  = "read_from_device";
  $hash->{ReadyFn} = "is_ready";

  # Attribute f�r Anwender
  $hash->{AttrList}= "loglevel:0,1,2,3,4,5,6";
}

##
# �ffnen eines Ger�tes und Verkn�pfung mit diesem Modul
##
sub define_module {
  my ($hash, $arguments) = @_;
  my @a = split("[ \t]+", $arguments);

  return "wrong syntax: define <name> IMPCOUNT <devicename>"
    if (@a != 3);
  DevIo_CloseDev($hash);
  $hash->{DeviceName} = $a[2];

  return DevIo_OpenDev($hash, 0, "init_device");
}

##
# Wird beim �ffnen des Ger�tes von FHEM aufgerufen
##
sub init_device {
  my ($hash) = @_;
  Log(2, "Successfully opened $hash->{NAME} device $hash->{DeviceName}");
}

##
# Schlie�en eines verkn�pften Ger�tes
##
sub undefine_module {
  my ($hash, $arg) = @_;
  DevIo_CloseDev($hash);
  RemoveInternalTimer($hash);
  return undef;
}

##
# Liefert einen Modul-Parameter.
# Die Abfrage des Wertes 'readings' triggert die Abfrage des Ger�tes.
##
sub get_parameter {
  my ($hash, @a) = @_;

  if ($a[1] eq "readings") {
    Log(3, "Read was triggered manually");
    read_from_device($hash);
    return;
  }

  my $cmd = $module_parameters{$a[1]};
  return "Unknown argument $a[1], choose one of " .
                join(" ", sort keys %module_parameters) if(!defined($cmd));
  
  return $module_parameters{$a[1]};
}

##
# �ndert den Wert eines Modul-Parameters.
##
sub set_parameter {
  my ($hash, @a) = @_;
  my $cmd = $module_parameters{$a[1]};
  return "Unknown argument $a[1], choose one of " .
                join(" ", sort keys %module_parameters) if(!defined($cmd));
  $module_parameters{$a[1]} = $a[2];

  # Ein ge�nderter Modul-Parameter kann Auswirkungen auf das Format der Werte
  # haben; hier werden deshalb alle Parameter ausgegeben. Damit kann im Log
  # zu diesem Modul besser nachvollzogen werden, warum sich Werte �ndern.
  my $info = join(", ", map { "$_ = '$module_parameters{$_}'" }
                        keys %module_parameters);
  DoTrigger($hash->{NAME}, "set $a[1] = '$a[2]'; parameters: " . $info);
  return undef;
}

##
# Wird beim Eintreffen von Daten von FHEM aufgerufen (select for hash->{FD})
##
sub read_from_device {
	my ($hash) = @_;

    Log(5, "> read_from_device");
    fill_buffer($hash);
    read_values($hash);
    check_buffer($hash);
    Log(5, "< read_from_device");
}

##
# Liest Daten vom Ger�t in den Puffer.
##
sub fill_buffer {
  my ($hash) = @_;

  Log(5, "Buffer before read: '$buffer'");
  $buffer = $buffer . DevIo_SimpleRead($hash);
  Log(5, "Buffer after read : '$buffer'");
}

##
# Liest Datens�tze aus dem Puffer aus.
##
sub read_values {
	my ($hash) = @_;
    
    # Die Angabe von '-1' ist wichtig, weil sonst der Terminator verschwinden
    # w�rde; dann k�nnte nicht mehr gepr�ft werden, ob ein kompletter Datensatz
    # im Puffer liegt. Die '-1' sorgt daf�r, dass f�r jeden Terminator ein
    # Leerstring im Array enthalten ist.
    my @records = split(/$module_parameters{record_separator}/, $buffer, -1);
    Log(5, "Found " . scalar(@records) . " tokens");
    
    # Alle Datens�tze bis auf den letzten verarbeiten. Denn: Wenn der letzte
    # Datensatz der Leerstring ist, muss er nicht verarbeitet werden;
    # andernfalls ist er noch nicht komplett und kann erst nach dem n�chsten
    # Auslesen verarbeitet werden.
    while (@records > 1) {
      my $record = shift(@records);
      Log(5, "Processing '$record'");
      if (length($record) > 0) {
        process_record($hash, $record);
      }
    }
    
    # Den nicht verarbeiteten Rest im Puffer speichern
    $buffer = $records[0];
    Log(5, "Buffer after read_values(): '$buffer'");
}    

##
# Pr�ft einen Datensatz und speichert ihn, sofern er g�ltig ist.
##
sub process_record {
	my ($hash, $record) = @_;
    my $timestamp = TimeNow();
    my ($id, $duration) = split(/$module_parameters{value_separator}/, $record);
    Log(5, "ID $id: $duration ms");
    
    # Die Dauer muss positiv sein
    if ($duration > 0) {
    
      # Wert aus der Dauer berechnen
      my $value = value_from_duration($duration);

      # Der Wert darf den Maximalwert nicht �berschreiten
      if ($value <= $module_parameters{max_value}) {
        my $key = $module_parameters{key_prefix}.$id;
        set_reading($hash, $timestamp, $key, $value);
      } else {
        Log (2, "Ignoring too large value: '$record'");
      }
    } else {
      Log (2, "Ignoring invalid record: '$record'");
    }
}

##
# Umrechnung der Dauer in die Z�hler-spezifische Einheit.
##
sub value_from_duration {
	my ($duration) = @_;
    if ($duration > 0) {
      my $time_scale = $module_parameters{user_time_scale}
                        / $module_parameters{sender_time_scale};

      my $signals_per_unit = $module_parameters{signals_per_unit};
      if ($signals_per_unit == 0) {
        Log(2, "signals_per_unit is 0, using 1 instead");
        $signals_per_unit = 1;
      }

      my $unit_factor = $module_parameters{unit_factor};
      if ($unit_factor == 0) {
        Log(2, "unit_factor is 0, using 1 instead");
        $unit_factor = 1;
      }

      my $value = $time_scale / ($signals_per_unit * $unit_factor * $duration);
      return $value;
    } else {
      Log(2, "Duration is 0.");
    }
}

##
# Speichert einen Datensatz.
##
sub set_reading {
  my ($hash, $timestamp, $key, $value) = @_;
  my $name = $hash->{NAME};

  Log(4, "$timestamp: $key=$value");
  $hash->{READINGS}{$key}{TIME} = $timestamp;
  $hash->{READINGS}{$key}{VAL}  = $value;
  DoTrigger($name, "$key: $value");
}

##
# Pr�ft und korrigiert Puffer�berl�ufe.
##
sub check_buffer {
  my ($hash) = @_;

  # Im Fall eines �berlaufs den Puffer halbieren
  if (length($buffer) > $module_parameters{buffer_size}) {
    my $oldLength = length($buffer);
    my $newLength = int($oldLength/2);
    Log(2, "Buffer overflow, removing first $newLength characters from buffer");
    $buffer = substr($buffer, $newLength, $oldLength);
  }
  Log(4, "Buffer after check: '$buffer'");

  # Puffer in die Readings aufnehmen, sofern Daten vorliegen.
  # Damit hat der Anwender die Chance, Probleme zu bemerken.
  if (length($buffer) > 0) {
    $hash->{READINGS}{"buffer"}{TIME} = TimeNow();
    $hash->{READINGS}{"buffer"}{VAL}  = $buffer;
  } else {
    delete $hash->{READINGS}{"buffer"};
  }
}

##
# Diese Funktion wurde aus Modul "WHR962" �bernommen.
##
sub is_ready {
  my ($hash) = @_;

  return DevIo_OpenDev($hash, 1, undef)
                if($hash->{STATE} eq "disconnected");

  # Nur f�r Windows/USB relevant
  my $po = $hash->{USBDev};
  my ($BlockingFlags, $InBytes, $OutBytes, $ErrorFlags) = $po->status;
  return ($InBytes>0);
}

################################################################################

# Der R�ckgabewert dieses Moduls signalisiert erfolgreiche Initialisierung
1;
