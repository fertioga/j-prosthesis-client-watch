---

description: "Task list for the Michelangelo screen feature"
---

# Tasks: Screen Michelangelo

**Input**: Design documents from `/specs/002-screen-michelangelo/`

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

- [X] T001 Create the directory `src/ui/components/screen_michelangelo/` for the new tile described in `specs/002-screen-michelangelo/plan.md`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Cabeçalho e contexto compartilhados pelas três histórias. O cliente BLE e `apply_screen_led` já existem e não são alterados.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T002 Create `src/ui/components/screen_michelangelo/screen_michelangelo.h` with `#include "bootstrap.h"`, `#include "services/bluetooth/WatchBleClient.h"`, and `#include "ui/led_rgb.h"`, and define `ScreenMichelangeloContext` with exactly these fields: `TTGOClass *ttgo`, `bool *screenOn`, `lv_obj_t *tv`, `lv_obj_t *tile_mic`, `WatchBleClient *ble`, `LedRgb *led`. Do not edit `src/services/bluetooth/WatchBleClient.h` or `src/ui/led_rgb.h`. Do not declare a second `LedRgb`; the shared one already lives in `src/ui/screen_main.h` as `static LedRgb led{0, 0, 0}` with `r`, `g`, and `b` as `uint8` starting at 0

**Checkpoint**: Header e contexto prontos. O comando continua saindo só por `apply_screen_led` → `WatchBleClient::send`

---

## Phase 3: User Story 1 - Ligar o LED laranja na tela do Michelangelo (Priority: P1) 🎯 MVP

**Goal**: Um hold de 800 ms no tile Michelangelo envia laranja pelo BLE quando a cor compartilhada ainda não é laranja; o hold seguinte desliga. Falha de envio não altera `LedRgb`.

**Independent Test**: Com a prótese conectada, a tela ligada no tile Michelangelo e o LED apagado, segurar o toque por 800 ms acende o LED laranja (`255, 165, 0`) e o serial mostra `Michelangelo LED ON`. O hold seguinte apaga e mostra `Michelangelo LED OFF`. Se `send` falhar, o serial mostra `Falha ao enviar comando LED via BLE` e o próximo hold bem-sucedido ainda liga.

### Implementation for User Story 1

- [X] T003 [US1] Implement `screen_michelangelo_task_loop` in `src/ui/components/screen_michelangelo/screen_michelangelo.h`. Define `static constexpr uint8_t MIC_LED_R = 0xFF`, `MIC_LED_G = 0xA5`, `MIC_LED_B = 0x00` (ligar: r=255, g=165, b=0, brilho=255, efeito ligado=1; desligar: r=0, g=0, b=0, brilho=0, efeito desligado=0). Initialize `touching=false`, `holdSent=false`, `pressStartedAt=0`. Use `const uint32_t HOLD_MS = 800`. On a new touch set `touching=true`, `holdSent=false`, and store `pressStartedAt`. When the press has lasted at least 800 ms and `holdSent` is false, call `apply_screen_led(ble, led, MIC_LED_R, MIC_LED_G, MIC_LED_B)` once and set `holdSent=true`. Do not write `LedRgb` in this loop: `apply_screen_led` sends ON when the shared color is not already `(255, 165, 0)` and OFF when it is, and it updates `LedRgb` only if `send` returns true. On failure `Serial.println("Falha ao enviar comando LED via BLE")` and leave `LedRgb` unchanged. On success, `const bool ledOff = led->r == 0 && led->g == 0 && led->b == 0` then `Serial.println(ledOff ? "Michelangelo LED OFF" : "Michelangelo LED ON")`. End each iteration with `vTaskDelay(pdMS_TO_TICKS(50))`
- [X] T004 [US1] Add `screen_michelangelo_sleep_service_task` in `src/ui/components/screen_michelangelo/screen_michelangelo.h` that allocates `ScreenMichelangeloContext` and calls `xTaskCreate(screen_michelangelo_task_loop, "screen_michelangelo_task_loop", 2048, ctx, 1, nullptr)`

**Checkpoint**: A máquina de hold e os bytes laranja/desligar estão no header. A checagem no relógio depende do tile registrado na Phase 4

---

## Phase 4: User Story 2 - Navegar até o Michelangelo depois do Rafael (Priority: P1)

**Goal**: O tileview mostra Michelangelo na coluna 4, linha 0, direção horizontal (`LV_DIR_HOR`), com o rótulo `MICHELANGELO` centralizado, no mesmo tipo de tela do Rafael.

**Independent Test**: Da home, deslizar para a direita e ver a ordem Home, Leonardo, Donatelo, Rafael, Michelangelo. O tile Michelangelo mostra só o texto `MICHELANGELO` no centro, sem imagem.

### Implementation for User Story 2

- [X] T005 [US2] Implement `void screen_michelangelo(lv_obj_t *tv, TTGOClass *ttgo, bool *screenOn, WatchBleClient *ble, LedRgb *led, int col)` in `src/ui/components/screen_michelangelo/screen_michelangelo.h`. Create the tile with `lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR)` (coluna 4, linha 0, direção horizontal), store it in `tile_mic`, add a centered label whose text is `MICHELANGELO`, and call `screen_michelangelo_sleep_service_task`. Do not add an image
- [X] T006 [US2] Register the tile in `src/ui/screen_main.h`: include `ui/components/screen_michelangelo/screen_michelangelo.h` and, immediately after `screen_rafael(tv, ttgo, *&screenOn, ble, &led, 3)`, call `screen_michelangelo(tv, ttgo, *&screenOn, ble, &led, 4)`. Pass the existing `static LedRgb led{0, 0, 0}`. Leave Home at column 0, Leonardo at column 1, Donatelo at column 2, and Rafael at column 3

**Checkpoint**: A ordem visual Home → Leonardo → Donatelo → Rafael → Michelangelo está no firmware, e o hold da Phase 3 passa a ter tile ativo

---

## Phase 5: User Story 3 - Hold só vale no tile ativo (Priority: P2)

**Goal**: Toque fora do Michelangelo, tela apagada ou hold curto não envia comando. Leonardo, Donatelo e Rafael continuam com as cores atuais.

**Independent Test**: Segurar o toque na home, em Leonardo, em Donatelo, em Rafael ou com a tela em sleep não produz linha `Michelangelo LED`. Leonardo continua azul (`0x00, 0x00, 0xFF`), Donatelo roxo (`0x90, 0x13, 0xFE`) e Rafael vermelho (`0xFF, 0x00, 0x00`). Um toque no Michelangelo solto antes de 800 ms não envia. O tile está ativo só quando `*screenOn == true` e `lv_tileview_get_tile_act(tv)` é o tile desta tela.

### Implementation for User Story 3

- [X] T007 [US3] In `screen_michelangelo_task_loop` inside `src/ui/components/screen_michelangelo/screen_michelangelo.h`, read `lv_obj_t *active_tile = lv_tileview_get_tile_act(tv)` and gate the gesture with a single `const bool onMic = *screenOn && active_tile == tile_mic`. Send only inside `if (onMic && ttgo->getTouch(x, y))`. In the else branch set `touching=false` and `holdSent=false`, and do not change `LedRgb`. A press with `(millis() - pressStartedAt < 800)` must not call `apply_screen_led`. `holdSent` stays true until the finger lifts, so one press yields at most one command. Do not redeclare `onMic` inside the touch branch
- [X] T008 [P] [US3] Leave the existing ON colors untouched: `src/ui/components/screen_leonardo/screen_leonardo.h` (`LEO_LED` `0x00, 0x00, 0xFF`), `src/ui/components/screen_donatelo/screen_donatelo.h` (`DON_LED` `0x90, 0x13, 0xFE`), `src/ui/components/screen_rafael/screen_rafael.h` (`RAF_LED` `0xFF, 0x00, 0x00`), and `src/ui/led_rgb.h`. Do not edit those files

**Checkpoint**: O gesto do Michelangelo fica isolado no próprio tile. As telas vizinhas seguem iguais

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Validar o quickstart e confirmar que o contrato BLE não foi mexido

- [ ] T009 [P] Run the seven scenarios in `specs/002-screen-michelangelo/quickstart.md` on the `ttgo-t-watch` env (build with `pio run -e ttgo-t-watch`, serial 115200)
- [X] T010 [P] Confirm `src/services/bluetooth/WatchBleClient.h`, `src/ui/led_rgb.h`, `platformio.ini`, `src/ui/components/screen_home/screen_home.h`, and `j-prosthesis-server/src/app/structs/LedPayloadBle.hpp` have no edits from this feature

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **Foundational (Phase 2)**: Depends on T001. Blocks every user story
- **User Story 1 (Phase 3)**: Depends on T002. The send logic does not depend on US2; the on-watch check does, because the tile is registered in T006
- **User Story 2 (Phase 4)**: Depends on T004 (the task starter called by `screen_michelangelo`)
- **User Story 3 (Phase 5)**: Depends on T003 (same loop). T008 has no code dependency on T007
- **Polish (Phase 6)**: Depends on T006 and T007

### User Story Dependencies

- **User Story 1 (P1)**: Starts after Phase 2. No code dependency on US2 or US3
- **User Story 2 (P1)**: Starts after T004. Adds the tile and the column-4 registration that make US1 reachable
- **User Story 3 (P2)**: Starts after T003. Tightens the same loop; does not change the orange payload

### Within Each User Story

- US1: task loop (T003) before `xTaskCreate` (T004)
- US2: `screen_michelangelo()` (T005) before the `screen_main.h` call (T006)
- US3: guard in the Michelangelo loop (T007). The sibling-color check (T008) does not wait for T007

### Parallel Opportunities

- T008 can run beside T007 (different files)
- T009 and T010 can run together after the stories
- US1 and US2 both edit `screen_michelangelo.h`, so T003, T004, and T005 stay sequential

---

## Parallel Example: User Story 3

```bash
# After T003, these touch different files:
Task: "T007 Gate screen_michelangelo_task_loop in src/ui/components/screen_michelangelo/screen_michelangelo.h"
Task: "T008 Leave Leonardo, Donatelo, Rafael, and led_rgb.h unchanged"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1 and Phase 2
2. Complete Phase 3 (orange on / off in the task loop)
3. Complete T005 and T006 so the tile exists at column 4
4. **STOP and VALIDATE**: hold 800 ms on Michelangelo turns the prosthesis LED orange; the next hold turns it off

US1 sozinha não aparece no relógio sem o registro da Phase 4. O menor demo é Phase 3 mais T005 e T006.

### Incremental Delivery

1. Setup + context → header pronto
2. US1 → comando laranja/desligar na task, via `apply_screen_led`
3. US2 → tile `MICHELANGELO` depois do Rafael; primeiro demo no pulso
4. US3 → hold ignorado fora do tile; Leonardo, Donatelo e Rafael intactos
5. Polish → quickstart completo

### Parallel Team Strategy

Uma pessoa. O único par real é T007 com T008, e T009 com T010. O header do Michelangelo é editado em sequência.

---

## Notes

- [P] tasks = different files, no unfinished dependency
- [US1], [US2], and [US3] map to the stories in `specs/002-screen-michelangelo/spec.md`
- Do not copy the inner `const bool onDon` redeclaration from `src/ui/components/screen_donatelo/screen_donatelo.h`
- Commit after each phase if you want reviewable steps
