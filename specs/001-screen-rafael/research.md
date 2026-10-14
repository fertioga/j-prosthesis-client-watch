# Research: Screen Rafael

## 1. Onde a tela entra no tileview

- **Decision**: Registrar `screen_rafael` na coluna 3, depois de `screen_donatelo` na coluna 2. Ordem final: Home 0, Leonardo 1, Donatelo 2, Rafael 3.
- **Rationale**: O pedido é explícito: Rafael logo depois do Donatelo, no tileview horizontal já usado (`LV_DIR_HOR`, snap center).
- **Alternatives considered**: Inserir Rafael entre Leonardo e Donatelo exigiria renumerar a coluna do Donatelo. Rejeitado.

## 2. Estrutura da tela

- **Decision**: Espelhar a casca do Donatelo: `lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR)`, label `RAFAEL` centralizado, contexto tipado, task FreeRTOS criada na entrada da tela. A máquina de toque segue o hold já usado em Leonardo e Donatelo: 800 ms, um envio por pressionamento, poll de 50 ms, stack 2048, prioridade 1.
- **Rationale**: "Mesma estrutura" e ausência de bitmap do Rafael. Donatelo é o vizinho e já usa label em vez de imagem.
- **Alternatives considered**: Copiar a imagem do Leonardo. Rejeitado: não há asset `rafael` e o pedido não pede imagem. Copiar também a declaração interna duplicada de `onDon` em `screen_donatelo.h`. Rejeitado: é um acidente local; o guard de tile ativo fica uma vez, como em `screen_leonardo.h`.

## 3. Cor vermelha no payload existente

- **Decision**: Ligar com `send(0xFF, 0x00, 0x00, 0xFF, LED_EFFECT_ON)`. Desligar com `send(0, 0, 0, 0, LED_EFFECT_OFF)`.
- **Rationale**: `LedPayloadBle` no client e no server é `r, g, b, brigthness, effect` (o campo de brilho mantém o typo `brigthness` dos dois lados). Leonardo já liga em azul `0x00, 0x00, 0xFF`. Donatelo liga em roxo `0x90, 0x13, 0xFE`. Vermelho puro é o canal R no máximo e G/B em zero, com o mesmo brilho máximo e o mesmo efeito ligado.
- **Alternatives considered**: Novo efeito ou novo UUID. Rejeitado pela constituição III e VII. Vermelho "escuro" ou com brilho menor. Rejeitado: as outras telas usam brilho `0xFF` ao ligar.

## 4. O que não muda

- **Decision**: `WatchBleClient`, UUIDs, `platformio.ini`, home, Leonardo, Donatelo, sleep e o firmware `j-prosthesis-server` ficam intactos.
- **Rationale**: O client já consome o contrato. Vermelho cabe no schema atual (`LED_EFFECT_ON = 1`, `LED_EFFECT_OFF = 0`).
- **Alternatives considered**: Ajustar o server para uma cor nomeada "rafael". Rejeitado: a cor é escolha do payload, não um efeito novo.

## 5. Falha de BLE e estado local

- **Decision**: Só inverter `ledOn` quando `send` retorna verdadeiro. Falha imprime `Falha ao enviar comando LED via BLE`. Sucesso imprime `Rafael LED ON` ou `Rafael LED OFF`.
- **Rationale**: É o comportamento de Leonardo e Donatelo. A UI local continua navegável sem conexão (constituição IV).
- **Alternatives considered**: Indicador visual de conexão neste tile. Fora do pedido e ausente nas telas irmãs.
