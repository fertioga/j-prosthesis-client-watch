# Feature Specification: Screen Michelangelo

**Feature Branch**: `002-screen-michelangelo`

**Created**: 2026-10-15

**Status**: Draft

**Input**: User description: "Crie um plano para criar mais uma screen. Será a screen_michelangelo. A cor do led será laranja."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Ligar o LED laranja na tela do Michelangelo (Priority: P1)

O usuário desliza o tileview até a tela do Michelangelo e segura o toque para acender o LED da prótese em laranja.

**Why this priority**: É o comando novo da feature. Sem o envio laranja, a tela não entrega feedback de engajamento.

**Independent Test**: Com o relógio conectado à prótese, abrir o tile Michelangelo e segurar o toque por cerca de 800 ms. O LED da prótese acende laranja e o serial registra o comando ligado.

**Acceptance Scenarios**:

1. **Given** a prótese está conectada por BLE, a tela do relógio está ligada no tile Michelangelo e a cor compartilhada do LED não é laranja, **When** o usuário mantém o toque por 800 ms, **Then** o relógio envia o comando de LED ligado na cor laranja (vermelho `255`, verde `165`, azul `0`, brilho máximo, efeito ligado) e a cor compartilhada passa a ser essa laranja.
2. **Given** a cor compartilhada do LED já é a laranja do Michelangelo, **When** o usuário mantém o toque de novo por 800 ms no mesmo tile, **Then** o relógio envia o comando de LED desligado (todos os canais e o brilho zerados) e a cor compartilhada volta a zero.
3. **Given** o envio BLE falha, **When** o hold completa, **Then** a cor compartilhada não muda e a falha aparece no serial.

---

### User Story 2 - Navegar até o Michelangelo depois do Rafael (Priority: P1)

O usuário percorre os tiles na ordem já existente e encontra Michelangelo como o próximo tile depois do Rafael, com o mesmo tipo de tela (rótulo centralizado e gesto de hold).

**Why this priority**: "Mais uma screen" acrescenta um tile ao fim da sequência atual. Sem essa posição, a tela nova não é alcançável no tileview.

**Independent Test**: A partir da home, deslizar horizontalmente e confirmar a sequência Home, Leonardo, Donatelo, Rafael, Michelangelo.

**Acceptance Scenarios**:

1. **Given** o tileview horizontal está montado, **When** o usuário desliza da esquerda para a direita, **Then** a ordem dos tiles é Home (coluna 0), Leonardo (coluna 1), Donatelo (coluna 2), Rafael (coluna 3) e Michelangelo (coluna 4).
2. **Given** o tile Michelangelo está visível, **When** a tela é desenhada, **Then** o rótulo `MICHELANGELO` aparece centralizado, no mesmo padrão visual da tela do Rafael.

---

### User Story 3 - Hold só vale no tile ativo (Priority: P2)

O gesto de hold do Michelangelo não dispara comando quando a tela está apagada ou quando outro tile está ativo.

**Why this priority**: Evita comando acidental fora da tela do personagem, no mesmo comportamento das telas Leonardo, Donatelo e Rafael.

**Independent Test**: Com o LED apagado, segurar o toque na home, em Leonardo, em Donatelo ou em Rafael, e também com a tela em sleep. Nenhum comando laranja é enviado.

**Acceptance Scenarios**:

1. **Given** a tela está em sleep ou o tile ativo não é o Michelangelo, **When** o usuário toca o display, **Then** a task do Michelangelo não envia comando BLE.
2. **Given** o usuário está no tile Leonardo, Donatelo ou Rafael, **When** segura o toque, **Then** o comando continua sendo o daquela tela (azul, roxo e vermelho, respectivamente), sem alteração desta feature.

---

### Edge Cases

- BLE desconectado no momento do hold: `send` retorna falha, o serial mostra a falha e a cor compartilhada permanece no valor anterior.
- Toque mais curto que 800 ms: nenhum comando é enviado; soltar o dedo reinicia o gesto.
- Segundo hold enquanto o dedo continua pressionado depois do primeiro envio: um único comando por pressionamento (`holdSent`).
- Tela apaga no meio do toque: o próximo ciclo deixa de considerar o tile ativo e o gesto é descartado.
- Outra tela de personagem deixou o LED ligado numa cor diferente: o hold do Michelangelo troca para laranja, porque a cor compartilhada não é a laranja. Só um hold com a laranja já ativa apaga o LED.
- Não existe asset de imagem do Michelangelo: a tela usa rótulo, como Rafael e Donatelo, sem novo bitmap.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: O firmware MUST registrar uma tela `screen_michelangelo` no tileview horizontal, na coluna imediatamente seguinte à do Rafael (coluna 4).
- **FR-002**: A tela MUST seguir a mesma estrutura das telas de personagem já existentes: contexto tipado, task FreeRTOS de toque, tile com direção horizontal, ponteiro para a cor compartilhada e rótulo centralizado `MICHELANGELO`.
- **FR-003**: Com a tela ligada, o tile Michelangelo ativo e a cor compartilhada diferente de laranja, um toque contínuo de 800 ms MUST enviar, via `apply_screen_led`, o LED ligado em laranja: vermelho `255`, verde `165`, azul `0`, brilho `255`, efeito ligado.
- **FR-004**: O hold seguinte, quando a cor compartilhada já é essa laranja, MUST enviar o LED desligado: vermelho, verde, azul e brilho `0`, efeito desligado, e zerar a cor compartilhada.
- **FR-005**: A tela MUST chamar somente `apply_screen_led` (que usa `WatchBleClient::send`). UUIDs, layout do payload, `led_rgb.h` e o server da prótese MUST permanecer inalterados.
- **FR-006**: Falha de envio MUST ser registrada no serial e MUST NOT alterar a cor compartilhada.
- **FR-007**: A task MUST ignorar toque quando a tela está apagada ou quando o tile ativo não é o Michelangelo, e MUST ceder a CPU com atraso curto (~50 ms) em vez de bloquear o loop LVGL.
- **FR-008**: Leonardo, Donatelo e Rafael MUST continuar enviando as cores que já enviam (azul, roxo e vermelho). Home, sleep e wakeup MUST permanecer como estão.

### Key Entities

- **Tile Michelangelo**: coluna 4 do tileview; rótulo `MICHELANGELO`; só aceita o gesto de hold quando está ativo e a tela está ligada.
- **Cor compartilhada do LED**: `LedRgb` único, criado em `screen_main` e entregue a todas as telas de personagem. Não é persistido. Só muda depois de um envio BLE bem-sucedido.
- **Comando de LED**: payload já existente (vermelho, verde, azul, brilho, efeito). Laranja de ligar usa vermelho `255`, verde `165` e azul `0`.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Um hold de 800 ms no tile Michelangelo, com BLE conectado e LED apagado ou noutra cor, acende o LED da prótese em laranja no primeiro comando bem-sucedido.
- **SC-002**: O hold seguinte, com a laranja já ativa, apaga o LED. A alternância exige um novo pressionamento; um único toque longo não envia os dois comandos.
- **SC-003**: A ordem visual dos tiles é Home, Leonardo, Donatelo, Rafael, Michelangelo, sem tile extra e sem reordenar as telas anteriores.
- **SC-004**: Com BLE desconectado, a UI do relógio continua navegável e o hold do Michelangelo não trava a tela; a falha fica visível no serial.
- **SC-005**: Holds em Leonardo, Donatelo e Rafael continuam produzindo azul, roxo e vermelho.

## Assumptions

- "Mais uma screen" acrescenta o tile no fim da sequência atual, coluna 4, imediatamente depois do Rafael. O pedido não pede reordenar Home, Leonardo, Donatelo ou Rafael.
- "Mesma estrutura" significa espelhar a tela do Rafael (rótulo centralizado, `apply_screen_led`, cor compartilhada `LedRgb`), porque o Rafael é o tile vizinho e já usa esse fluxo. A task de toque segue o hold de 800 ms já usado nas telas de personagem.
- Laranja é o laranja nomeado padrão `#FFA500`: canal vermelho `255`, verde `165`, azul `0`, brilho no máximo, efeito ligado. Desligar zera cor e brilho, como nas outras telas de personagem, pela regra já existente em `apply_screen_led`.
- Não há mudança de contrato BLE, de `platformio.ini`, de services, de `led_rgb.h`, de widgets nem do firmware do server.
- O feedback de falha permanece no serial, no mesmo texto já usado pelas telas de personagem. Esta feature não adiciona indicador visual de conexão.
- O nome exibido na tela é `MICHELANGELO`, em maiúsculas, no mesmo estilo de `RAFAEL`.
