# Contract: Tela Michelangelo

Contrato de UI do firmware do relógio. Não é API de rede.

## Registro

`screen_main` monta o tileview horizontal e chama as telas nesta ordem:

| Coluna | Entrada | Dependências |
|--------|---------|--------------|
| 0 | `screen_home` | `tv`, `ttgo` |
| 1 | `screen_leonardo` | `tv`, `ttgo`, `screenOn`, `ble`, `led` |
| 2 | `screen_donatelo` | `tv`, `ttgo`, `screenOn`, `ble`, `led` |
| 3 | `screen_rafael` | `tv`, `ttgo`, `screenOn`, `ble`, `led` |
| 4 | `screen_michelangelo` | `tv`, `ttgo`, `screenOn`, `ble`, `led` |

Assinatura esperada:

```text
void screen_michelangelo(lv_obj_t *tv, TTGOClass *ttgo, bool *screenOn, WatchBleClient *ble, LedRgb *led, int col);
```

`col` recebido na chamada é 4. `led` é o mesmo `LedRgb` já passado às outras telas de personagem.

## Composição do tile

- Tile em `(col, 0)` com direção horizontal.
- Um label filho, texto `MICHELANGELO`, centralizado no tile.
- Sem imagem e sem widget de relógio ou bateria.

## Gesto

| Condição | Resultado |
|----------|-----------|
| Tela apagada ou tile ativo diferente do Michelangelo | Nenhum `send` |
| Toque solto antes de 800 ms | Nenhum `send` |
| Toque contínuo ≥ 800 ms, cor compartilhada diferente de laranja, BLE ok | Comando ligar laranja; serial `Michelangelo LED ON` |
| Toque contínuo ≥ 800 ms, cor compartilhada já laranja, BLE ok | Comando desligar; serial `Michelangelo LED OFF` |
| `send` retorna false | Serial `Falha ao enviar comando LED via BLE`; `LedRgb` intacto |

O dedo que permanece depois do envio não gera segundo comando. Um novo comando exige soltar e segurar de novo.

## Fora deste contrato

- Home, Leonardo, Donatelo, Rafael, sleep e wakeup.
- Indicador visual de conexão BLE.
- Asset gráfico do Michelangelo.
- Mudança em `apply_screen_led`.
