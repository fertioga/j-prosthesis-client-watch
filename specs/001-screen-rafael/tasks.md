---

description: "Task list for the Rafael screen feature"
---

# Tasks: Screen Rafael

**Input**: Design documents from `/specs/001-screen-rafael/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: Not requested. Validation is the manual quickstart on the LilyGo T-Watch. No test tasks.

**Organization**: Tasks are grouped by user story so each story can be implemented and checked on its own.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- Firmware do relógio: `src/ui/`, `src/services/` na raiz de `j-prosthesis-client-watch`
- Esta feature cria um header e altera o registro do tileview. Não cria `tests/`

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Criar o lugar da tela nova, sem mudar stack nem contrato BLE

- [X] T001 Create the directory `src/ui/components/screen_rafael/` for the new tile described in `specs/001-screen-rafael/plan.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Cabeçalho e contexto compartilhados pelas três histórias. O cliente BLE já existe e não é alterado.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T002 Create `src/ui/components/screen_rafael/screen_rafael.h` with `#include "bootstrap.h"` and `#include "services/bluetooth/WatchBleClient.h"`, and define `ScreenRafaelContext` with exactly these fields: `TTGOClass *ttgo`, `bool *screenOn`, `lv_obj_t *tv`, `lv_obj_t *tile_raf`, `WatchBleClient *ble`. Do not edit `src/services/bluetooth/WatchBleClient.h`

**Checkpoint**: Header e contexto prontos. `WatchBleClient::send(r, g, b, brightness, effect)` segue sendo a única API de comando

---

## Phase 3: User Story 1 - Ligar o LED vermelho na tela do Rafael (Priority: P1) 🎯 MVP

**Goal**: Um hold de 800 ms no tile Rafael envia vermelho pelo BLE; o hold seguinte desliga. Falha de envio não inverte o estado local.

**Independent Test**: Com a prótese conectada e a tela ligada no tile Rafael, segurar o toque por 800 ms acende o LED vermelho e o serial mostra `Rafael LED ON`. O hold seguinte apaga e mostra `Rafael LED OFF`. Se `send` falhar, o serial mostra `Falha ao enviar comando LED via BLE` e o próximo hold bem-sucedido ainda liga.

### Implementation for User Story 1

- [X] T003 [US1] Implement `screen_rafael_task_loop` in `src/ui/components/screen_rafael/screen_rafael.h`. Initialize `touching=false`, `holdSent=false`, `ledOn=false`, `pressStartedAt=0`. Use `const uint32_t HOLD_MS = 800`. On a new touch set `touching=true`, `holdSent=false`, and store `pressStartedAt`. When the press has lasted at least 800 ms and `holdSent` is false, send once and set `holdSent=true`. If `ledOn` is false call `ble->send(0xFF, 0x00, 0x00, 0xFF, LED_EFFECT_ON)` (r=255, g=0, b=0, brilho=255, efeito ligado=1). If `ledOn` is true call `ble->send(0, 0, 0, 0, LED_EFFECT_OFF)` (r=0, g=0, b=0, brilho=0, efeito desligado=0). Set `ledOn` only when `send` returns true. On failure `Serial.println("Falha ao enviar comando LED via BLE")` and leave `ledOn` unchanged. On success `Serial.println(ledOn ? "Rafael LED ON" : "Rafael LED OFF")`. End each iteration with `vTaskDelay(pdMS_TO_TICKS(50))`
- [X] T004 [US1] Add `screen_rafael_sleep_service_task` in `src/ui/components/screen_rafael/screen_rafael.h` that allocates `ScreenRafaelContext` and calls `xTaskCreate(screen_rafael_task_loop, "screen_rafael_task_loop", 2048, ctx, 1, nullptr)`

**Checkpoint**: A máquina de hold e os bytes vermelho/desligar estão no header. A checagem no relógio depende do tile registrado na Phase 4

---

## Phase 4: User Story 2 - Navegar até o Rafael depois do Donatelo (Priority: P1)

**Goal**: O tileview mostra Rafael na coluna 3, com o rótulo `RAFAEL` centralizado, no mesmo tipo de tela do Donatelo.

**Independent Test**: Da home, deslizar para a direita e ver a ordem Home, Leonardo, Donatelo, Rafael. O tile Rafael mostra só o texto `RAFAEL` no centro, sem imagem.

### Implementation for User Story 2

- [X] T005 [US2] Implement `void screen_rafael(lv_obj_t *tv, TTGOClass *ttgo, bool *screenOn, WatchBleClient *ble, int col)` in `src/ui/components/screen_rafael/screen_rafael.h`. Create the tile with `lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR)`, add a centered label whose text is `RAFAEL`, and call `screen_rafael_sleep_service_task`. Do not add an image
- [X] T006 [US2] Register the tile in `src/ui/screen_main.h`: include `ui/components/screen_rafael/screen_rafael.h` and, immediately after `screen_donatelo(tv, ttgo, *&screenOn, ble, 2)`, call `screen_rafael(tv, ttgo, *&screenOn, ble, 3)`. Leave Home at column 0 and Leonardo at column 1

**Checkpoint**: A ordem visual Home → Leonardo → Donatelo → Rafael está no firmware, e o hold da Phase 3 passa a ter tile ativo

---

## Phase 5: User Story 3 - Hold só vale no tile ativo (Priority: P2)

**Goal**: Toque fora do Rafael, tela apagada ou hold curto não envia comando. Leonardo e Donatelo continuam com as cores atuais.

**Independent Test**: Segurar o toque na home, em Leonardo, em Donatelo ou com a tela em sleep não produz linha `Rafael LED`. Leonardo continua azul (`0x00, 0x00, 0xFF`) e Donatelo continua roxo (`0x90, 0x13, 0xFE`). Um toque no Rafael solto antes de 800 ms não envia.

### Implementation for User Story 3

- [X] T007 [US3] In `screen_rafael_task_loop` inside `src/ui/components/screen_rafael/screen_rafael.h`, read `lv_obj_t *active_tile = lv_tileview_get_tile_act(tv)` and gate the gesture with a single `const bool onRaf = *screenOn && active_tile == tile_raf`. Send only inside `if (onRaf && ttgo->getTouch(x, y))`. In the else branch set `touching=false` and `holdSent=false`, and do not change `ledOn`. A press with `(millis() - pressStartedAt < 800)` must not call `send`. `holdSent` stays true until the finger lifts, so one press yields at most one command. Do not redeclare `onRaf` inside the touch branch
- [X] T008 [P] [US3] Leave the existing ON payloads untouched in `src/ui/components/screen_leonardo/screen_leonardo.h` (`ble->send(0x00, 0x00, 0xFF, 0xFF, LED_EFFECT_ON)`) and `src/ui/components/screen_donatelo/screen_donatelo.h` (`ble->send(0x90, 0x13, 0xFE, 0xFF, LED_EFFECT_ON)`)

**Checkpoint**: O gesto do Rafael fica isolado no próprio tile. As telas vizinhas seguem iguais

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Validar o quickstart e confirmar que o contrato BLE não foi mexido

- [ ] T009 [P] Run the six scenarios in `specs/001-screen-rafael/quickstart.md` on the `ttgo-t-watch` env (build with `pio run -e ttgo-t-watch`, serial 115200)
- [X] T010 [P] Confirm `src/services/bluetooth/WatchBleClient.h`, `platformio.ini`, `src/ui/components/screen_home/screen_home.h`, and `j-prosthesis-server/src/app/structs/LedPayloadBle.hpp` have no edits from this feature

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **Foundational (Phase 2)**: Depends on T001. Blocks every user story
- **User Story 1 (Phase 3)**: Depends on T002. The send logic does not depend on US2; the on-watch check does, because the tile is registered in T006
- **User Story 2 (Phase 4)**: Depends on T004 (the task starter called by `screen_rafael`)
- **User Story 3 (Phase 5)**: Depends on T003 (same loop). T008 has no code dependency on T007
- **Polish (Phase 6)**: Depends on T006 and T007

### User Story Dependencies

- **User Story 1 (P1)**: Starts after Phase 2. No code dependency on US2 or US3
- **User Story 2 (P1)**: Starts after T004. Adds the tile and the column-3 registration that make US1 reachable
- **User Story 3 (P2)**: Starts after T003. Tightens the same loop; does not change the red payload

### Within Each User Story

- US1: task loop (T003) before `xTaskCreate` (T004)
- US2: `screen_rafael()` (T005) before the `screen_main.h` call (T006)
- US3: guard in the Rafael loop (T007). The Leonardo/Donatelo check (T008) does not wait for T007

### Parallel Opportunities

- T008 can run beside T007 (different files)
- T009 and T010 can run together after the stories
- US1 and US2 both edit `screen_rafael.h`, so T003, T004, and T005 stay sequential

---

## Parallel Example: User Story 3

```bash
# After T003, these touch different files:
Task: "T007 Gate screen_rafael_task_loop in src/ui/components/screen_rafael/screen_rafael.h"
Task: "T008 Leave Leonardo and Donatelo ON payloads unchanged"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1 and Phase 2
2. Complete Phase 3 (red on / off in the task loop)
3. Complete T005 and T006 so the tile exists at column 3
4. **STOP and VALIDATE**: hold 800 ms on Rafael turns the prosthesis LED red; the next hold turns it off

US1 sozinha não aparece no relógio sem o registro da Phase 4. O menor demo é Phase 3 mais T005 e T006.

### Incremental Delivery

1. Setup + context → header pronto
2. US1 → comando vermelho/desligar na task
3. US2 → tile `RAFAEL` depois do Donatelo; primeiro demo no pulso
4. US3 → hold ignorado fora do tile; Leonardo e Donatelo intactos
5. Polish → quickstart completo

### Parallel Team Strategy

Uma pessoa. O único par real é T007 com T008, e T009 com T010. O header do Rafael é editado em sequência.

---

## Notes

- [P] tasks = different files, no unfinished dependency
- [US1], [US2], and [US3] map to the stories in `specs/001-screen-rafael/spec.md`
- Do not copy the inner `const bool onDon` redeclaration from `src/ui/components/screen_donatelo/screen_donatelo.h`
- Commit after each phase if you want reviewable steps
