/**
 * Impulse counter
 * ===============
 * 
 * Autor:   fhem-user ( http://forum.fhem.de/index.php?t=email&toi=1713 )
 * Version: 2.0
 * Date:    06.09.2013
 * 
 * Changes since 1.0
 * -----------------
 *  - Reporting of impulse duration
 *  - Support for minimum and maximum impulse durations
 *     to filter invalid impulses
 *  - Separation of impulse capturing and reporting
 *     Reporting is done from the main loop only,
 *     interrupt service routines return faster.
 *     This should minimize impulse loss on interrupt pins
 *  - Optional: output of statistics about actual polling interval
 *
 * Description
 * -----------
 * Connect S0 counters to Arduino's digital pins and this sketch will print
 * the duration between two impulses to the serial port. Interrupts are used
 * when the pin supports it, so the loop() function may do other things.
 * 
 * Parameters
 * ----------
 * S0_PINS:               Carries the pins to monitor
 * MIN_IMPULSE_DURATIONS: Lower bound for impulse duration
 * MAX_IMPULSE_DURATIONS: Upper bound for impulse duration
 * BAUD_RATE:             Communication speed of the serial port
 * STATISTICS:            Flag to activate output of statistics
 *                         about actual polling interval
 * LOOPS_PER_STATISTIC:   Controls frequency of statistics output
 *                         (higher value means lower frequency)
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

#define ULONG_MAX 4294967295

/* Start of user-defined variables ****************************************** */

/** Contains a pin number for each pin which connected to a S0 interface */
const byte          S0_PINS[]               = { 20,  21, 22,  23 };

/** For each S0 pin: lower bound for valid impulse durations (unit: ms) */
const unsigned long MIN_IMPULSE_DURATIONS[] = { 60,   0, 60,   0 };

/** For each S0 pin: upper bound for valid impulse durations (unit: ms) */
const unsigned long MAX_IMPULSE_DURATIONS[] = { 90, 500, 90, ULONG_MAX };

/** Communication speed for the serial port */
const unsigned long BAUD_RATE = 9600;

/* Uncomment to activate polling statistics */
//#define STATISTICS

/** Controls statistics output, higher value means less statistics */
const unsigned long LOOPS_PER_STATISTIC = 30000;

/* End of user-defined variables ******************************************** */


/** Number of pins that are connected to a S0 interface  */
const int     PIN_COUNT = sizeof(S0_PINS);

/** For each S0 pin: timestamp of the latest falling edge */
volatile unsigned long falling[PIN_COUNT];

/** For each S0 pin: timestamp of the latest rising edge */
volatile unsigned long rising[PIN_COUNT];

/** For each S0 pin: timestamp of the latest valid impulse */
         unsigned long impulse[PIN_COUNT];

/**
 * For each interrupt: index of the corresponding pin in S0_PINS
 *
 * Interrupt/pin mapping for Mega2560:
 *  INT0=2, INT1=3, INT2=21, INT3=20, INT4=19, INT5=18
 *
 * Example:
 *  pinForInterrupt[2] corresponds to INT2 / pin 21
 *  If S0_PINS == { 18, 19, 20, 21, 22 }, then pinForInterrupt[2] == 3
 *  because S0_PINS[3] == 21
 */
int           pinForInterrupt[] = { 0, 0, 0, 0, 0, 0 };

#ifdef STATISTICS
unsigned long statisticsTimestamp = micros();
unsigned long loopCount = 0;
unsigned long maxLoopDuration = 0;

void resetStatistics() {
  loopCount = 0;
  maxLoopDuration = 0;
}
#endif

/**
 * Initialization of pins, variables and serial communication. 
 */
void setup() {
  const unsigned long now = millis();
  for (int i = 0; i < PIN_COUNT; i++) {
    pinMode(S0_PINS[i], INPUT_PULLUP);
    falling[i]  = now;
    rising[i]   = now;
    impulse[i]  = now;
    
    /* 
     * if the current pin support interrupts:
     *  save the position of this pin within S0_PINS,
     *  and declare an ISR to report S0 impulses
     */
    switch(S0_PINS[i]) {
      case  2: pinForInterrupt[0] = i;
               attachInterrupt(0, onInterrupt0, CHANGE);
               break;
      case  3: pinForInterrupt[1] = i;
               attachInterrupt(1, onInterrupt1, CHANGE);
               break;
      case 21: pinForInterrupt[2] = i;
               attachInterrupt(2, onInterrupt2, CHANGE);
               break;
      case 20: pinForInterrupt[3] = i;
               attachInterrupt(3, onInterrupt3, CHANGE);
               break;
      case 19: pinForInterrupt[4] = i;
               attachInterrupt(4, onInterrupt4, CHANGE);
               break;
      case 18: pinForInterrupt[5] = i;
               attachInterrupt(5, onInterrupt5, CHANGE);
               break;
      default:;
    }
  }
  
  Serial.begin(BAUD_RATE);
}

/**
 * 1.) Polls all pins from S0_PINS that don't support interrupts.
 * 2.) Reports all arrived impulses.
 */
void loop() {
  for (int i = 0; i < PIN_COUNT; i++) {
    if (needsPolling(i)) {
       poll(i);
    }
    report(i);
  }
  
  #ifdef STATISTICS
    const unsigned long now = micros();
    if (loopCount++ > 0) {
      const unsigned long loopDuration = now - statisticsTimestamp;
      if (loopDuration > maxLoopDuration) {
        maxLoopDuration = loopDuration;
      }
      if (loopCount >= LOOPS_PER_STATISTIC) {
        Serial.print("poll-interval: ");
        Serial.print(maxLoopDuration);
        Serial.println("us");
        resetStatistics();
      }
    }
    statisticsTimestamp = now;
  #endif
}


/* Impulse checking ********************************************************** */

/* Interrupt service routines */
void onInterrupt0() { update(pinForInterrupt[0], read(pinForInterrupt[0])); }
void onInterrupt1() { update(pinForInterrupt[1], read(pinForInterrupt[1])); }
void onInterrupt2() { update(pinForInterrupt[2], read(pinForInterrupt[2])); }
void onInterrupt3() { update(pinForInterrupt[3], read(pinForInterrupt[3])); }
void onInterrupt4() { update(pinForInterrupt[4], read(pinForInterrupt[4])); }
void onInterrupt5() { update(pinForInterrupt[5], read(pinForInterrupt[5])); }

/**
 * Checks if a pin must be polled (because it doesn't support interrupts).
 *
 * @param i Index of a pin within S0_PINS
 * @return true if the pin must be polled, false if it supports interrupts
 */
boolean needsPolling(int i) {
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
 * Checks a pin for a S0 impulse and stores it, if so.
 *
 * @param i Index of a pin within S0_PINS
 */
void poll(int i) {
    byte newValue = read(i);
    if (newValue != valueOf(i)) {
      update(i, newValue);
    }
}

/**
 * @param i Index of a pin within S0_PINS
 * @return Current value of the pin: one of (LOW, HIGH)
 */
byte read(int i) {
    return digitalRead(S0_PINS[i]);
}

/**
 * @param i Index of a pin within S0_PINS
 * @return Latest known value of the pin: one of (LOW, HIGH)
 */
byte valueOf(int i) {
    return isAfter(falling[i], rising[i]) ? LOW : HIGH;
}

/**
 * Updates the value of a pin.
 *
 * May be called from an interrupt service routine
 * (returns fast, used fields are volatile).
 *
 * @param i Index of a pin within S0_PINS
 * @param value New value of the pin
 */
void update(int i, byte value) {
    const unsigned long now = millis();
    if (value == LOW) {
      
      /*
       * When storing a new falling edge, reset the latest rising edge.
       * This is necessary for the case that
       * 1.) the previous impulse (#1) was not reported yet and
       * 2.) a falling edge of impulse (#2) arrives and
       * 3.) the report starts before the rising edge of impulse #2.
       *
       * Without reset, falling and rising edge would not match.
       */
      rising[i] = impulse[i];
      falling[i] = now;
    } else {
      rising[i] = now;
    }
}

/**
 * Reports an impulse.
 *
 * Must NOT be called from an interrupt service routine.
 * 
 * @param i Index of a pin within S0_PINS
 */
void report(int i) {

  /*
   * Work with a copy of the timestamps,
   * because their values can be updated anytime by an ISR.
   *
   * Forbid interrupts between the reads.
   * This guarantees that either falling and rising edge match each other,
   * or falling[i] is new and rising[i] == impulse[i] (because of reset in update()).
   */
  noInterrupts();
  const unsigned long latestFalling = falling[i];
  const unsigned long latestRising  = rising[i];
  interrupts();
  
  /* Check if an impulse arrived */
  if (isAfter(latestRising, impulse[i])) {
  
    /** Calculate durations (works on overflows, too) */
    const unsigned long impulseDistance = latestRising - impulse[i];
    const unsigned long impulseDuration = latestRising - latestFalling;
    
    /** Validate */
    if (!isValid(i, impulseDuration, latestRising, latestFalling)) {
       Serial.print("INVALID ");
    }

    /* Send report */     
    Serial.print(S0_PINS[i]);
    Serial.print(":");
    Serial.print(impulseDistance);
    Serial.print(",");
    Serial.println(impulseDuration);
  }
}

/**
 * Compares two timestamps; considers variable rollover of t2.
 *
 * @param t1 A timestamp
 * @param t2 Another timestamp
 * @return true iff t1 is probably after than t2
 */
boolean isAfter(unsigned long t1, unsigned long t2) {
  return t1 - t2 < t2 - t1;
}

/** 
 * Checks if an impulse is valid, and removes timestamps of an invalid impulse.
 *
 * @param i               Index of a pin within S0_PINS
 * @param impulseDuration Duration of the current impulse
 * @param latestRising    Rising edge of the previous valid impulse
 * @param latestFalling   Falling edge of the previous valid impulse
 * @return true iff the impulse is valid
 */
boolean isValid(int i,
                unsigned long impulseDuration,
                unsigned long latestRising,
                unsigned long latestFalling) { 
  boolean valid = false;
                        
  if (impulseDuration < MIN_IMPULSE_DURATIONS[i]
   || impulseDuration > MAX_IMPULSE_DURATIONS[i]) {
     
     /* 
      * Overwrite the timestamps of the invalid impulse with the
      * timestamp of the previous rising edge, but only if no ISR
      * has overwritten it in the meantime.
      */
     noInterrupts();
     if (rising[i] == latestRising) {
       rising[i] = impulse[i];
       if (falling[i] == latestFalling) {
         falling[i] = impulse[i];
       }
     }
     interrupts();
  } else {
  
    /* Update timestamp for valid impulses */
    impulse[i] = latestRising;
    valid = true;
  }
  return valid;
}

