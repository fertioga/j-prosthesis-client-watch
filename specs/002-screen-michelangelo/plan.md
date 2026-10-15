# Implementation Plan: Screen Michelangelo

**Branch**: `002-screen-michelangelo` | **Date**: 2026-10-15 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/002-screen-michelangelo/spec.md`

**Note**: This template is filled in by the `/speckit-plan` command; its definition describes the execution workflow.

## Summary

Adicionar o tile Michelangelo na coluna 4 do tileview, imediatamente depois do Rafael, com a mesma estrutura das telas de personagem. Um hold de 800 ms no tile ativo chama `apply_screen_led` com laranja `#FFA500` (`255, 165, 0`). Se a cor compartilhada `LedRgb` ainda não é essa laranja, o LED liga com brilho `255` e efeito ligado. Se já é, o LED desliga. O contrato BLE, `led_rgb.h` e o firmware do server não mudam.

A árvore de trabalho do git continua em `screen_rafael`. Este comando não criou nem trocou de branch.

## Technical Context

**Language/Version**: C++ estilo Arduino (headers `.h`; `#pragma once` no registro de telas e em `led_rgb.h`; as telas de personagem seguem o estilo atual)

**Primary Dependencies**: PlatformIO `espressif32`, TTGO TWatch Library (LVGL), NimBLE-Arduino como cliente BLE, FreeRTOS do core ESP32

**Storage**: N/A (a cor compartilhada vive em `LedRgb` na RAM de `screen_main`; o gesto vive só na task; nenhum arquivo em LittleFS)

**Testing**: Validação manual no LilyGo T-Watch 2020 V3, env `ttgo-t-watch`, serial 115200. Não há suíte unitária de tiles LVGL neste repositório.

**Target Platform**: Firmware do smartwatch (`j-prosthesis-client-watch`), board `ttgo-t-watch` / `LILYGO_WATCH_2020_V3`

**Project Type**: Firmware embarcado com UI LVGL em tileview horizontal

**Performance Goals**: A task do Michelangelo cede a CPU a cada 50 ms. O loop LVGL em `main` permanece só com `lv_task_handler`. O comando sai uma vez por pressionamento, após 800 ms.

**Constraints**: Não bloquear o loop LVGL. Não alterar `platformio.ini`, UUIDs, layout de `LedPayloadBle`, `apply_screen_led` nem o server. Hold e stack seguem as telas de personagem (800 ms, stack 2048, prioridade 1).

**Scale/Scope**: Uma tela nova e uma linha de registro. Cinco tiles no total: Home, Leonardo, Donatelo, Rafael, Michelangelo.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Constituição `j-prosthesis-client-watch` v1.0.0.

| Gate | Resultado | Evidência |
|------|-----------|-----------|
| I. Propósito clínico | Pass | Feedback de engajamento (LED na prótese) a partir de uma tela de personagem. |
| II. Usabilidade e energia | Pass | Polling em task FreeRTOS, atraso de 50 ms, gesto só com tela ligada e tile ativo. Sleep/wakeup existentes não são reimplementados. |
| III. Contrato BLE | Pass | Reuso de `apply_screen_led` → `WatchBleClient::send` e de `LedPayloadBle` (`r, g, b, brigthness, effect`). Laranja é dado, não schema novo. |
| IV. Offline-first | Pass | Tile e navegação existem sem BLE. Falha de `send` não trava a UI nem altera `LedRgb`. |
| V. Arquitetura modular | Pass | Tela em `src/ui/components/screen_michelangelo/`. Registro em `screen_main.h`. Protocolo BLE permanece no service. |
| VI. Lúdico vs. clínico | Pass | Michelangelo é feedback de engajamento. Falha de comando continua no serial, como Leonardo, Donatelo e Rafael. Esta feature não omite um status operacional que as telas irmãs já exponham. |
| VII. YAGNI | Pass | Sem lib nova, sem widget novo, sem segunda navegação, sem efeito LED novo. |

**Post-design re-check**: os artefatos de Phase 1 (modelo, contratos, quickstart) confirmam os mesmos gates. Nenhuma violação a justificar.

## Project Structure

### Documentation (this feature)

```text
specs/002-screen-michelangelo/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── screen-michelangelo-ui.md
│   └── led-command.md
└── tasks.md             # /speckit-tasks — não criado aqui
```

### Source Code (repository root)

```text
src/ui/components/screen_michelangelo/screen_michelangelo.h   # novo
src/ui/screen_main.h                                          # include + coluna 4

src/ui/components/screen_rafael/screen_rafael.h               # referência de estrutura; não alterar
src/ui/led_rgb.h                                              # cor compartilhada e toggle; não alterar
src/services/bluetooth/WatchBleClient.h                       # API de envio; não alterar
```

**Structure Decision**: Uma pasta `screen_michelangelo` no padrão `screen_<nome>/screen_<nome>.h`, registrada em `screen_main.h` na coluna 4, recebendo o mesmo `LedRgb` já passado às outras telas de personagem. Sem service, widget, asset ou mudança no server.

## Complexity Tracking

Nenhuma violação da constituição. Esta seção fica vazia.
