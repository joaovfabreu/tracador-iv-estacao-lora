// --- RECEPTOR (ESTABILIZADO + MULTI-CORE) ---
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Preferences.h>
#include <ModbusMaster.h>
#include <RTClib.h>
#include <SD.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "secrets.h"
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include "web_dashboard.h"
#include "web_teste.h"

// --- PINOS HELTEC V2 ---
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DI0 26
#define LORA_BAND 915E6

#define SD_SCK 17
#define SD_MISO 13
#define SD_MOSI 23
#define SD_CS 12

struct SensorData {
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
} __attribute__((packed));

struct IVPoint {
  float v;
  float i;
  int pwm;
};

#define MAX_BUFFER_STEPS 1024

int pwmStartVal = 1000; 
int pwmEndVal = 400;   
int pwmStepSize = 4;   
int currentTotalSteps = 20; 
int pwmSweepProfile[21];

IVPoint ivBuffer[MAX_BUFFER_STEPS];

enum SystemState { IDLE, PRE_CONDITIONING, SWEEPING, SAVING, OTA_MODE };
volatile SystemState currentState = IDLE; 

ModbusMaster node;
RTC_DS3231 rtc;
SPIClass spiSD(HSPI); 
Preferences prefs;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 16, 15, 4);

SensorData lastReceivedData; 
SensorData snapshotData;     

float VOLTAGE_SCALE = 1.0f, CURRENT_SCALE = 1.0f;
float raw_v = 0, raw_i = 0; 
int testIndex = 0, currentStep = 0, packetCount = 0;

AsyncWebServer server(80);
uint32_t lastRtcSyncDay = 0;
int currentPwmFreq = 15000; 
unsigned long stateTimer = 0, lastPacketMillis = 0;
bool sdCardOK = false, rtcOK = false;
float PZEMVoltage = 0, PZEMCurrent = 0;

void preTransmission() { digitalWrite(21, HIGH); delayMicroseconds(500); }
void postTransmission() { Serial2.flush(); delay(5); digitalWrite(21, LOW); }

void taskPZEM(void *pvParameters) {
  for(;;) {
    if (currentState == IDLE) {
      if (node.readInputRegisters(0x0000, 2) == node.ku8MBSuccess) {
        raw_v = node.getResponseBuffer(0) / 100.0f;
        raw_i = node.getResponseBuffer(1) / 100.0f;
        PZEMVoltage = raw_v * VOLTAGE_SCALE;
        PZEMCurrent = raw_i * CURRENT_SCALE;
      }
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS); 
  }
}

void receiveLoRaData() {
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    if (packetSize == sizeof(SensorData)) {
      LoRa.readBytes((uint8_t*)&lastReceivedData, sizeof(lastReceivedData));
      packetCount++;
      lastPacketMillis = millis();
      Serial.printf(">>> Pacote #%d Recebido!\n", packetCount);
      
      if (currentState == IDLE && lastReceivedData.ano >= 2024 && lastRtcSyncDay != lastReceivedData.dia) {
        if (rtcOK) {
          rtc.adjust(DateTime(lastReceivedData.ano, lastReceivedData.mes, lastReceivedData.dia, 
                              lastReceivedData.hora, lastReceivedData.minuto, lastReceivedData.segundo));
        }
        lastRtcSyncDay = lastReceivedData.dia;
      }
    } else {
      Serial.printf("!!! Erro Tam: %d\n", packetSize);
      while (LoRa.available()) LoRa.read();
    }
  }
}

void saveToSD() {
  testIndex++;
  prefs.putInt("test_idx", testIndex);
  File f = SD.open("/iv_test_" + String(testIndex) + ".csv", FILE_WRITE);
  if (f) {
    f.println("PWM;Data_Hora;Tensao_V;Corrente_A;Potencia_W;Radiacao_Solar_Wm2;Velocidade_Vento_ms;Direcao_Vento_graus;Umidade_Ar_Perc;Temp_Placa_C;Temp_Ambiente_C;Inclinacao_graus;GPS_Lat;GPS_Long");
    for (int i = 0; i < currentTotalSteps; i++) {
      if (ivBuffer[i].v > 0 || ivBuffer[i].i > 0) {
        char dt[20];
        sprintf(dt, "%02d/%02d/%04d %02d:%02d", lastReceivedData.dia, lastReceivedData.mes, lastReceivedData.ano, lastReceivedData.hora, lastReceivedData.minuto);
        f.printf("%d;%s;%.3f;%.3f;%.3f;%.1f;%.1f;%.1f;%.1f;%.1f;%.1f;%.3f;%.3f\n",
               ivBuffer[i].pwm, dt, ivBuffer[i].v, ivBuffer[i].i, ivBuffer[i].v * ivBuffer[i].i,
               snapshotData.radiacaoSolar, snapshotData.velocidadeVento, snapshotData.direcaoVento,
               snapshotData.umidadeAr, snapshotData.tempPlaca, snapshotData.tempAmbiente,
               snapshotData.inclinacao, snapshotData.gpsLat, snapshotData.gpsLong);
      }
    }
    f.close();
    Serial.println(">>> SD Salvo.");
  }
}

// Função isolada para calcular o Perfil Não-Linear (Zoom Dinâmico)
void calculateSweepProfile(int startPct, int endPct) {
    currentTotalSteps = 20;
    pwmStartVal = (startPct * 1023) / 100;
    pwmEndVal = (endPct * 1023) / 100;
    
    int pwmKneeStart = (98.0 / 100.0) * 1023;
    int pwmKneeEnd = (94.1 / 100.0) * 1023;
    
    int kneePoints = 15;
    int tailPoints = currentTotalSteps - kneePoints;
    
    for(int i = 0; i < kneePoints; i++) {
      pwmSweepProfile[i] = pwmKneeStart - i * ((pwmKneeStart - pwmKneeEnd) / (kneePoints - 1));
    }
    for(int i = 0; i < tailPoints; i++) {
      pwmSweepProfile[kneePoints + i] = pwmKneeEnd - (i + 1) * ((pwmKneeEnd - pwmEndVal) / tailPoints);
    }
    pwmStartVal = pwmSweepProfile[0];
}

void handleFSM() {
  static unsigned long btnPressTime = 0;
  static bool isBtnPressed = false;
  
  bool currentBtn = (digitalRead(0) == LOW);
  
  if (currentBtn && !isBtnPressed) {
    isBtnPressed = true;
    btnPressTime = millis();
    delay(50); 
  } else if (!currentBtn && isBtnPressed) {
    isBtnPressed = false;
    unsigned long duration = millis() - btnPressTime;
    
    if (currentState == OTA_MODE) {
    } else if (duration >= 3000) {
      currentState = OTA_MODE;
      ledcWrite(22, 0); 
      Serial.println(">>> Entrando em modo OTA...");
      
      if (WiFi.getMode() != WIFI_AP) {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_AP);
        WiFi.softAP("Tracador-IV", "admin_password");
      }
      
      ArduinoOTA.setHostname(OTA_HOSTNAME);
      ArduinoOTA.setPassword(OTA_PASSWORD);
      ArduinoOTA.begin();
      
      u8g2.clearBuffer();
      u8g2.drawStr(0, 15, "MODO OTA ATIVO");
      u8g2.drawStr(0, 30, "Rede: Tracador-IV");
      u8g2.drawStr(0, 45, WiFi.softAPIP().toString().c_str());
      u8g2.sendBuffer();
    } else if (duration > 50) {
      if (currentState == IDLE) {
        // Correção Crítica: Inicializa os valores matemáticos se acionado pelo botão físico!
        calculateSweepProfile(98, 40); 

        memcpy(&snapshotData, &lastReceivedData, sizeof(SensorData));
        memset(ivBuffer, 0, sizeof(ivBuffer)); 
        currentState = PRE_CONDITIONING;
        currentStep = 0;
        stateTimer = millis();
        ledcWrite(22, pwmStartVal); 
        Serial.printf(">>> Teste I-V INICIADO via Botão (Zoom Dinamico: %d a %d)\n", pwmStartVal, pwmEndVal);
      }
    }
  }

  switch (currentState) {
    case IDLE:
      break;
      
    case PRE_CONDITIONING:
      if (millis() - stateTimer >= 1500) {
        currentState = SWEEPING;
        stateTimer = millis() - 500; 
      }
      break;
      
    case SWEEPING:
      if (millis() - stateTimer >= 500) {
        uint8_t res = node.readInputRegisters(0x0000, 2);
        ivBuffer[currentStep].v = (res == node.ku8MBSuccess) ? (node.getResponseBuffer(0) / 100.0f) * VOLTAGE_SCALE : 0;
        ivBuffer[currentStep].i = (res == node.ku8MBSuccess) ? (node.getResponseBuffer(1) / 100.0f) * CURRENT_SCALE : 0;
        ivBuffer[currentStep].pwm = pwmSweepProfile[currentStep];
        
        currentStep++;
        
        if (currentStep < currentTotalSteps) { 
          ledcWrite(22, (uint32_t)pwmSweepProfile[currentStep]);
          stateTimer = millis(); 
        } else {
          ledcWrite(22, 0); 
          currentState = SAVING; 
        }
      }
      break;
      
    case SAVING: 
      saveToSD(); 
      currentState = IDLE; 
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(21, OUTPUT); 
  digitalWrite(21, LOW); 
  delay(200); // Aumento de delay para estabilizar o Vext do OLED

  Wire.begin(4, 15);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.clearBuffer();
  u8g2.drawStr(10, 30, "Iniciando Sistema...");
  u8g2.sendBuffer();

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  if (LoRa.begin(LORA_BAND)) {
    LoRa.setSyncWord(0x12);
    Serial.println(">>> LoRa OK.");
  }

  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "Conectando WiFi...");
  u8g2.sendBuffer();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  IPAddress local_IP(192, 168, 137, 80);
  IPAddress gateway(192, 168, 137, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_IP, gateway, subnet);
  
  WiFi.begin("JOÃO-VICTOR 3128", "seilansei");
  
  int attempts = 0;
  // Limitando espera do WiFi a 5s para não parecer que a placa morreu
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  u8g2.clearBuffer();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n>>> WiFi Conectado!");
    u8g2.drawStr(0, 15, "WiFi Conectado!");
    u8g2.drawStr(0, 30, WiFi.localIP().toString().c_str());
    ArduinoOTA.setHostname("esp32-tracador");
    ArduinoOTA.setPassword("admin_password");
    ArduinoOTA.begin();
  } else {
    Serial.println("\n>>> Falha no WiFi.");
    u8g2.drawStr(0, 15, "Falha no WiFi");
  }
  u8g2.sendBuffer();
  delay(1000);

  spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  sdCardOK = SD.begin(SD_CS, spiSD);

  rtcOK = rtc.begin();
  pinMode(0, INPUT_PULLUP);
  
  ledcAttach(22, currentPwmFreq, 10); 
  ledcWrite(22, 0); 
  
  Serial2.begin(9600, SERIAL_8N1, 32, 33);
  node.begin(0x01, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  prefs.begin("pzem17", false);
  VOLTAGE_SCALE = prefs.getFloat("v_scale", 1.0f);
  CURRENT_SCALE = prefs.getFloat("i_scale", 1.0f);
  testIndex = prefs.getInt("test_idx", 0);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", WEB_TESTE_HTML);
  });
  
  server.on("/ajustes", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", WEB_DASHBOARD_HTML);
  });

  server.on("/live", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"v\":" + String(PZEMVoltage) + ",\"i\":" + String(PZEMCurrent) + 
                  ",\"raw_v\":" + String(raw_v) + ",\"raw_i\":" + String(raw_i) + "}";
    request->send(200, "application/json", json);
  });

  server.on("/calibrate", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("real_v", true)) {
      float real = request->getParam("real_v", true)->value().toFloat();
      if (raw_v > 0) {
        VOLTAGE_SCALE = real / raw_v;
        prefs.putFloat("v_scale", VOLTAGE_SCALE);
      }
    }
    if (request->hasParam("real_i", true)) {
      float real = request->getParam("real_i", true)->value().toFloat();
      if (raw_i > 0) {
        CURRENT_SCALE = real / raw_i;
        prefs.putFloat("i_scale", CURRENT_SCALE);
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/set_pwm", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("val", true) && currentState == IDLE) {
      int val_pct = request->getParam("val", true)->value().toInt();
      val_pct = max(0, min(100, val_pct));
      int pwm_val = (val_pct / 100.0) * 1023; // Ajustado para 10-bits (1023) em vez de 255
      ledcWrite(22, pwm_val);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Teste em curso ou erro");
    }
  });

  server.on("/set_freq", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("val")) {
      currentPwmFreq = request->getParam("val")->value().toInt();
      ledcAttach(22, currentPwmFreq, 10); 
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing val");
    }
  });

  server.on("/start_test", HTTP_POST, [](AsyncWebServerRequest *request){
    if (currentState == IDLE) {
      int startPct = request->hasParam("start") ? request->getParam("start")->value().toInt() : 98;
      int endPct = request->hasParam("end") ? request->getParam("end")->value().toInt() : 40;
      
      calculateSweepProfile(startPct, endPct);

      memcpy(&snapshotData, &lastReceivedData, sizeof(SensorData));
      memset(ivBuffer, 0, sizeof(ivBuffer)); 
      currentState = PRE_CONDITIONING;
      currentStep = 0;
      stateTimer = millis();
      ledcWrite(22, pwmStartVal);
      Serial.printf(">>> Teste I-V INICIADO via Web Dashboard (Zoom Dinamico: %d a %d)\n", pwmStartVal, pwmEndVal);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Ocupado");
    }
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String stateStr = "IDLE";
    if (currentState == PRE_CONDITIONING || currentState == SWEEPING) stateStr = "SWEEPING";
    else if (currentState == SAVING) stateStr = "SAVING";
    else if (currentState == OTA_MODE) stateStr = "OTA";
    
    String json = "{\"state\":\"" + stateStr + "\",\"step\":" + String(currentStep) + "}";
    request->send(200, "application/json", json);
  });

  server.on("/last_test", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "[";
    for(int i=0; i<currentTotalSteps; i++) {
      json += "{\"v\":" + String(ivBuffer[i].v, 2) + ",\"i\":" + String(ivBuffer[i].i, 2) + "}";
      if(i < currentTotalSteps - 1) json += ",";
    }
    json += "]";
    request->send(200, "application/json", json);
  });

  server.on("/download_csv", HTTP_GET, [](AsyncWebServerRequest *request){
    if (sdCardOK) {
      String fileName = "/iv_test_" + String(testIndex) + ".csv";
      if (SD.exists(fileName)) {
        request->send(SD, fileName, "text/csv", true);
      } else {
        request->send(404, "text/plain", "Nenhum teste encontrado no SD ainda.");
      }
    } else {
      request->send(500, "text/plain", "Erro no Cartao SD");
    }
  });

  server.begin();
  
  // Só inicia a task Modbus DEPOIS de todo o Setup e WiFi (evita o pin 21 desligar o OLED no boot)
  xTaskCreatePinnedToCore(taskPZEM, "TaskPZEM", 4096, NULL, 1, NULL, 0);
  Serial.println(">>> Setup concluído!");
}

void loop() {
  ArduinoOTA.handle(); 

  if (currentState == OTA_MODE) {
    return; 
  }

  receiveLoRaData(); 
  handleFSM();       
  
  static unsigned long lastOled = 0;
  if (millis() - lastOled > 500) {
    lastOled = millis();
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, currentState == IDLE ? "RECEPTOR: IDLE" : "TESTE EM CURSO");
    
    char buf[40];
    u8g2.drawStr(0, 25, "IP do Tracador:");
    u8g2.drawStr(0, 40, WiFi.localIP().toString().c_str());
    
    int currentPwm = ledcRead(22);
    int pct = (currentPwm * 100) / 1023; // Ajustado para 10-bits
    sprintf(buf, "Carga IGBT: %d%%", pct);
    u8g2.drawStr(0, 55, buf);
    
    u8g2.sendBuffer();
  }
}
