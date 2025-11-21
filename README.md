# Estação de Bem-Estar Inteligente (ESP32 + MQTT + Wokwi)

## Problema
Fadiga, má postura e ambiente inadequado reduzem produtividade e saúde em ambientes de trabalho.

## Solução
Uma estação com ESP32 que monitora temperatura/umidade (DHT22), postura (ultrassônico), sugere pausas, publica dados via MQTT e permite comandos remotos (Node-RED).

## Componentes
- ESP32 (DevKit v1)
- DHT22
- HC-SR04
- 2 LEDs (vermelho/verde)
- Buzzer
- Botão
- Broker MQTT (test.mosquitto.org) e Node-RED para dashboard

## Como rodar
1. Abra o projeto no **Wokwi** e cole `diagram.json` e `main.ino`.
2. Clique em Play (rede habilitada no Wokwi).
3. No Node-RED, importe `node-red/flow.json` e Deploy.
4. Veja gauges e alertas; use o botão “Forçar Pausa”.

## Tópicos MQTT
- `fiap/deivid/edge/status` – status do dispositivo
- `fiap/deivid/edge/sensors` – JSON com temp/umidade/distância
- `fiap/deivid/edge/alerts` – alertas de postura/pausa/ambiente
- `fiap/deivid/edge/commands` – comandos (ex.: `pause_now`)

## Links
- Simulação Wokwi: (adicione o link público do seu projeto)
- Vídeo explicativo: (adicione o link, até 3 min)

## Explicação técnica (MQTT/HTTP)
Este projeto usa MQTT (broker Mosquitto público):
- ESP32 publica dados/alertas e assina comandos.
- Node-RED assina dados/alertas e envia comandos.

## Integrantes
- Deivid ruan – RM566356
- Felipe cordeiro - RM566518