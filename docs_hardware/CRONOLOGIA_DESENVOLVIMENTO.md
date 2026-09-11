# Cronologia de Desenvolvimento do Projeto

Este documento resume a linha do tempo de evolução estrutural e lógica do firmware, baseada no histórico de versionamento (Git) e nos diários de testes. Para ver o código exato de cada etapa descrita abaixo, consulte o arquivo `Evolucao_Codigo_Completa.diff`.

## Fase 1: Fundação Básica
*   **Leitura Inicial do PZEM-017:** O projeto começou apenas com a integração do medidor de energia via Modbus. Foi estabelecido o uso da serial e os comandos básicos.
*   **Calibração e UX:** Implementação dos comandos de calibração (`calv`, `cali`) pelo monitor serial e da funcionalidade de pausar/retomar a tela.

## Fase 2: Estruturação de Hardware e Sensores Adicionais
*   **Integração do Ambiente:** Adição da leitura dos sensores meteorológicos (Anemômetro e Biruta) no escopo do projeto, preparando a estrutura para os dados combinados.
*   **Correção Crítica (GPIO 21):** Resolvido o conflito de reset/energia no Display OLED causado pelo uso do Pino 21 (Vext), separando o controle de boot do pino de controle do MAX485.

## Fase 3: A Lógica do Traçador I-V (Curva Fotovoltaica)
*   **Implementação do PWM (IGBT):** Inserção do controle de largura de pulso (PWM) no botão PRG da placa para abrir e fechar a "válvula" IGBT conectada à carga resistiva.
*   **Ajuste do Shunt:** Configuração rígida para shunt de 50A e leitura em registradores de 16-bits conforme as especificações elétricas em teste.
*   **Máquina de Estados Finita (FSM) do Traçador I-V:** Implementação de varredura em 20 degraus de estabilização térmica de 500ms, em vez de leituras contínuas arriscadas. O sistema passou a tirar "snapshots" precisos a cada patamar.

## Fase 4: Sincronização, LoRa e Aperfeiçoamento Físico
*   **Divisão Emissor/Receptor (LoRa):** O código monolítico foi quebrado em duas tarefas concorrentes e distribuído via rádio.
*   **Sincronização do Pacote (Struct):** Implementado o empacotamento das 15 variáveis de `SensorData` via LoRa, assegurando alinhamento de memória para não perder dados climáticos durante a execução dos 10 segundos da curva I-V.
*   **Testes Extremos (Logbook):** Testes exaustivos com frequências (15kHz vs 20kHz) e resoluções de PWM (8-bits vs 12-bits). Um dos códigos de 12-bits resultou na queima da solda do IGBT por dissipação térmica excessiva, originando o registro histórico de não repetir o feito.
*   **Estabilidade do Display:** Ajustes finais e calibrações de boot para garantir o bom funcionamento ininterrupto sob condições agressivas de hardware.

## Referência Histórica Adicional
A pasta conta ainda com arquivos de museu:
*   `Logbook_Testes.md`: O diário documentado de todos os testes em bancada e campo.
*   `Historico_Codigos_Antigos.md`: Uma relíquia registrando as tentativas de código que deram errado (como o "Derretedor de IGBT") documentados explicitamente para não serem replicados.
