---

description: "Task list for the prosthesis battery indicator on the watch"
---

# Tasks: Bateria da prótese no relógio

**Input**: Design documents from `/specs/003-prosthesis-battery/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: Not requested. Validation is the manual quickstart on the LilyGo T-Watch. No test tasks.

**Organization**: Tasks are grouped by user story so each story can be implemented and checked on its own.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- Firmware do relógio: `src/services/bluetooth/`, `src/ui/` na raiz de `j-prosthesis-client-watch`
- Esta feature cria um widget e altera o cliente BLE e a home. Não cria `tests/` e não edita `j-prosthesis-server`

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Criar o header do widget sem mudar stack, UUIDs nem o server

- [X] T001 Create `src/ui/components/widgets/widget_prosthesis_battery.h` with `#pragma once`, `#include "bootstrap.h"`, and `#include "services/bluetooth/WatchBleClient.h"`. Define `PROSTHESIS_BATTERY_TASK_STACK_SIZE` 2048, `PROSTHESIS_BATTERY_TASK_PRIORITY` 1, `PROSTHESIS_BATTERY_TASK_NAME` `"ProsthesisBattery"`, `PROSTHESIS_BATTERY_DELAY_MS` 1000, and `struct ProsthesisBatteryWidgetContext` with exactly `WatchBleClient *ble` and `lv_obj_t *parent`. Do not create LVGL objects yet. Do not edit `platformio.ini` or `src/ui/components/widgets/widget_watch_battery.h`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Amostra da carga e o getter que a UI vai ler. Nenhum scan novo ainda.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T002 Add the prosthesis-battery sample to `src/services/bluetooth/WatchBleClient.h`. Private fields, initial values exactly: `bool known = false`, `uint8_t percent = 0`, `uint32_t seenAtMs = 0`, and `portMUX_TYPE batteryMux = portMUX_INITIALIZER_UNLOCKED`. Public `struct ProsthesisBatteryReading { bool known; uint8_t percent; }` and `ProsthesisBatteryReading prosthesisBattery()`. Copy the three fields under `portENTER_CRITICAL` / `portEXIT_CRITICAL`, then apply these rules outside the critical section: (1) `known == false` returns `{false, 0}`; (2) `millis() - seenAtMs >= 30000` returns `{false, 0}` even if the stored `known` is still true; (3) otherwise return `{true, percent}`. Add `static constexpr uint16_t BLE_BATTERY_SERVICE_UUID = 0x180F`, `static constexpr uint32_t BLE_BATTERY_FRESH_MS = 30000`, `static constexpr uint32_t BLE_BATTERY_OBSERVE_WINDOW_MS = 1000`, and `static constexpr uint32_t BLE_BATTERY_OBSERVE_INTERVAL_MS = 10000`. Do not log the percent. Do not read characteristic `0x2A19`. Do not change `send()`

**Checkpoint**: `prosthesisBattery()` returns unavailable (`known == false`). The existing connect scan and LED write are unchanged

---

## Phase 3: User Story 1 - Ver a carga da prótese na home (Priority: P1) 🎯 MVP

**Goal**: A home mostra, no canto superior esquerdo, o inteiro 0–100 publicado no anúncio da prótese, inclusive com a sessão de comando aberta. A bateria do relógio continua no canto superior direito.

**Independent Test**: Com a prótese publicando 42, a home mostra `P 42%` em até 30 segundos. Com o serial do relógio já em "BLE conectado", publicar 100 faz a home ir a `P 100%` em até 30 segundos sem reconectar. Publicar 0 mostra `P 0%`. O canto direito continua a bateria do T-Watch.

### Implementation for User Story 1

- [X] T003 [US1] Add `void noteProsthesisAdvertisement(const NimBLEAdvertisedDevice* dev)` to `src/services/bluetooth/WatchBleClient.h`. Read the payload with `dev->getServiceData(NimBLEUUID((uint16_t)BLE_BATTERY_SERVICE_UUID))` as a `std::string`. Do not use the template `getServiceData<uint8_t>` (a missing payload returns 0, and 0 is a valid empty battery). When `size() == 1` and the byte is `<= 100`, under `batteryMux` set `known = true`, `percent` to that byte, and `seenAtMs = millis()`. A byte `> 100` must not be written in this task. Leave withdrawal (`known = false`) for T008
- [X] T004 [US1] In `connect()` of `src/services/bluetooth/WatchBleClient.h`, call `scan->setDuplicateFilter(0)` and keep `scan->setActiveScan(true)`. On the first result that already matches (`haveName()` and name `BLE_CONF_DEVICE_NAME`, or `haveServiceUUID()` and `isAdvertisingService` of `BLE_CONF_MAIN_SERVICE_UUID`), call `noteProsthesisAdvertisement(dev)` before `connectToDevice(dev)`. Still `break` after that device. Call `noteProsthesisAdvertisement` even if `connectToDevice` later returns false. Do not call it for a device that failed the name/UUID match. Keep `scan->clearResults()` after the loop
- [X] T005 [US1] Add `void observeProsthesisBattery()` to `src/services/bluetooth/WatchBleClient.h` and call it from `connectionTask` only while `isConnected()` is true. Use `NimBLEDevice::getScan()`, `setActiveScan(false)`, `setDuplicateFilter(0)`, and `getResults(BLE_BATTERY_OBSERVE_WINDOW_MS)` (1000 ms). Consider only a device whose `getAddress()` equals `client->getPeerAddress()`; call `noteProsthesisAdvertisement` for that device, then `clearResults()`. If the peer is absent, do not change `known`, `percent`, or `seenAtMs`. Do not call `disconnect()`, do not assign `ledChar = nullptr`, and do not stop advertising handling on the server. In `connectionTask`, when disconnected keep `connect()` plus `vTaskDelay(pdMS_TO_TICKS(BLE_RECONNECT_INTERVAL_MS))` (2000 ms). When connected, after `observeProsthesisBattery()` delay `vTaskDelay(pdMS_TO_TICKS(BLE_BATTERY_OBSERVE_INTERVAL_MS))` (10000 ms) instead of the 2000 ms reconnect delay
- [X] T006 [P] [US1] Implement the label task in `src/ui/components/widgets/widget_prosthesis_battery.h`. `prosthesis_battery_exec` creates one `lv_label` on `parent`, aligns it with `lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10)`, and does not set a custom font. Every `PROSTHESIS_BATTERY_DELAY_MS` (1000) call `ble->prosthesisBattery()`. When `known` is true, the visible text is `P ` + the integer + `%` (`P 0%`, `P 42%`, `P 100%`; `snprintf` pattern `P %u%%`). When `known` is false, the visible text is exactly `P --`. No bar, no charge icon, no history. End the loop with `vTaskDelay`, not `delay()`. `widget_prosthesis_battery_service_task(WatchBleClient *ble, lv_obj_t *parent)` allocates `ProsthesisBatteryWidgetContext` and calls `xTaskCreate(prosthesis_battery_exec, PROSTHESIS_BATTERY_TASK_NAME, PROSTHESIS_BATTERY_TASK_STACK_SIZE, ctx, PROSTHESIS_BATTERY_TASK_PRIORITY, nullptr)`
- [X] T007 [US1] Show the widget on the home tile only. Change `screen_home` in `src/ui/components/screen_home/screen_home.h` to `void screen_home(lv_obj_t *tv, TTGOClass *ttgo, WatchBleClient *ble, int col)`, include `ui/components/widgets/widget_prosthesis_battery.h`, and call `widget_prosthesis_battery_service_task(ble, tile1)` on the same tile as the watch battery. In `src/ui/screen_main.h` change the home call to `screen_home(tv, ttgo, ble, 0)`. Leave `widget_watch_battery_service_task(ttgo, tile1)` and `widget_clock_task(ttgo, tile1)` in place. Do not call the prosthesis widget from `screen_leonardo`, `screen_donatelo`, `screen_rafael`, or `screen_michelangelo`

**Checkpoint**: Home top-left shows `P N%` for a published 0–100, including while the LED session is connected. Top-right watch battery is unchanged

---

## Phase 4: User Story 2 - Saber quando a carga da prótese não está disponível (Priority: P1)

**Goal**: Sem o byte, ou 30 segundos sem ver a prótese, o canto esquerdo mostra `P --` e não conserva o percentual antigo nem troca ausência por `0%`.

**Independent Test**: Com `P 42%` na home, `BAT OFF` (ou o anúncio da prótese sem o Service Data `0x180F`) passa a `P --` no ciclo de observação seguinte, não a `P 0%`. Desligar a prótese ou sair de alcance passa a `P --` em até 30 segundos. Publicar 42 de novo volta a `P 42%` em até 30 segundos. Um byte maior que 100 não substitui o valor anterior antes da janela de 30 segundos.

### Implementation for User Story 2

- [X] T008 [US2] Extend `noteProsthesisAdvertisement` in `src/services/bluetooth/WatchBleClient.h` for the prosthesis already selected by T004 or T005. When that device has no Service Data for `NimBLEUUID((uint16_t)0x180F)`, or `getServiceData` size is not 1, set `known = false` under `batteryMux` immediately and do not keep showing the previous percent. When size is 1 and the byte is `> 100`, write nothing (`known`, `percent`, and `seenAtMs` stay as they were). When size is 1 and the byte is 0–100, keep the T003 path, including byte 0 (`known = true`, `percent = 0`). When the prosthesis is missing from the scan, do not call this setter and do not touch the sample; `prosthesisBattery()` already returns unavailable once `millis() - seenAtMs >= 30000`

**Checkpoint**: Retirada do byte e ausência de 30 segundos aparecem como `P --`. Zero publicado continua `P 0%`

---

## Phase 5: User Story 3 - Usar o relógio sem a carga da prótese (Priority: P2)

**Goal**: Hora, bateria do relógio, tiles, sono da tela e comando de LED seguem utilizáveis quando a carga da prótese está indisponível. O indicador não existe fora da home.

**Independent Test**: Com a prótese fora de alcance e a home em `P --`, a hora e a bateria do relógio continuam atualizando, o deslize Home → Leonardo → Donatelo → Rafael → Michelangelo não mostra `P --` nem `P N%`, a tela ainda apaga e acorda, e um hold numa tela de personagem ainda envia o LED.

### Implementation for User Story 3

- [X] T009 [US3] In `src/ui/components/widgets/widget_prosthesis_battery.h`, keep the update loop free of `NimBLEDevice`, `getScan`, `delay()`, and any screen-sleep call. In `src/ui/components/screen_home/screen_home.h`, keep `widget_watch_battery_service_task(ttgo, tile1)` and `widget_clock_task(ttgo, tile1)` on `tile1` together with the prosthesis widget. Do not start that widget from any other screen file
- [X] T010 [P] [US3] Leave these files unedited: `src/ui/components/widgets/widget_watch_battery.h`, `src/ui/components/screen_leonardo/screen_leonardo.h`, `src/ui/components/screen_donatelo/screen_donatelo.h`, `src/ui/components/screen_rafael/screen_rafael.h`, `src/ui/components/screen_michelangelo/screen_michelangelo.h`, `platformio.ini`, and the tree `j-prosthesis-server/`

**Checkpoint**: Carga indisponível não tira hora, bateria local, navegação, sono nem LED

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Validar o quickstart e confirmar que o contrato do server não foi mexido

- [ ] T011 [P] Run scenarios 1 through 7 in `specs/003-prosthesis-battery/quickstart.md` on the `ttgo-t-watch` env (build with `pio run -e ttgo-t-watch`, serial 115200). Bench percents use the server flag `-DBATTERY_LEVEL_INJECT` and the lines `BAT 42`, `BAT 0`, `BAT 100`, and `BAT OFF` only when the prosthesis is not already publishing a real cell reading. Do not add `BATTERY_LEVEL_INJECT` to either `platformio.ini`
- [X] T012 [P] Confirm `src/services/bluetooth/WatchBleClient.h` does not print the percent on each observation, `send()` is unchanged, and `j-prosthesis-server/src/app/service/bluetooth/ble/BleService.cpp` has no edits from this feature

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **Foundational (Phase 2)**: Depends on nothing in T001 (different file) but blocks every user story. Finish T002 before T003
- **User Story 1 (Phase 3)**: Depends on T002. T006 also depends on T001. T007 depends on T006
- **User Story 2 (Phase 4)**: Depends on T003 (same `noteProsthesisAdvertisement`). The out-of-range window is already in T002
- **User Story 3 (Phase 5)**: Depends on T006 and T007 (same widget and `screen_home.h`). T010 has no code dependency
- **Polish (Phase 6)**: Depends on T008, T009, and T010

### User Story Dependencies

- **User Story 1 (P1)**: Starts after T002. No code dependency on US2 or US3. This is the MVP
- **User Story 2 (P1)**: Starts after T003. Adds withdrawal and the `> 100` ignore on the writer US1 already calls. Uses the label from T006 to become visible
- **User Story 3 (P2)**: Starts after T007. Does not change the byte rules

### Within Each User Story

- US1: valid-byte writer (T003) before the disconnected scan (T004) and the connected scan (T005). T004 and T005 both edit `WatchBleClient.h`, so they stay sequential. The widget (T006) is another file and can proceed once T001 and T002 exist. Home wiring (T007) waits for T006
- US2: one change to `noteProsthesisAdvertisement` (T008)
- US3: home/widget guard (T009) after T007. The untouched-files check (T010) does not wait for T009

### Parallel Opportunities

- T006 can run beside T003, T004, and T005 (widget file versus `WatchBleClient.h`) after T001 and T002
- T010 can run beside T009
- T011 and T012 can run together after the stories
- T003, T004, T005, and T008 all edit `WatchBleClient.h`, so they stay sequential

---

## Parallel Example: User Story 1

```bash
# After T001 and T002, different files:
Task: "T003 Record a valid 0-100 Service Data byte in src/services/bluetooth/WatchBleClient.h"
Task: "T006 Draw P N% / P -- in src/ui/components/widgets/widget_prosthesis_battery.h"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1 and Phase 2
2. Complete Phase 3 (parse the published byte, observe it while connected, show `P N%` on the home top-left)
3. **STOP and VALIDATE**: quickstart scenarios 1, 2, and 3 (`P 42%`, `P 100%` while connected, `P 0%`)

### Incremental Delivery

1. Setup + sample getter → `prosthesisBattery()` returns unavailable
2. US1 → home shows the published percent, including during the LED session
3. US2 → missing byte and 30 seconds out of range become `P --` without turning into `0%`
4. US3 → clock, watch battery, tiles, sleep, and LED still work while the prosthesis percent is unavailable
5. Polish → quickstart scenarios 1 through 7

### Parallel Team Strategy

Uma pessoa. O único par de implementação é T006 junto com T003–T005. O cliente BLE é editado em sequência: T002, T003, T004, T005, T008.

---

## Notes

- [P] tasks = different files, no unfinished dependency
- [US1], [US2], and [US3] map to the stories in `specs/003-prosthesis-battery/spec.md`
- The advertisement layout is owned by `j-prosthesis-server/specs/001-battery-level-advertising/contracts/battery-level-advertising.md`. Do not invent a second schema
- Commit after each phase if you want reviewable steps
