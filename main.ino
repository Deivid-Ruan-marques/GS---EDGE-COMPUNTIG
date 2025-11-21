#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ====== CONFIG WIFI/MQTT ======
const char* WIFI_SSID     = "SEU_SSID";
const char* WIFI_PASSWORD = "SUA_SENHA";

// Broker público de testes (use outro se preferir)
const char* MQTT_SERVER   = "test.mosquitto.org";
const int   MQTT_PORT     = 1883;
const char* MQTT_CLIENTID = "esp32-deivid-fiap";

// Tópicos MQTT
const char* TOPIC_STATUS     = "fiap/deivid/edge/status";
const char* TOPIC_SENSOR     = "fiap/deivid/edge/sensors";
const char* TOPIC_ALERT      = "fiap/deivid/edge/alerts";
const char* TOPIC_COMMAND    = "fiap/deivid/edge/commands"; // comandos vindos do Node-RED

WiFiClient espClient;
PubSubClient client(espClient);

// ====== PINOS ======
#define PIN_DHT      15
#define PIN_DHTTYPE  DHT22

#define PIN_TRIG     5
#define PIN_ECHO     18

#define PIN_LED_RED   4
#define PIN_LED_GREEN 2
#define PIN_BUZZER    13
#define PIN_BUTTON    14

DHT dht(PIN_DHT, PIN_DHTTYPE);

// ====== LÓGICA ======
unsigned long lastSensorMs = 0;
unsigned long lastBreakMs  = 0;
const unsigned long SENSOR_INTERVAL_MS = 3000;     // publica a cada 3s
const unsigned long BREAK_INTERVAL_MS  = 40UL * 60UL * 1000UL; // 40 minutos

// Thresholds
const float MAX_TEMP_C  = 28.0;
const float MIN_HUMID   = 30.0;
const float BAD_POSTURE_DISTANCE_CM = 20.0; // menos que 20cm do sensor = postura ruim

// Estado
bool postureBad = false;
bool envBad     = false;

// ====== WIFI ======
void connectWifi() {
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
}

// ====== MQTT ======
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("Comando recebido (%s): %s\n", topic, msg.c_str());

  // Comandos: "pause_now", "led_green", "led_red_off", etc.
  if (msg == "pause_now") {
    triggerBreakAlert();
  } else if (msg == "led_green") {
    digitalWrite(PIN_LED_GREEN, HIGH);
  } else if (msg == "led_green_off") {
    digitalWrite(PIN_LED_GREEN, LOW);
  } else if (msg == "led_red") {
    digitalWrite(PIN_LED_RED, HIGH);
  } else if (msg == "led_red_off") {
    digitalWrite(PIN_LED_RED, LOW);
  }
}

void connectMqtt() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect(MQTT_CLIENTID)) {
      Serial.println("conectado!");
      client.subscribe(TOPIC_COMMAND);
      publishStatus("online");
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 2s");
      delay(2000);
    }
  }
}

void publishStatus(const String& status) {
  String payload = String("{\"status\":\"") + status + "\"}";
  client.publish(TOPIC_STATUS, payload.c_str(), true);
}

void publishSensors(float t, float h, float dist) {
  String payload = String("{\"temp\":") + String(t,1) +
                   ",\"humid\":" + String(h,1) +
                   ",\"distance_cm\":" + String(dist,1) + "}";
  client.publish(TOPIC_SENSOR, payload.c_str());
}

void publishAlert(const String& type, const String& msg) {
  String payload = String("{\"type\":\"") + type + "\",\"msg\":\"" + msg + "\"}";
  client.publish(TOPIC_ALERT, payload.c_str());
}

// ====== ULTRASSÔNICO ======
float readDistanceCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duration = pulseIn(PIN_ECHO, HIGH, 30000); // timeout 30ms
  float distance = duration * 0.034 / 2.0;
  return distance;
}

// ====== ALERTAS ======
void triggerBreakAlert() {
  publishAlert("break", "Hora da pausa!");
  // Buzzer + LED Green piscando
  for (int i = 0; i < 3; i++) {
    tone(PIN_BUZZER, 1000);
    digitalWrite(PIN_LED_GREEN, HIGH);
    delay(600);
    noTone(PIN_BUZZER);
    digitalWrite(PIN_LED_GREEN, LOW);
    delay(400);
  }
}

void updatePostureAlert(bool bad) {
  if (bad) {
    digitalWrite(PIN_LED_RED, HIGH);
    publishAlert("posture", "Postura ruim detectada.");
  } else {
    digitalWrite(PIN_LED_RED, LOW);
  }
}

void updateEnvAlert(bool bad, float t, float h) {
  if (bad) {
    publishAlert("environment", "Condições ruins: temp/hum fora do ideal.");
  }
}

// ====== SETUP/LOOP ======
void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  dht.begin();
  connectWifi();
  connectMqtt();

  lastSensorMs = millis();
  lastBreakMs  = millis();
}

void loop() {
  if (!client.connected()) connectMqtt();
  client.loop();

  unsigned long now = millis();

  // Leitura periódica
  if (now - lastSensorMs >= SENSOR_INTERVAL_MS) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    float dist = readDistanceCM();

    // Valida leituras
    if (isnan(t) || isnan(h)) {
      Serial.println("Falha no DHT22");
    } else {
      envBad = (t > MAX_TEMP_C) || (h < MIN_HUMID);
      updateEnvAlert(envBad, t, h);
    }
    postureBad = (dist > 0 && dist < BAD_POSTURE_DISTANCE_CM);
    updatePostureAlert(postureBad);

    publishSensors(t, h, dist);
    Serial.printf("T:%.1fC H:%.1f%% Dist:%.1fcm | envBad:%d postureBad:%d\n", t, h, dist, envBad, postureBad);

    lastSensorMs = now;
  }

  // Lembrete de pausa
  if (now - lastBreakMs >= BREAK_INTERVAL_MS) {
    triggerBreakAlert();
    lastBreakMs = now;
  }

  // Confirmação de pausa pelo botão (LOW = pressionado)
  if (digitalRead(PIN_BUTTON) == LOW) {
    publishAlert("break_ack", "Pausa confirmada pelo usuário.");
    delay(300);
  }
}
