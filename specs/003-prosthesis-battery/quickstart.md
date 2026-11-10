# Quickstart: Bateria da prótese no relógio

Validação no relógio. O byte observado está em [contracts/battery-level-observation.md](./contracts/battery-level-observation.md). O texto e o canto da tela estão em [contracts/prosthesis-battery-ui.md](./contracts/prosthesis-battery-ui.md).

## Pré-requisitos

- Env PlatformIO `ttgo-t-watch` em `j-prosthesis-client-watch`.
- LilyGo T-Watch 2020 V3 e a prótese (`J1-Prosthesis`) ligados, perto um do outro.
- Serial do relógio em 115200.
- A prótese precisa estar publicando o byte. Se o pino de bateria estiver fora da faixa que o server aceita, o anúncio não leva percentual e a home fica em `P --`. Para forçar 0, 42 e 100 na bancada, gravar o server com `-DBATTERY_LEVEL_INJECT` (não colocar esse flag no `platformio.ini`) e enviar no serial da prótese, uma linha por vez: `BAT 42`, `BAT 0`, `BAT 100`, `BAT OFF`.

## Build

Na raiz do client-watch:

```bash
pio run -e ttgo-t-watch
```

Gravar com o upload já usado neste ambiente (`pio run -e ttgo-t-watch -t upload`).

## Cenários

### 1. Carga no canto esquerdo

1. Fazer a prótese publicar `42`.
2. Acordar a home do relógio e esperar até 30 segundos.

Esperado: canto superior esquerdo mostra `P 42%`. Canto superior direito continua a bateria do relógio. A hora no centro segue atualizando.

### 2. Troca com a sessão aberta

1. Confirmar no serial do relógio que o BLE conectou.
2. Publicar `100` na prótese.
3. Sem sair da home, esperar até 30 segundos.
4. Segurar o toque numa tela de personagem e confirmar que o LED ainda obedece.

Esperado: a home passa a `P 100%` sem o usuário reconectar. O comando de LED continua saindo.

### 3. Zero publicado

1. Publicar `0`.

Esperado: `P 0%`, não `P --`.

### 4. Publicação retirada

1. Com `P 42%` na tela, enviar `BAT OFF` (ou afastar o cabo de medição até o server omitir o byte) e manter a prótese no alcance.

Esperado: a home vai a `P --` no ciclo seguinte de observação. Não permanece `P 42%` nem vira `P 0%`.

### 5. Fora de alcance

1. Com um percentual na tela, desligar a prótese ou sair de alcance.

Esperado: em até 30 segundos a home mostra `P --`. Hora, bateria do relógio, deslize entre os tiles e o sono da tela continuam.

### 6. Volta da carga

1. A partir de `P --`, publicar `42` de novo com a prótese perto.

Esperado: `P 42%` em até 30 segundos.

### 7. As outras telas

1. Deslizar para Leonardo, Donatelo, Rafael e Michelangelo.

Esperado: nenhuma delas mostra `P 42%` ou `P --`. A ordem dos tiles não muda. Ao voltar à home, o canto esquerdo mostra a leitura atual.
