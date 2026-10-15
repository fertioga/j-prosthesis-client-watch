# Quickstart: Screen Michelangelo

Validação no relógio. Bytes do comando: [contracts/led-command.md](./contracts/led-command.md). Gesto e ordem dos tiles: [contracts/screen-michelangelo-ui.md](./contracts/screen-michelangelo-ui.md).

## Pré-requisitos

- Env PlatformIO `ttgo-t-watch` em `j-prosthesis-client-watch`.
- LilyGo T-Watch 2020 V3 e a prótese (`J1-Prosthesis`) ligados.
- Serial em 115200.

## Build

Na raiz do client-watch:

```bash
pio run -e ttgo-t-watch
```

Gravar o firmware com o comando de upload já usado neste ambiente (`pio run -e ttgo-t-watch -t upload`).

## Cenários

### 1. Ordem dos tiles

1. Acordar a tela.
2. Deslizar da home para a direita.

Esperado: Home, Leonardo, Donatelo, Rafael, Michelangelo. O tile Michelangelo mostra o texto `MICHELANGELO` no centro.

### 2. Ligar laranja

1. Deixar o tile Michelangelo ativo, com BLE conectado (serial do relógio já mostrou conexão) e o LED apagado.
2. Segurar o toque por pelo menos 800 ms e soltar.

Esperado: LED da prótese laranja (`r=255`, `g=165`, `b=0`). Serial: `Michelangelo LED ON`. Um único pressionamento gera uma linha.

### 3. Desligar

1. Com o LED do Michelangelo ligado, segurar de novo por pelo menos 800 ms.

Esperado: LED apaga. Serial: `Michelangelo LED OFF`.

### 4. Troca a partir de outra cor

1. No Rafael, segurar até o LED ficar vermelho.
2. Ir ao Michelangelo e segurar 800 ms.

Esperado: o LED passa de vermelho para laranja. Serial: `Michelangelo LED ON`. A cor compartilhada não exige um desligamento prévio.

### 5. Toque curto

1. Tocar o Michelangelo e soltar antes de 800 ms.

Esperado: nenhum `send` e nenhuma linha `Michelangelo LED`.

### 6. Tile errado e tela apagada

1. No Leonardo, segurar o toque. Esperado: azul, serial `Leonardo LED ON` ou `OFF`. Nenhuma linha `Michelangelo`.
2. No Donatelo, segurar o toque. Esperado: roxo (`0x90, 0x13, 0xFE`), serial do Donatelo.
3. No Rafael, segurar o toque. Esperado: vermelho (`0xFF, 0x00, 0x00`), serial do Rafael.
4. Deixar a tela dormir e tocar. Esperado: wakeup existente; o Michelangelo não envia comando enquanto o tile não está ativo com a tela ligada.

### 7. BLE ausente

1. Desligar a prótese ou sair de alcance.
2. No tile Michelangelo, segurar 800 ms.

Esperado: a UI continua deslizando. Serial: `Falha ao enviar comando LED via BLE`. O próximo hold bem-sucedido, depois de reconectar, ainda liga se a cor compartilhada não ficou laranja na falha.
