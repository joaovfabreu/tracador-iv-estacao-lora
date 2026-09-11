#include <Arduino.h>
#include "heltec.h"

struct __attribute__((packed)) SensorData {
  float radiacaoSolar;
  float velocidadeVento;
  float direcaoVento;
  float umidadeAr;
  float tempPlaca;
  float tempAmbiente;
  float inclinacao;
  double gpsLat;
  double gpsLong;
  uint16_t ano;
  uint8_t mes;
  uint8_t dia;
  uint8_t hora;
  uint8_t minuto;
  uint8_t segundo;
};

void setup() {
  Heltec.begin(true, true, true, true, 470E6);
  LoRa.setSyncWord(0x12);
  Serial.begin(9600);
}

void loop() {
  static unsigned long lastTx = 0;
  if (millis() - lastTx > 3000) {
    SensorData data;
    // INICIALIZA TODOS OS CAMPOS PARA NÃO ENVIAR LIXO
    data.radiacaoSolar = 800.0 + (random(-10, 11) / 1.0);
    data.velocidadeVento = 5.0 + (random(0, 100) / 100.0);
    data.direcaoVento = 180.0;
    data.umidadeAr = 60.0;
    data.tempPlaca = 45.0;
    data.tempAmbiente = 25.0;
    data.inclinacao = 23.5;
    data.gpsLat = -23.550;
    data.gpsLong = -46.633;
    data.ano = 2026;
    data.mes = 5;
    data.dia = 14;
    data.hora = 14;
    data.minuto = random(0, 60);
    data.segundo = random(0, 60);

    LoRa.beginPacket();
    LoRa.write((uint8_t*)&data, sizeof(SensorData));
    LoRa.endPacket();
    
    Serial.printf(">>> ENVIANDO LORA: Rad=%.1f | Tam=%d bytes\n", data.radiacaoSolar, sizeof(SensorData));
    
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, "EMISSOR OK");
    Heltec.display->drawString(0, 20, "Enviando Rad: " + String(data.radiacaoSolar, 1));
    Heltec.display->display();
    
    lastTx = millis();
  }
}
