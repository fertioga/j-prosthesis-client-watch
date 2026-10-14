# Data Model: Screen Rafael

Nenhuma entidade persistida. O estado vive na task da tela e o comando reutiliza o payload BLE já existente. Detalhe de bytes em [contracts/led-command.md](./contracts/led-command.md).

## Tile Rafael

| Campo | Valor |
|-------|--------|
| Coluna | 3 |
| Linha | 0 |
| Direção | horizontal (`LV_DIR_HOR`) |
| Rótulo | `RAFAEL`, centralizado |
| Ativo quando | `*screenOn == true` e `lv_tileview_get_tile_act(tv)` é o tile desta tela |

Relação: filho do tileview criado em `screen_main`. Irmão à direita do tile Donatelo (coluna 2).

## Estado local do LED

Escopo: variáveis da task, não sobrevivem a reboot.

| Campo | Tipo lógico | Inicial |
|-------|-------------|---------|
| touching | o dedo está no vidro neste ciclo | false |
| holdSent | este pressionamento já enviou comando | false |
| ledOn | último envio bem-sucedido foi ligar | false |
| pressStartedAt | millis do início do toque | 0 |

Transições:

1. Tile inativo ou sem toque → `touching = false`, `holdSent = false`. `ledOn` não muda.
2. Toque novo no tile ativo → `touching = true`, `holdSent = false`, guarda `pressStartedAt`.
3. Toque contínuo por pelo menos 800 ms e `holdSent == false` → envia um comando e marca `holdSent = true`.
4. Envio ok e `ledOn` era false → envia ligar vermelho, `ledOn = true`.
5. Envio ok e `ledOn` era true → envia desligar, `ledOn = false`.
6. Envio falha → `ledOn` permanece.

Validação: no máximo um comando por pressionamento. Hold abaixo de 800 ms não envia.

## Comando de LED

Entidade de contrato, não de armazenamento. Campos e regras em [contracts/led-command.md](./contracts/led-command.md).

| Uso | r | g | b | brilho | efeito |
|-----|---|---|---|--------|--------|
| Ligar Rafael | 255 | 0 | 0 | 255 | ligado (1) |
| Desligar | 0 | 0 | 0 | 0 | desligado (0) |

Relação: produzido só pela task do Rafael e entregue a `WatchBleClient::send`. O server interpreta o mesmo layout; esta feature não altera o struct de lá.
