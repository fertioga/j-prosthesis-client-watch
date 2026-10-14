# Contract: Tela Rafael

Contrato de UI do firmware do relógio. Não é API de rede.

## Registro

`screen_main` monta o tileview horizontal e chama as telas nesta ordem:

| Coluna | Entrada | Dependências |
|--------|---------|--------------|
| 0 | `screen_home` | `tv`, `ttgo` |
| 1 | `screen_leonardo` | `tv`, `ttgo`, `screenOn`, `ble` |
| 2 | `screen_donatelo` | `tv`, `ttgo`, `screenOn`, `ble` |
| 3 | `screen_rafael` | `tv`, `ttgo`, `screenOn`, `ble` |

Assinatura esperada:

```text
void screen_rafael(lv_obj_t *tv, TTGOClass *ttgo, bool *screenOn, WatchBleClient *ble, int col);
```

`col` recebido na chamada é 3.

## Composição do tile

- Tile em `(col, 0)` com direção horizontal.
- Um label filho, texto `RAFAEL`, centralizado no tile.
- Sem imagem e sem widget de relógio ou bateria.

## Gesto

| Condição | Resultado |
|----------|-----------|
| Tela apagada ou tile ativo diferente do Rafael | Nenhum `send` |
| Toque solto antes de 800 ms | Nenhum `send` |
| Toque contínuo ≥ 800 ms, LED local desligado, BLE ok | Comando ligar vermelho; serial `Rafael LED ON` |
| Toque contínuo ≥ 800 ms, LED local ligado, BLE ok | Comando desligar; serial `Rafael LED OFF` |
| `send` retorna false | Serial `Falha ao enviar comando LED via BLE`; estado local intacto |

O dedo que permanece depois do envio não gera segundo comando. Um novo comando exige soltar e segurar de novo.

## Fora deste contrato

- Home, Leonardo, Donatelo, sleep e wakeup.
- Indicador visual de conexão BLE.
- Asset gráfico do Rafael.
