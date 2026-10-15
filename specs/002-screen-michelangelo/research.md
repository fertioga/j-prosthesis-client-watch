# Research: Screen Michelangelo

## 1. Onde a tela entra no tileview

- **Decision**: Registrar `screen_michelangelo` na coluna 4, depois de `screen_rafael` na coluna 3. Ordem final: Home 0, Leonardo 1, Donatelo 2, Rafael 3, Michelangelo 4.
- **Rationale**: O pedido é "mais uma screen". A sequência atual termina no Rafael. Acrescentar à direita preserva Home, Leonardo, Donatelo e Rafael.
- **Alternatives considered**: Inserir Michelangelo entre tiles existentes exigiria renumerar colunas. Rejeitado: o pedido não pede reordenação.

## 2. Estrutura da tela

- **Decision**: Espelhar a casca do Rafael: `lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR)`, label `MICHELANGELO` centralizado, contexto tipado com `LedRgb *`, task FreeRTOS criada na entrada da tela. A máquina de toque segue o hold já usado nas telas de personagem: 800 ms, um envio por pressionamento, poll de 50 ms, stack 2048, prioridade 1. O envio passa por `apply_screen_led`.
- **Rationale**: Rafael é o vizinho e já usa rótulo, cor compartilhada e `apply_screen_led`. Não há bitmap do Michelangelo.
- **Alternatives considered**: Copiar a imagem do Leonardo. Rejeitado: não há asset `michelangelo` e o pedido não pede imagem. Copiar a declaração interna duplicada de `onDon` em `screen_donatelo.h`. Rejeitado: é um acidente local; o guard de tile ativo fica uma vez, como em `screen_rafael.h`. Toggle privado `ledOn` por tela. Rejeitado: as telas atuais compartilham um único `LedRgb` em `screen_main`.

## 3. Cor laranja no payload existente

- **Decision**: Ligar com `apply_screen_led(ble, led, 0xFF, 0xA5, 0x00)`, que chama `send(0xFF, 0xA5, 0x00, 0xFF, LED_EFFECT_ON)` quando a cor compartilhada é outra. Desligar, quando a cor compartilhada já é essa, chama `send(0, 0, 0, 0, LED_EFFECT_OFF)` e zera `LedRgb`.
- **Rationale**: `#FFA500` é o laranja nomeado padrão (vermelho 255, verde 165, azul 0). `LedPayloadBle` no client e no server é `r, g, b, brigthness, effect` (o campo de brilho mantém o typo `brigthness` dos dois lados). Leonardo liga em azul `0x00, 0x00, 0xFF`, Donatelo em roxo `0x90, 0x13, 0xFE`, Rafael em vermelho `0xFF, 0x00, 0x00`. O server aplica `effect == 1` com os canais recebidos; laranja cabe nesse schema. O brilho máximo ao ligar é o de `apply_screen_led`.
- **Alternatives considered**: Novo efeito ou novo UUID. Rejeitado pela constituição III e VII. Laranja "de LED" com verde mais baixo (por exemplo `255, 100, 0`) para compensar o verde forte do NeoPixel. Rejeitado neste plano: o pedido diz laranja, e `#FFA500` é o valor nomeado sem calibração de hardware que ninguém pediu. Se no relógio o verde 165 parecer amarelo, o ajuste fica para uma calibração explícita, não para esta feature.

## 4. O que não muda

- **Decision**: `WatchBleClient`, `led_rgb.h`, UUIDs, `platformio.ini`, home, Leonardo, Donatelo, Rafael, sleep e o firmware `j-prosthesis-server` ficam intactos.
- **Rationale**: O client já consome o contrato. Laranja cabe no schema atual (`LED_EFFECT_ON = 1`, `LED_EFFECT_OFF = 0`). A regra de ligar/desligar já está em `apply_screen_led`.
- **Alternatives considered**: Ajustar o server para uma cor nomeada "michelangelo". Rejeitado: a cor é escolha do payload, não um efeito novo.

## 5. Falha de BLE e cor compartilhada

- **Decision**: `apply_screen_led` só grava `LedRgb` quando `send` retorna verdadeiro. Falha imprime `Falha ao enviar comando LED via BLE`. Sucesso imprime `Michelangelo LED ON` quando a cor resultante não é zero, ou `Michelangelo LED OFF` quando é.
- **Rationale**: É o comportamento de Rafael, Donatelo e Leonardo. A UI local continua navegável sem conexão (constituição IV).
- **Alternatives considered**: Indicador visual de conexão neste tile. Fora do pedido e ausente nas telas irmãs.
