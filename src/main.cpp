#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}