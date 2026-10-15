# Contract: Comando de LED laranja

O client já fala este contrato. Esta feature só fixa os valores do Michelangelo. Fonte do layout: `LedPayloadBle` em `src/services/bluetooth/WatchBleClient.h` e o struct homônimo em `j-prosthesis-server` `src/app/structs/LedPayloadBle.hpp`. A escolha ligar/desligar está em `src/ui/led_rgb.h` (`apply_screen_led`).

## Transporte

| Campo | Valor |
|-------|--------|
| Device | `J1-Prosthesis` |
| Service | `12345678-1234-1234-1234-1234567890ab` |
| Characteristic | `12345678-1234-1234-1234-1234567890ac` |
| Operação | write com response, 5 bytes packed |

## Layout (little-endian, sem padding)

| Offset | Campo no struct | Tipo |
|--------|-----------------|------|
| 0 | `r` | uint8 |
| 1 | `g` | uint8 |
| 2 | `b` | uint8 |
| 3 | `brigthness` | uint8 |
| 4 | `effect` | uint8 |

O nome `brigthness` é o do código atual nos dois repositórios.

## Efeitos usados

| Nome | Valor |
|------|-------|
| `LED_EFFECT_OFF` | 0 |
| `LED_EFFECT_ON` | 1 |

## Valores do Michelangelo

Laranja nomeada `#FFA500`. Ligar, quando `LedRgb` ainda não é essa cor:

| r | g | b | brilho | effect |
|---|---|---|--------|--------|
| `0xFF` | `0xA5` | `0x00` | `0xFF` | `0x01` |

Desligar, quando `LedRgb` já é `0xFF, 0xA5, 0x00`:

| r | g | b | brilho | effect |
|---|---|---|--------|--------|
| `0x00` | `0x00` | `0x00` | `0x00` | `0x00` |

Chamada correspondente na API existente: `apply_screen_led(ble, led, 0xFF, 0xA5, 0x00)`, que delega a `WatchBleClient::send`.

## Cores que esta feature não altera

| Tela | Ligar (r, g, b, brilho, effect) |
|------|----------------------------------|
| Leonardo | `0x00, 0x00, 0xFF, 0xFF, ON` |
| Donatelo | `0x90, 0x13, 0xFE, 0xFF, ON` |
| Rafael | `0xFF, 0x00, 0x00, 0xFF, ON` |

Desligar continua zerado, efeito off, pela mesma função `apply_screen_led`.

## Compatibilidade

Sem bump de contrato. O server já aplica `effect == 1` como acender com os canais recebidos. Laranja não exige characteristic, enum ou fila novos.
