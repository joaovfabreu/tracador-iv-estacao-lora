# Base Teórica do Projeto: Receptor e Traçador I-V

Este documento compila a teoria necessária para compreender o hardware do projeto, focando no medidor de energia, comunicação serial e o circuito de potência usado para traçar curvas I-V.

## 1. O Medidor de Energia DC (PZEM-017)

O **PZEM-017** é um módulo medidor de energia de corrente contínua (DC) capaz de medir Tensão, Corrente, Potência e Energia.

*   **Funcionamento Base:** Ele usa um divisor de tensão interno para medir a voltagem (suporta até 300V). Para a medição de corrente, ele requer um **Resistor Shunt** externo. O Shunt é um resistor de precisão de valor muito baixo que causa uma queda de tensão em milivolts proporcional à corrente que o atravessa. O PZEM lê essa queda de tensão e a converte internamente em Amperes.
*   **Comunicação:** Diferente de sensores simples com saída analógica, ele possui um chip que digitaliza as medições e as envia através de uma porta serial usando o protocolo de rede **Modbus-RTU** sobre o padrão físico **RS-485**.
*   **Acesso aos Dados:** O PZEM armazena as leituras em "registradores" de memória internos. O microcontrolador (ESP32) envia um pacote de solicitação (comando Modbus 0x04 - *Read Input Registers*), e o PZEM responde devolvendo os pacotes contendo os valores exatos de tensão, corrente, etc.

## 2. O Padrão de Comunicação RS-485

O microcontrolador (ESP32) funciona com sinais seriais convencionais (UART - Pinos RX e TX), operando em níveis lógicos de 0 a 3.3V referenciados ao Terra (GND). Esse tipo de sinal é eletricamente frágil e suscetível a ruído e distorção em fios longos.

O **RS-485** é um padrão físico de comunicação industrial que resolve isso utilizando **sinalização diferencial**.
*   Em vez de ler a tensão de um fio em relação ao GND, o sinal viaja em um par de fios trançados (Linha **A** e Linha **B**).
*   Os bits de informação são definidos pela diferença de tensão entre as duas linhas. (Se A > B é um estado; se B > A é outro estado).
*   **Vantagem (Imunidade a Ruído):** Se uma interferência eletromagnética (ruído) atingir o cabo, ela afetará os fios A e B igualmente, elevando a tensão de ambos. Como o receptor lê apenas a *diferença* entre eles, a flutuação idêntica é cancelada matematicamente (Rejeição de Modo Comum).
*   **Topologia:** É um barramento *Half-Duplex*, o que significa que o mesmo par de fios é usado tanto para transmitir quanto para receber, mas a rede só pode fazer uma dessas ações por vez (falar ou escutar).

## 3. O Módulo Conversor MAX485 (A Interface)

Como o ESP32 não consegue gerar os sinais diferenciais do RS-485 nativamente, usamos o módulo **MAX485** como ponte e tradutor elétrico.

*   **Pinos DI e RO (Lado ESP32):** O pino **DI** (Data Input) recebe o sinal TX (transmissão) do ESP32. O pino **RO** (Receiver Output) devolve o sinal convertido para o RX do ESP32.
*   **Pinos DE e RE (Controle de Fluxo):** Sendo uma rede *half-duplex*, o MAX485 precisa ser avisado se deve funcionar como alto-falante (transmitindo) ou como microfone (escutando).
    *   **RE** (*Receiver Enable*) ativa a escuta quando recebe um sinal de nível baixo (LOW).
    *   **DE** (*Driver Enable*) ativa a transmissão quando recebe um sinal de nível alto (HIGH).
    *   No nosso projeto, unimos esses dois pinos fisicamente com um jumper e os controlamos simultaneamente usando apenas um pino do ESP32 (GPIO 21 no Receptor).
    *   **Quando o ESP32 quer enviar uma pergunta ao PZEM:** Ele coloca o GPIO 21 em HIGH. O MAX485 abre a transmissão e empurra os dados do DI para os fios A/B.
    *   **Imediatamente após enviar:** Ele coloca o GPIO 21 em LOW. O MAX485 silencia a transmissão e fica escutando os fios A/B, esperando a resposta do PZEM para enviá-la ao pino RO.

## 4. O Circuito de Potência (Traçador I-V com IGBT)

Para extrair a Curva I-V (Corrente x Tensão) de um painel solar (ou gerador), precisamos conectá-lo a uma carga que possa variar seu "peso" (esforço exigido) do circuito aberto (corrente zero) até o curto-circuito (tensão zero), medindo a energia em dezenas de pontos no meio do caminho. Fazemos isso usando um **Transistor IGBT (IRGP50B60PD1)** operando no limite linear/comutação rápida junto a resistores de carga.

### A Mecânica da Varredura
1.  **Carga Resistiva Máxima:** Possuímos dois resistores cerâmicos de potência (10W, 8Ω) ligados em série (R4 e R5). Isso forma a "pia" de energia de 16Ω conectada aos terminais de geração.
2.  **O Dreno (IGBT):** O IGBT (U2) atua como uma torneira eletrônica colocada em série com esses resistores e o Terra (GND) do painel. 
    *   Se o Gate do IGBT não recebe sinal, a "torneira" fecha 100%. A corrente é zero, e o painel opera no seu estado natural de Tensão Máxima de Circuito Aberto (Voc).
    *   Se o Gate é ativado totalmente, o IGBT satura e deixa a corrente fluir através dos 16Ω dos resistores dissipadores.
3.  **Controle Progressivo via PWM:** O ESP32 possui um pino (GPIO 9) conectado ao Gate do IGBT (através do resistor limitador R3 de 1kΩ). Em vez de só ligar/desligar, o ESP32 envia um sinal **PWM** (Pulse Width Modulation), uma onda quadrada de alta frequência cuja largura (duty cycle) pode variar de 255 (sinal contínuo alto) até 0 (desligado).
4.  **Máquina de Estados de Teste:** Quando o botão é pressionado, o ESP32 inicia uma varredura decremental (Sweep). O PWM começa no máximo (impondo alta corrente na carga) e desce até zero ao longo de 10 segundos, em 20 degraus de 500 milissegundos cada.
5.  **Captura (Snapshot) pelo PZEM:** O PZEM-017 está conectado paralelamente aos polos (para ler a tensão do painel) e em série usando o Shunt (para ler a corrente). Ao longo da descida do PWM, a tensão média percebida na malha sobe e a corrente despenca. Imediatamente após estabilizar cada um dos 20 degraus de PWM, o ESP32 usa o MAX485 para gritar um comando de leitura ao PZEM. Os dados de Tensão e Corrente daquele instante exato são recebidos, processados e empacotados pelo ESP32, e gravados como uma linha individual no Cartão SD para posterior plotagem da Curva I-V no relatório.
