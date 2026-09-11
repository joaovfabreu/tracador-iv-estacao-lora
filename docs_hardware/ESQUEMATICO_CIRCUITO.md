# Esquema de Ligações — Placa Rogério IGBT (v2 2026-03-06)

Baseado no arquivo PCB `PCB_placa_Rorgerio_IGBT_2026-03-06_v2`.

---

## Componentes

| Ref | Componente | Descrição |
|-----|-----------|-----------|
| **U1** | Heltec WiFi LoRa 32 (V2) | Microcontrolador principal |
| **U2** | IRGP50B60PD1 | IGBT de potência |
| **U3** | MicroSD Card Adapter | Cartão SD (SPI) |
| **U4** | MAX485 | Transceiver RS-485 |
| **R1** | Resistor 5W 1 kΩ | Resistor de pull do gate do IGBT |
| **R2** | Resistor 5W 1 kΩ | Resistor auxiliar |
| **R3** | Resistor 5W 1 kΩ | Resistor série PWM → gate do IGBT |
| **R4** | Resistor 10W 8 Ω | Carga cerâmica (1ª metade) |
| **R5** | Resistor 10W 8 Ω | Carga cerâmica (2ª metade) |
| **C1, C2** | Capacitores | Desacoplamento de alimentação |
| **IO1–IO5** | Conectores | Entradas/Saídas externas |

---

## Netlist (conexões por net)

| Net | Componentes conectados |
|-----|----------------------|
| **5V** | U1 pin 2 · R1 pin 2 · R2 pin 2 · U3 pin 5 · conectores IO |
| **GND** | U1 pin 1 · U3 pin 6 · U4 GND · todos os retornos |
| **12V** | Alimentação da seção de potência |
| **FONTE_VARIAVEL** | R4 pin 2 — entrada de tensão variável (fonte de potência) |
| **PWM** | U1 pin 9 → R3 pin 1 — saída PWM do Heltec |
| **R3_2** | R3 pin 2 → gate do IGBT U2 |
| **U2_1 (Gate)** | Gate IGBT U2 · R1 pin 1 (pull) |
| **U2_2 (Emissor)** | Emissor IGBT U2 → R5 pin 1 |
| **R4_1** | R4 pin 1 · R5 pin 2 — junção carga (R4 + R5 = 16 Ω total) |
| **MOSI** | U1 pin 11 → U3 pin 3 |
| **MISO** | U1 pin 20 ← U3 pin 4 |
| **SCK** | U1 pin 17 → U3 pin 2 |
| **CS** | U1 pin 21 → U3 pin 1 |
| **SCL** | U1 pin 14 — barramento I²C |
| **SDA** | U1 pin 16 — barramento I²C |
| **DE** | U1 pin 19 → MAX485 pinos 2 e 3 (controle direção TX/RX) |
| **DI** | U1 pin 26 → MAX485 pin 4 (TX RS-485) |
| **RO** | U1 pin 27 ← MAX485 pin 1 (RX RS-485) |
| **A** | MAX485 pin 7 — barramento RS-485 (+) externo (PZEM-017) |
| **B** | MAX485 pin 6 — barramento RS-485 (−) externo (PZEM-017) |

---

## Diagrama de blocos

```
                        ┌─────────────────────────────┐
  FONTE_VARIAVEL ────►  │  R4 (8Ω) + R5 (8Ω) = 16Ω  │ ◄── Coletor IGBT U2
                        └─────────────────────────────┘
                                      │ Emissor U2
                                     GND

                ┌──────────────────────────────┐
                │     Heltec WiFi LoRa 32 V2   │
                │  (U1)                        │
   RS-485 ──► A/B ──► MAX485 ──► RO ──► pin27 │
                │            ◄── DI ◄── pin26  │
                │  DE ◄── pin19               │
                │                             │
                │  PWM (pin9) ──► R3 (1kΩ) ──► Gate IGBT U2
                │                             │
                │  SPI ──► SD Card (U3)       │
                │  (SCK/MOSI/MISO/CS)         │
                │                             │
                │  I²C (SCL/SDA)              │
                └──────────────────────────────┘

Gate IGBT U2: controlado por PWM via R3 (1 kΩ série)
            + R1 (1 kΩ pull para GND no gate)
```

---

## Pinos do Heltec WiFi LoRa 32 V2 utilizados

| Pino | Net | Função |
|------|-----|--------|
| 1 | GND | Terra |
| 2 | 5V | Alimentação 5V |
| 9 | PWM | Saída PWM → gate IGBT (via R3) |
| 11 | MOSI | SPI MOSI → SD Card |
| 14 | SCL | I²C Clock |
| 16 | SDA | I²C Data |
| 17 | SCK | SPI Clock → SD Card |
| 19 | DE | Controle direção MAX485 |
| 20 | MISO | SPI MISO ← SD Card |
| 21 | CS | SPI Chip Select → SD Card |
| 26 | DI | TX RS-485 → MAX485 DI |
| 27 | RO | RX RS-485 ← MAX485 RO |
