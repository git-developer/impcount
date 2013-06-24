/**
 * Impulse counter
 * ===============
 * 
 * Autor:   fhem-user ( http://forum.fhem.de/index.php?t=email&toi=1713 )
 * Version: 1.0
 * Date:    24.06.2013
 *
 * Description
 * -----------
 * Connect S0 counters to Arduino's digital pins and this sketch will print
 * the duration between two signals to the serial port. Interrupts are used
 * when the pin supports it, so the loop() function may do other things.
 * 
 * Parameters
 * ----------
 * S0_PINS:   Carries the pins to monitor
 * BAUD_RATE: Communication speed of the serial port
 *
 * 
 * References
 * ----------
 * See http://forum.fhem.de/index.php?t=rview&goto=83400 for details.
 *
 * Copyright notice
 * ----------------
 *  (c) 2013
 *  Copyright: fhem-user ( http://forum.fhem.de/index.php?t=email&toi=1713 )
 *  All rights reserved
 *
 *  This script free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  The GNU General Public License can be found at
 *  http://www.gnu.org/copyleft/gpl.html.
 *
 *  This script is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  This copyright notice MUST APPEAR in all copies of the script!
 *
 */

/* Start of user-defined variables ****************************************** */

/** Contains a pin number for each pin which connected to a S0 interface */
const byte          S0_PINS[] = { 10, 2, 3, 17, 18 };

/** Communication speed for the serial port */
const unsigned long BAUD_RATE = 9600;

/* End of user-defined variables ******************************************** */


/** Number of pins that are connected to a S0 interface  */
const int     PIN_COUNT = sizeof(S0_PINS);

/** For each S0 pin: timestamp of the previous S0 signal */
unsigned long previousTimestamps[PIN_COUNT];

/** For each S0 pin: value of the previous S0 signal */
byte          previousValues[PIN_COUNT];

/**
 * For each interrupt: index of the corresponding pin in S0_PINS
 *
 * Interrupt/pin mapping for Mega2560:
 *  INT0=2, INT1=3, INT2=21, INT3=20, INT4=19, INT5=18
 *
 * Example:
 *  interruptIndexes[2] corresponds to INT2 / pin 21
 *  If S0_PINS == { 18, 19, 20, 21, 22 }, then interruptIndexes[2] == 3
 *  because S0_PINS[3] == 21
 */
int           interruptIndexes[] = { 0, 0, 0, 0, 0, 0 };

boolean initialized = false;

/**
 * Initialization of pins, variables and serial communication. 
 */
void setup() {
  const int now = millis();
  for (int i = 0; i < PIN_COUNT; i++) {
    pinMode(S0_PINS[i], INPUT_PULLUP);
    previousTimestamps[i] = now;
    previousValues[i]     = LOW;
    
    /* 
     * if the current pin support interrupts:
     *  save the position of this pin within S0_PINS,
     *  and declare an ISR to report S0 signals
     */
    switch(S0_PINS[i]) {
      case  2: interruptIndexes[0] = i;
               attachInterrupt(0, onInterrupt0, FALLING);
               break;
      case  3: interruptIndexes[1] = i;
               attachInterrupt(1, onInterrupt1, FALLING);
               break;
      case 21: interruptIndexes[2] = i;
               attachInterrupt(2, onInterrupt2, FALLING);
               break;
      case 20: interruptIndexes[3] = i;
               attachInterrupt(3, onInterrupt3, FALLING);
               break;
      case 19: interruptIndexes[4] = i;
               attachInterrupt(4, onInterrupt4, FALLING);
               break;
      case 18: interruptIndexes[5] = i;
               attachInterrupt(5, onInterrupt5, FALLING);
               break;
      default:;
    }
  }
  
  Serial.begin(BAUD_RATE);
  initialized = true;
}

/* Interrupt service routines */
void onInterrupt0() { signal(interruptIndexes[0]); }
void onInterrupt1() { signal(interruptIndexes[1]); }
void onInterrupt2() { signal(interruptIndexes[2]); }
void onInterrupt3() { signal(interruptIndexes[3]); }
void onInterrupt4() { signal(interruptIndexes[4]); }
void onInterrupt5() { signal(interruptIndexes[5]); }

/**
 * Polls all pins from S0_PINS that don't support interrupts.
 */
void loop() {
  for (int i = 0; i < PIN_COUNT; i++) {
    if (needsActiveRead(i)) {
       read(i);
    }
  }
}

/**
 * Checks if a pin must be polled (because it doesn't support interrupts).
 *
 * @param i Index of a pin within S0_PINS
 * @return true if the pin must be polled, false if it supports interrupts
 */
boolean needsActiveRead(int i) {
    switch(S0_PINS[i]) {
      case  2: // falls through
      case  3: // falls through
      case 21: // falls through
      case 20: // falls through
      case 19: // falls through
      case 18: return false;
      default: return true;
    }
}

/**
 * Checks a pin for a S0 signal
 */
void read(int i) {
    byte value = digitalRead(S0_PINS[i]);
    
    /* Signal only a falling edge */
    if (value == LOW && previousValues[i] == HIGH) {
      signal(i);
    }
    previousValues[i] = value;
}

/**
 * Reports a S0 signal for a pin.
 * 
 * "<pin>:<duration>" is written to the serial port,
 *  where <pin> is the pin number,
 *  and <duration> is the duration in milliseconds since the last known value
 *
 * @param i Index of a pin within S0_PINS
 */
void signal(int i) {
  const unsigned long now = millis();
  
  /** Calculation of duration works even on overflow of 'now'  */
  const unsigned long duration = now - previousTimestamps[i];
  
  /* 
   * An ISR seems to be called once when it is associated,
   * no need to print anything in this case. 
   */
  if (initialized) {
    previousTimestamps[i] = now;
    Serial.print(S0_PINS[i]);
    Serial.print(":");
    Serial.println(duration);
  }
}
