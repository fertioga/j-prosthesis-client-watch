# Feature Specification: Screen Rafael

**Feature Branch**: `001-screen-rafael`

**Created**: 2026-10-14

**Status**: Draft

**Input**: User description: "Crie o plano para criar o screen do Rafael de modo que a cor a ser enviada via bluetooth seja a cor vermelha. Mantenha a mesma estrutura e coloque o screen do Rafael logo depois da screen do Donatelo."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Ligar o LED vermelho na tela do Rafael (Priority: P1)

O usuário desliza o tileview até a tela do Rafael, que aparece imediatamente depois da tela do Donatelo, e segura o toque para acender o LED da prótese em vermelho.

**Why this priority**: É o comando novo da feature. Sem o envio vermelho, a tela não entrega feedback de engajamento.

**Independent Test**: Com o relógio conectado à prótese, abrir o tile Rafael e segurar o toque por cerca de 800 ms. O LED da prótese acende vermelho e o serial registra o comando ligado.

**Acceptance Scenarios**:

1. **Given** a prótese está conectada por BLE e a tela do relógio está ligada no tile Rafael, **When** o usuário mantém o toque por 800 ms, **Then** o relógio envia o comando de LED ligado na cor vermelha (R máximo, G e B zerados, brilho máximo).
2. **Given** o LED do Rafael já está ligado, **When** o usuário mantém o toque de novo por 800 ms no mesmo tile, **Then** o relógio envia o comando de LED desligado (todos os canais e o brilho zerados).
3. **Given** o envio BLE falha, **When** o hold completa, **Then** o estado local de ligado/desligado não inverte e a falha aparece no serial.

---

### User Story 2 - Navegar até o Rafael depois do Donatelo (Priority: P1)

O usuário percorre os tiles na ordem já existente e encontra Rafael como o próximo tile depois de Donatelo, com o mesmo tipo de tela (rótulo centralizado e gesto de hold).

**Why this priority**: A posição no tileview faz parte do pedido e define como a tela é alcançada.

**Independent Test**: A partir da home, deslizar horizontalmente e confirmar a sequência Home, Leonardo, Donatelo, Rafael.

**Acceptance Scenarios**:

1. **Given** o tileview horizontal está montado, **When** o usuário desliza da esquerda para a direita, **Then** a ordem dos tiles é Home (coluna 0), Leonardo (coluna 1), Donatelo (coluna 2) e Rafael (coluna 3).
2. **Given** o tile Rafael está visível, **When** a tela é desenhada, **Then** o rótulo `RAFAEL` aparece centralizado, no mesmo padrão visual da tela do Donatelo.

---

### User Story 3 - Hold só vale no tile ativo (Priority: P2)

O gesto de hold do Rafael não dispara comando quando a tela está apagada ou quando outro tile está ativo.

**Why this priority**: Evita comando acidental fora da tela do personagem, no mesmo comportamento das telas Leonardo e Donatelo.

**Independent Test**: Com o LED do Rafael desligado, segurar o toque na home, em Leonardo ou em Donatelo, e também com a tela em sleep. Nenhum comando vermelho é enviado.

**Acceptance Scenarios**:

1. **Given** a tela está em sleep ou o tile ativo não é o Rafael, **When** o usuário toca o display, **Then** a task do Rafael não envia comando BLE.
2. **Given** o usuário está no tile Leonardo ou Donatelo, **When** segura o toque, **Then** o comando continua sendo o daquela tela (azul e roxo, respectivamente), sem alteração.

---

### Edge Cases

- BLE desconectado no momento do hold: `send` retorna falha, o serial mostra a falha e `ledOn` permanece no valor anterior.
- Toque mais curto que 800 ms: nenhum comando é enviado; soltar o dedo reinicia o gesto.
- Segundo hold enquanto o dedo continua pressionado depois do primeiro envio: um único comando por pressionamento (`holdSent`).
- Tela apaga no meio do toque: o próximo ciclo deixa de considerar o tile ativo e o gesto é descartado.
- Não existe asset de imagem do Rafael: a tela usa rótulo, como Donatelo, sem novo bitmap.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: O firmware MUST registrar uma tela `screen_rafael` no tileview horizontal, na coluna imediatamente seguinte à do Donatelo (coluna 3).
- **FR-002**: A tela MUST seguir a mesma estrutura das telas de personagem já existentes: contexto tipado, task FreeRTOS de toque, tile com direção horizontal e rótulo centralizado `RAFAEL`.
- **FR-003**: Com a tela ligada e o tile Rafael ativo, um toque contínuo de 800 ms MUST enviar, via o cliente BLE já existente, o LED ligado em vermelho: vermelho `255`, verde `0`, azul `0`, brilho `255`, efeito ligado.
- **FR-004**: O hold seguinte, no mesmo critério, MUST enviar o LED desligado: vermelho, verde, azul e brilho `0`, efeito desligado.
- **FR-005**: A tela MUST chamar somente a API `WatchBleClient::send`. UUIDs, layout do payload e o server da prótese MUST permanecer inalterados.
- **FR-006**: Falha de envio MUST ser registrada no serial e MUST NOT inverter o estado local ligado/desligado.
- **FR-007**: A task MUST ignorar toque quando a tela está apagada ou quando o tile ativo não é o Rafael, e MUST ceder a CPU com atraso curto (~50 ms) em vez de bloquear o loop LVGL.
- **FR-008**: Leonardo e Donatelo MUST continuar enviando as cores que já enviam (azul e roxo). Home, sleep e wakeup MUST permanecer como estão.

### Key Entities

- **Tile Rafael**: coluna 3 do tileview; rótulo `RAFAEL`; só aceita o gesto de hold quando está ativo e a tela está ligada.
- **Estado do LED local**: booleano da task (`ligado` / `desligado`). Não é persistido. Só muda depois de um envio BLE bem-sucedido.
- **Comando de LED**: payload já existente (vermelho, verde, azul, brilho, efeito). Vermelho de ligar usa vermelho máximo e os outros canais de cor zerados.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Um hold de 800 ms no tile Rafael, com BLE conectado, acende o LED da prótese em vermelho no primeiro comando bem-sucedido.
- **SC-002**: O hold seguinte apaga o LED. A alternância ligado/desligado exige um novo pressionamento; um único toque longo não envia os dois comandos.
- **SC-003**: A ordem visual dos tiles é Home, Leonardo, Donatelo, Rafael, sem tile extra e sem reordenar as telas anteriores.
- **SC-004**: Com BLE desconectado, a UI do relógio continua navegável e o hold do Rafael não trava a tela; a falha fica visível no serial.
- **SC-005**: Holds em Leonardo e Donatelo continuam produzindo azul e roxo.

## Assumptions

- "Mesma estrutura" significa espelhar a tela do Donatelo (rótulo centralizado, sem imagem), porque o Rafael não tem bitmap e o Donatelo é o tile vizinho. A task de toque segue o hold de 800 ms já usado em Leonardo e Donatelo.
- Vermelho é o vermelho puro do contrato atual: canal vermelho no máximo, verde e azul em zero, brilho no máximo, efeito ligado. Desligar zera cor e brilho, como nas outras telas de personagem.
- Não há mudança de contrato BLE, de `platformio.ini`, de services, de widgets nem do firmware do server.
- O feedback de falha permanece no serial, no mesmo texto já usado pelas telas de personagem. Esta feature não adiciona indicador visual de conexão.
- O nome exibido na tela é `RAFAEL`, em maiúsculas, no mesmo estilo de `DONATELO`.
