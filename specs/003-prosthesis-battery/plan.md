# Implementation Plan: Bateria da prótese no relógio

**Branch**: `003-prosthesis-battery` | **Date**: 2026-11-10 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/003-prosthesis-battery/spec.md`

**Note**: This template is filled in by the `/speckit-plan` command; its definition describes the execution workflow.

## Summary

A home passa a mostrar, no canto superior esquerdo, o percentual de carga que a prótese já publica no anúncio BLE. O relógio observa o Service Data `0x180F` (um byte, 0–100) definido pelo server e não abre a characteristic `0x2A19`. Sem publicação válida, ou 30 segundos sem ver a prótese, o mesmo lugar mostra `P --`. A bateria do próprio relógio continua no canto superior direito.

A leitura fica em `WatchBleClient`. O scan de descoberta já existente alimenta a amostra enquanto não há sessão. Com a sessão de comando ativa, a mesma task faz um scan passivo curto e periódico, sem desconectar. O widget só desenha o texto.

A árvore de trabalho do git continua em `action_button`. Este comando não criou nem trocou de branch.

## Technical Context

**Language/Version**: C++ estilo Arduino (headers `.h`, `#pragma once`)

**Primary Dependencies**: PlatformIO `espressif32`, TTGO TWatch Library (LVGL), NimBLE-Arduino `^2.3.7` como cliente BLE, FreeRTOS do core ESP32

**Storage**: N/A (a última amostra vive na RAM de `WatchBleClient`; nada em arquivo)

**Testing**: Validação manual no LilyGo T-Watch 2020 V3, env `ttgo-t-watch`, serial 115200, com a prótese publicando o anúncio. Não há suíte unitária de LVGL nem de scan BLE neste repositório.

**Target Platform**: Firmware do smartwatch (`j-prosthesis-client-watch`), board `ttgo-t-watch` / `LILYGO_WATCH_2020_V3`

**Project Type**: Firmware embarcado. UI LVGL em tileview horizontal e cliente BLE NimBLE.

**Performance Goals**: Uma publicação nova aparece na home em até 30 segundos, com ou sem sessão de comando. O scan de observação com sessão dura 1 segundo e repete a cada 10 segundos. O loop LVGL permanece só com `lv_task_handler`.

**Constraints**: Não bloquear o loop LVGL. Não alterar `platformio.ini`, UUIDs, `LedPayloadBle`, o widget da bateria do relógio, as telas de personagem, o sono da tela nem o firmware do server. A janela de frescor é 30 segundos. `0%` só quando o byte publicado é 0.

**Scale/Scope**: Um widget na home e a observação do anúncio já existente no cliente BLE. Uma prótese por sessão.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Constituição `j-prosthesis-client-watch` v1.0.0.

| Gate | Resultado | Evidência |
|------|-----------|-----------|
| I. Propósito clínico | Pass | A carga da prótese é status operacional do uso diário. |
| II. Usabilidade e energia | Pass | Scan curto na task BLE já existente. Widget só atualiza label. Sono da tela não é reimplementado. Sem scan contínuo. |
| III. Contrato BLE | Pass | Consome o Service Data `0x180F` já publicado pelo server (contrato 1.0.0). Não inventa UUID nem lê `0x2A19`. |
| IV. Offline-first | Pass | Sem a prótese a home segue com hora, bateria do relógio e navegação. O indicador fica `P --`. |
| V. Arquitetura modular | Pass | Parse no service `WatchBleClient`. Widget em `ui/components/widgets/`. A home só inicia a task. |
| VI. Lúdico vs. clínico | Pass | O indicador é dado operacional, com `%` explícito, separado das telas de personagem. Ausência não vira zero. |
| VII. YAGNI | Pass | Sem lib nova, sem histórico, sem alerta, sem segunda navegação, sem mudança no server. |

**Post-design re-check**: `research.md`, `data-model.md`, os contratos e o `quickstart.md` confirmam os mesmos gates. Nenhuma violação a justificar.

## Project Structure

### Documentation (this feature)

```text
specs/003-prosthesis-battery/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── battery-level-observation.md
│   └── prosthesis-battery-ui.md
└── tasks.md             # /speckit-tasks — não criado aqui
```

### Source Code (repository root)

```text
src/services/bluetooth/WatchBleClient.h
    # observa o Service Data durante o scan que já conecta
    # e num scan passivo curto enquanto a sessão existe

src/ui/components/widgets/widget_prosthesis_battery.h   # novo
src/ui/components/screen_home/screen_home.h             # recebe o client e inicia o widget
src/ui/screen_main.h                                    # passa o WatchBleClient* para a home

src/ui/components/widgets/widget_watch_battery.h        # canto direito; não alterar
src/ui/components/screen_leonardo/                      # não alterar
src/ui/components/screen_donatelo/                      # não alterar
src/ui/components/screen_rafael/                        # não alterar
src/ui/components/screen_michelangelo/                  # não alterar
```

**Structure Decision**: A prótese continua sendo observada só por `WatchBleClient`. A home ganha um widget irmão do relógio da bateria local, alinhado ao canto oposto. O server permanece a fonte do byte; este plano não edita `j-prosthesis-server`.

## Complexity Tracking

Nenhuma violação da constituição. Esta seção fica vazia.
