# Tópicos MQTT
#include <DHT.h>

#define PIN_DHT      15
#define PIN_DHTTYPE  DHT22
#define PIN_TRIG     5
#define PIN_ECHO     18
#define PIN_LED_RED   4
#define PIN_LED_GREEN 2
#define PIN_BUZZER    13
#define PIN_BUTTON    14

DHT dht(PIN_DHT, PIN_DHTTYPE);

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  dht.begin();
}

float readDistanceCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duration = pulseIn(PIN_ECHO, HIGH);
  return duration * 0.034 / 2;
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  float dist = readDistanceCM();

  Serial.printf("Temp: %.1f°C  Humid: %.1f%%  Dist: %.1fcm\n", t, h, dist);

  if (t > 28 || h < 30) {
    digitalWrite(PIN_LED_GREEN, HIGH);
    tone(PIN_BUZZER, 1000);
  } else {
    digitalWrite(PIN_LED_GREEN, LOW);
    noTone(PIN_BUZZER);
  }

  if (dist < 20) {
    digitalWrite(PIN_LED_RED, HIGH);
  } else {
    digitalWrite(PIN_LED_RED, LOW);
  }

  if (digitalRead(PIN_BUTTON) == LOW) {
    Serial.println("Pausa confirmada!");
    delay(300);
  }

  delay(2000);
}

