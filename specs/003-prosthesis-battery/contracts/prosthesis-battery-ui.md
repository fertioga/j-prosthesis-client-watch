# Contract: Indicador da carga da prótese na home

## Onde aparece

| Item | Valor |
|------|--------|
| Tile | Home, coluna 0 |
| Alinhamento | canto superior esquerdo (`LV_ALIGN_TOP_LEFT`, deslocamento `10, 10`) |
| Fonte | padrão do LVGL, não a fonte grande do relógio |
| Atualização | a cada 1 segundo, lendo o getter de `WatchBleClient` |
| Task | `ProsthesisBattery`, stack 2048, prioridade 1 |

## Texto

| Leitura | Texto exato |
|---------|-------------|
| percentual `N` de 0 a 100 | `P N%` (exemplo: `P 42%`, `P 0%`, `P 100%`) |
| indisponível | `P --` |

O prefixo `P` marca a prótese. Não há barra, ícone de carga, histórico nem alerta.

## O que permanece

| Elemento | Lugar |
|----------|--------|
| Bateria do relógio | canto superior direito da home, widget atual, inclusive ícone de carregamento |
| Hora e data | centro da home |
| Leonardo, Donatelo, Rafael, Michelangelo | sem este indicador |
| Sono e despertar da tela | iguais aos de hoje |

O indicador não é desenhado fora da home. Ao voltar para a home, o próximo ciclo de 1 segundo mostra a leitura atual do getter.
