# Data Model: Screen Michelangelo

Nenhuma entidade persistida. O gesto vive na task da tela. A cor do LED é o `LedRgb` compartilhado já criado em `screen_main`. O comando reutiliza o payload BLE existente. Detalhe de bytes em [contracts/led-command.md](./contracts/led-command.md).

## Tile Michelangelo

| Campo | Valor |
|-------|--------|
| Coluna | 4 |
| Linha | 0 |
| Direção | horizontal (`LV_DIR_HOR`) |
| Rótulo | `MICHELANGELO`, centralizado |
| Ativo quando | `*screenOn == true` e `lv_tileview_get_tile_act(tv)` é o tile desta tela |

Relação: filho do tileview criado em `screen_main`. Irmão à direita do tile Rafael (coluna 3).

## Estado local do gesto

Escopo: variáveis da task, não sobrevivem a reboot. Não incluem a cor do LED.

| Campo | Tipo lógico | Inicial |
|-------|-------------|---------|
| touching | o dedo está no vidro neste ciclo | false |
| holdSent | este pressionamento já enviou comando | false |
| pressStartedAt | millis do início do toque | 0 |

## Cor compartilhada do LED

Escopo: um `LedRgb` estático em `screen_main`, ponteiro entregue a Leonardo, Donatelo, Rafael e Michelangelo. Não é persistido.

| Campo | Tipo | Inicial |
|-------|------|---------|
| r | uint8 | 0 |
| g | uint8 | 0 |
| b | uint8 | 0 |

Transições do hold no tile Michelangelo ativo:

1. Tile inativo ou sem toque → `touching = false`, `holdSent = false`. `LedRgb` não muda.
2. Toque novo no tile ativo → `touching = true`, `holdSent = false`, guarda `pressStartedAt`.
3. Toque contínuo por pelo menos 800 ms e `holdSent == false` → chama `apply_screen_led` com laranja e marca `holdSent = true`.
4. Envio ok e `LedRgb` era diferente de `(255, 165, 0)` → envia ligar laranja e grava essa cor.
5. Envio ok e `LedRgb` já era `(255, 165, 0)` → envia desligar e zera `LedRgb`.
6. Envio falha → `LedRgb` permanece.

Validação: no máximo um comando por pressionamento. Hold abaixo de 800 ms não envia. Hold noutra tela de personagem não é responsabilidade desta task.

## Comando de LED

Entidade de contrato, não de armazenamento. Campos e regras em [contracts/led-command.md](./contracts/led-command.md).

| Uso | r | g | b | brilho | efeito |
|-----|---|---|---|--------|--------|
| Ligar Michelangelo | 255 | 165 | 0 | 255 | ligado (1) |
| Desligar | 0 | 0 | 0 | 0 | desligado (0) |

Relação: produzido pela task do Michelangelo via `apply_screen_led` e entregue a `WatchBleClient::send`. O server interpreta o mesmo layout; esta feature não altera o struct de lá.
