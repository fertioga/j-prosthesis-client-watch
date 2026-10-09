# J-Prosthesis Client Watch Constitution

## Core Principles

### I. Propósito Clínico Primário
O firmware do relógio existe para apoiar o uso da prótese transtibial
no dia a dia: controle remoto de feedback (LED/som), status da conexão
com o server e, quando especificado, visualização resumida de estado.
Toda feature MUST contribuir, direta ou indiretamente, para controle
da prótese, feedback ao usuário, confiabilidade da sessão BLE ou
usabilidade vestível. Features sem vínculo clínico ou operacional
claro NÃO DEVEM ser aceitas.

**Rationale**: O valor do produto é clínico/assistivo; o relógio é o
controle vestível do server, não um app genérico de smartwatch.

### II. Usabilidade Vestível e Energia (NON-NEGOTIABLE)
O client roda em smartwatch com bateria limitada e MUST priorizar
resposta a toque, sleep de tela e estabilidade da sessão BLE.
Operações de I/O (BLE, Wi-Fi/NTP, power) MUST NÃO bloquear o loop
LVGL indevidamente; trabalho contínuo MUST usar FreeRTOS
(tasks/queues) quando houver polling ou I/O longo. Falha de BLE,
Wi-Fi ou NTP MUST degradar com segurança (UI permanece utilizável;
reconexão/retry sem travar o device).

**Rationale**: Relógio no pulso; tela sempre ligada, UI travada ou
reconnect agressivo destroem autonomia e confiança no uso clínico.

### III. Contrato BLE como Cliente Explícito
A comunicação com a prótese (server) MUST ser bidirecional quando o
contrato do server assim definir, e MUST consumir UUIDs, payloads e
schemas **versionados** definidos pelo server (fonte de verdade do
contrato). O client MUST NÃO inventar UUIDs/payloads incompatíveis
nem “adivinhar” bytes fora do contrato documentado. Comandos do
relógio (ex.: efeito/cor de LED, som de passo) e leituras do server
(ex.: status, telemetria resumida, bateria da prótese) MUST evoluir
com bump de versão do contrato. Em desconexão, o client MUST
expor estado claro na UI e tentar reestabelecer a sessão sem
corrupção de comando.

**Rationale**: Relógio e prótese evoluem em repositórios separados;
o client é consumidor da API pública BLE do server.

### IV. Offline-First de Sessão (Degradação Controlada)
Ausência de BLE, Wi-Fi ou NTP NÃO DEVE impedir boot da UI nem sleep/
wakeup. Funções locais (relógio, bateria do watch, navegação de
telas) MUST permanecer disponíveis. Sync de tempo (NTP) e qualquer
caminho futuro para nuvem MUST ser opcionais ao runtime crítico de
controle da prótese. Credenciais e configs de rede NÃO DEVEM ser
commitadas em texto claro no repositório; MUST usar mecanismo
documentado (build flags, arquivo local não versionado ou equivalente).

**Rationale**: Uso clínico ocorre fora de Wi-Fi; o controle da
prótese depende de BLE, não de internet.

### V. Arquitetura Modular (UI / Services / Config)
Hardware e infra (BLE client, sleep de tela, Wi-Fi, NTP, power/
bateria) MUST ser encapsulados em services com APIs claras. Telas e
widgets LVGL MUST ficar em `ui/` e NÃO devem embutir detalhes de
protocolo BLE além de chamar o service. Novas funções MUST estender
services/UI existentes ou criar módulos isolados — NÃO acoplar
domínio direto em `main` além do wiring. `loop()` MUST permanecer
fino (ex.: `lv_task_handler` + yield).

**Rationale**: O código já caminha para `services/` + `ui/` +
`config/`; a constituição consolida isso para escalar comandos e
status sem spaghetti.

### VI. Feedback Lúdico Separado de Dados Clínicos
Efeitos e metáforas de UI (personagens, animações, toques que
disparam LED/som na prótese) MUST ser tratados como feedback de
engajamento. O client MUST NÃO apresentar feedback lúdico como
substituto de telemetria clínica nem adulterar/omitir status
operacional relevante (conectado/desconectado, falha de comando,
bateria crítica quando exposta). Quando houver telemetria resumida
na UI, unidades e significado MUST ser explícitos e auditáveis.

**Rationale**: Fisioterapeutas/ortopedistas e o paciente precisam
distinguir “efeito divertido” de informação operacional/clínica.

### VII. Simplicidade Embarcada (YAGNI)
O client MUST permanecer enxuto: LilyGo T-Watch (plataforma
documentada) + Arduino/PlatformIO, LVGL via stack do board, NimBLE
como cliente BLE, FreeRTOS e periféricos necessários. NÃO introduzir
stacks, middlewares ou abstrações “para o futuro” sem feature
aprovada. Complexidade extra MUST ser justificada em review
(flash, RAM, energia, risco de regressão na UI).

**Rationale**: Recursos finitos no watch; over-engineering aumenta
risco em dispositivo clínico-assistivo vestível.

## Platform & Scope Constraints

- Escopo desta constituição: **somente**
  `j-prosthesis-client-watch` (firmware do smartwatch). O server da
  prótese, backend cloud e dashboard são sistemas vizinhos; mudanças
  neles NÃO DEVEM ser feitas sob esta constituição, mas contratos
  compartilhados (BLE/schema) MUST permanecer coerentes com o server.
- Stack canônica: PlatformIO, `espressif32`, board `ttgo-t-watch`
  (LilyGo T-Watch 2020 V3 / `LILYGO_WATCH_2020_V3`), framework
  Arduino, FreeRTOS, LVGL (via TTGO TWatch Library), NimBLE-Arduino
  como **BLE client**.
- Funções de produto no client incluem, no mínimo: UI touch
  (navegação por tiles/telas), sleep/wakeup de display, leitura de
  bateria do watch, cliente BLE para comandos à prótese e, quando
  o contrato evoluir, leitura de status/telemetria resumida.
- Wi-Fi/NTP são suporte (hora, sync eventual); NÃO fazem parte do
  caminho crítico de comando BLE à prótese.
- Sync cloud e dashboard estão fora do runtime crítico do watch;
  o client MUST apenas preparar UI/comandos e, quando houver caminho
  definido, interagir com schemas estáveis.

## Privacy, Credentials & Session Integrity

- Dados de sessão e qualquer identificador de paciente, se existirem,
  são tratados como **dados sensíveis de saúde assistiva**: NÃO logar
  PII no Serial em builds de produção.
- Credenciais Wi-Fi, chaves e segredos NÃO DEVEM versionar no git;
  configs locais MUST ser excluídas ou injetadas no build.
- Contratos BLE consumidos MUST referenciar a mesma versão/schema
  documentada no server; breaking changes exigem bump MAJOR do
  contrato e atualização coordenada client/server.
- TODO(PRIVACY_POLICY): retenção/consentimento alinhados ao backend
  cloud e à constituição do server.
- TODO(BLE_RECONNECT_POLICY): política de scan/reconnect/backoff e
  feedback visual de estado de conexão.
- TODO(UX_COMMAND_MAP): mapa estável tela/gesto → comando BLE
  (alinhado a `led_codes` / structs do server).

## Governance

Esta constituição prevalece sobre convenções ad hoc no repositório
`j-prosthesis-client-watch`. Amendments MUST: (1) atualizar este
arquivo, (2) bumpar `CONSTITUTION_VERSION` (MAJOR = remoção/
redefinição incompatível; MINOR = novo princípio/seção; PATCH =
esclarecimento), (3) registrar `Last Amended` em ISO-8601,
(4) remover o Sync Impact Report antes do commit final. PRs e
reviews MUST verificar conformidade com os princípios I–VII.
Ambiguidades de implementação MUST ser resolvidas via
`/speckit-specify` (ou equivalente), não por bypass silencioso da
constituição. Em conflito de contrato BLE, a constituição do
**server** define o schema; o client MUST adaptar-se ou negociar
bump versionado.

**Version**: 1.0.0 | **Ratified**: 2026-10-08 | **Last Amended**: 2026-10-08
