# Tópicos MQTT

- fiap/deivid/edge/status
  Publica status do dispositivo (online/offline).

- fiap/deivid/edge/sensors
  JSON com temp, umidade e distância:
  {"temp": 26.5, "humid": 45.2, "distance_cm": 18.0}

- fiap/deivid/edge/alerts
  Alertas de postura, ambiente e pausas:
  {"type":"posture","msg":"Postura ruim detectada."}

- fiap/deivid/edge/commands
  Comandos recebidos do backend:
  Exemplos: "pause_now", "led_green", "led_green_off", "led_red", "led_red_off"
