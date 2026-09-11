# Pacote de Entrega Técnica: Traçador de Curva I-V e Estação LoRa

Este repositório contém o firmware completo, os esquemáticos elétricos, manuais de componentes e dados reais de ensaio para replicação ou continuação do desenvolvimento do **Traçador de Curva I-V Fotovoltaica e Monitoramento Meteorológico sem Fio**.

---

## 1. Visão Geral do Sistema

O sistema é baseado em microcontroladores **ESP32 (Heltec WiFi LoRa 32 V2)** e é dividido em dois módulos cooperativos:

1. **Módulo Emissor (Estação Meteorológica):**
   * Coleta dados climáticos (radiação solar, anemômetro RS-485, biruta RS-485, umidade e temperatura).
   * Empacota as grandezas na struct binária `SensorData` e transmite periodicamente via rádio **LoRa (915 MHz)**.
2. **Módulo Receptor + Traçador de Carga Ativa (IGBT):**
   * Recebe as variáveis meteorológicas instantâneas via rádio LoRa.
   * Modula o gate de um transistor **IGBT (IRGP50B60PD1)** conectado em série com resistores dissipadores cerâmicos (16 Ω) via sinal **PWM (10 bits, 15 kHz / 5 kHz ajustável)**.
   * Lê simultaneamente a Tensão e a Corrente através do medidor DC **PZEM-017** sobre rede **RS-485 / Modbus-RTU** (usando transceptor MAX485).
   * Registra as curvas levantadas diretamente em arquivos `.csv` no **Cartão SD** e disponibiliza um **Dashboard Web local via Wi-Fi** para visualização, disparo remoto, ajuste de limites e calibração de escalas.

---

## 2. Estrutura do Diretório

```
Pacote_Entrega_Tracador_IV/
├── firmware/
│   ├── platformio.ini               # Arquivo de compilação PlatformIO pronto para Heltec V2
│   └── src/
│       ├── main_receptor.cpp        # Firmware definitivo do receptor e traçador (479 linhas)
│       ├── main_emissor.cpp         # Firmware do transmissor de telemetria LoRa
│       ├── web_dashboard.h          # Interface HTML/CSS/JS do dashboard web embarcado
│       ├── web_teste.h              # Páginas de calibração e visualização de curvas
│       └── secrets.h.example        # Template de credenciais Wi-Fi (renomear para secrets.h)
├── docs_hardware/
│   ├── 6221-Peacefair-PZEM-003-DC-elektromr.pdf   # Manual oficial do medidor PZEM
│   ├── RSM100P-EX.pdf                              # Manual do painel solar de teste
│   ├── Schematic_placa_Rorgerio_IGBT_2026-05-08.pdf# Esquemático oficial da placa de potência
│   ├── SCH_placa_Rorgerio_IGBT_2026-07-28.json    # Projeto nativo para importação no EasyEDA
│   ├── CONEXOES_PINAGEM.md                         # Mapeamento completo de pinos e ligações
│   ├── ESQUEMATICO_CIRCUITO.md                     # Lista de componentes e circuito IGBT
│   ├── BASE_TEORICA_SISTEMA.md                     # Fundamentação física (RS-485, Modbus, Sweep FSM)
│   └── CRONOLOGIA_DESENVOLVIMENTO.md               # Linha do tempo dos testes e decisões de projeto
└── dados_referencia/
    └── iv_test_152.csv                             # Curva I-V real obtida em bancada (Golden Run)
```

---

## 3. Como Começar (Quickstart)

### Configuração do Ambiente de Firmware
1. Abra a pasta `firmware` no **VS Code com a extensão PlatformIO**.
2. Na pasta `firmware/src`, copie o arquivo `secrets.h.example` para `secrets.h`:
   ```bash
   cp firmware/src/secrets.h.example firmware/src/secrets.h
   ```
3. Edite o `secrets.h` com as credenciais da sua rede Wi-Fi local.
4. Para compilar e gravar o receptor:
   ```bash
   pio run -t upload -e receptor
   ```
5. Para compilar e gravar o emissor:
   ```bash
   pio run -t upload -e emissor
   ```

### Parâmetros de Hardware Críticos
* **Shunt do PZEM:** O medidor PZEM-017 está configurado e calibrado para operante com shunt resistivo de **50 A**.
* **Gate do IGBT:** Sinal PWM gerado no pino **GPIO 22** (com resistor limitador de 1 kΩ e filtro RC estabilizador no gate).
* **Controle Half-Duplex RS-485:** Os pinos `DE` e `RE` do módulo MAX485 estão jumpeados juntos e ligados ao **GPIO 21** do ESP32.
* **Segurança Térmica:** O tempo de estabilização do sweep está fixado em **500 ms por ponto**. Não reduza esse intervalo sem dissipação ativa forçada nos resistores cerâmicos.
