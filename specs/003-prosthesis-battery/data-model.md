# Data Model: Bateria da prótese no relógio

Nada é persistido. A amostra vive em `WatchBleClient` e morre no reboot. O texto da home é derivado dessa amostra. O layout do byte está em [contracts/battery-level-observation.md](./contracts/battery-level-observation.md). O texto na tela está em [contracts/prosthesis-battery-ui.md](./contracts/prosthesis-battery-ui.md).

## Amostra observada

Escopo: campos privados de `WatchBleClient`, escritos pela task `WatchBleConnect` e lidos pelo getter. Acesso sob `portMUX` só para copiar os três campos.

| Campo | Tipo lógico | Inicial | Significado |
|-------|-------------|---------|-------------|
| known | bool | false | Houve byte válido ainda não retirado e ainda dentro da janela |
| percent | uint8 | 0 | Inteiro 0–100. Só vale com `known == true` |
| seenAtMs | uint32 | 0 | `millis()` da última observação válida |

Relação: uma amostra por relógio. Não há histórico.

## Leitura para a UI

O getter devolve só isto. A UI não aplica a janela de 30 segundos por conta própria.

| Campo | Tipo lógico | Significado |
|-------|-------------|-------------|
| known | bool | `true` mostra o percentual. `false` mostra indisponível |
| percent | uint8 | Presente somente quando `known` é `true` |

Regras do getter:

1. `known == false` → indisponível.
2. `millis() - seenAtMs >= 30000` → indisponível, mesmo que `known` ainda esteja true na memória. A task pode não ter rodado; a idade manda.
3. Caso contrário → o `percent` guardado.

## Transições da amostra

Quem escreve é a task BLE, uma vez por scan.

| Observação neste scan | Efeito |
|-----------------------|--------|
| Prótese desta sessão com Service Data `0x180F` de 1 byte, valor 0–100 | `known = true`, grava o byte, `seenAtMs = millis()` |
| Prótese vista, sem esse Service Data, ou tamanho diferente de 1 | `known = false`. Retirada. Não conserva o percentual |
| Prótese vista, byte maior que 100 | Nenhum. A amostra anterior continua |
| Prótese não está nos resultados | Nenhum. O getter expira aos 30 segundos |
| Scan não rodou | Nenhum |

Identidade da "prótese desta sessão":

- Sem conexão: primeiro anúncio com nome `J1-Prosthesis` ou com o UUID de comando `12345678-1234-1234-1234-1234567890ab`.
- Com conexão: somente o endereço do peer já conectado.

O valor 0 segue a primeira linha, não a segunda. Carga esgotada publicada é `known = true` e `percent = 0`.

## Indicador da home

Escopo: objetos LVGL da task do widget. Não guardam o percentual; a cada 1 segundo copiam a leitura do getter.

| Estado | Texto |
|--------|-------|
| known, percent 0–100 | `P ` + inteiro + `%` |
| indisponível | `P --` |

Posição, fonte e o que não entra neste indicador: [contracts/prosthesis-battery-ui.md](./contracts/prosthesis-battery-ui.md).

## Bateria do relógio

Fora deste modelo, exceto como vizinho visual. Continua sendo a leitura de `ttgo->power` no widget já existente, no canto superior direito, com barra e ícone de carregamento. Não compartilha campos com a amostra da prótese.
