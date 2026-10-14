# Quickstart: Screen Rafael

Validação no relógio. Bytes do comando: [contracts/led-command.md](./contracts/led-command.md). Gesto e ordem dos tiles: [contracts/screen-rafael-ui.md](./contracts/screen-rafael-ui.md).

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

Esperado: Home, Leonardo, Donatelo, Rafael. O tile Rafael mostra o texto `RAFAEL` no centro.

### 2. Ligar vermelho

1. Deixar o tile Rafael ativo, com BLE conectado (serial do relógio já mostrou conexão).
2. Segurar o toque por pelo menos 800 ms e soltar.

Esperado: LED da prótese vermelho. Serial: `Rafael LED ON`. Um único pressionamento gera uma linha.

### 3. Desligar

1. Com o LED do Rafael ligado, segurar de novo por pelo menos 800 ms.

Esperado: LED apaga. Serial: `Rafael LED OFF`.

### 4. Toque curto

1. Tocar o Rafael e soltar antes de 800 ms.

Esperado: nenhum `send` e nenhuma linha `Rafael LED`.

### 5. Tile errado e tela apagada

1. No Leonardo, segurar o toque. Esperado: azul, serial `Leonardo LED ON` ou `OFF`. Nenhuma linha `Rafael`.
2. No Donatelo, segurar o toque. Esperado: roxo (`0x90, 0x13, 0xFE`), serial do Donatelo.
3. Deixar a tela dormir e tocar. Esperado: wakeup existente; o Rafael não envia comando enquanto o tile não está ativo com a tela ligada.

### 6. BLE ausente

1. Desligar a prótese ou sair de alcance.
2. No tile Rafael, segurar 800 ms.

Esperado: a UI continua deslizando. Serial: `Falha ao enviar comando LED via BLE`. O próximo hold bem-sucedido, depois de reconectar, ainda liga (o estado local não avançou na falha).
