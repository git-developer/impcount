const byte          S0_PINS[] = { 2, 3, 17, 18 };

const unsigned long BAUD_RATE = 9600;


const int     PIN_COUNT = sizeof(S0_PINS);

void setup() {
  Serial.begin(BAUD_RATE);
}

/**
 * Print a random S0 signal pin every few seconds
 */
void loop() {
  long time = random(2000, 6000);
  Serial.print(S0_PINS[random(PIN_COUNT)]);
  Serial.print(":");
  Serial.println(time);
  delay(time);
}

