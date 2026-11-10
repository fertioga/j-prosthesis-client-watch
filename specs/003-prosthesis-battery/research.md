# Research: Bateria da prótese no relógio

## 1. De onde vem o percentual

- **Decision**: Ler o Service Data do UUID `0x180F` no pacote de anúncio que a prótese já transmite. Um byte de 0 a 100. Ausência desse AD é carga desconhecida. Não ler a characteristic `0x2A19` e não alterar o server.
- **Rationale**: O contrato 1.0.0 do server (`j-prosthesis-server/specs/001-battery-level-advertising/contracts/battery-level-advertising.md`) coloca esse byte no pacote principal, junto com o nome `J1-Prosthesis`. O anúncio continua durante a sessão (`resumeAdvertising`). A characteristic Battery Level existe com `READ_ENC | NOTIFY`, mas o publish atual não a usa, e o contrato declara que ela não é o canal desta publicação. A constituição III manda o relógio consumir o schema do server.
- **Alternatives considered**: Read ou notify de `0x2A19`. Rejeitado: não é o canal publicado, exigiria sessão criptografada para um dado que já está no anúncio, e o `notifyBattery` atual não envia o inteiro. Medir a célula no relógio. Rejeitado: a barra da direita já é a bateria do T-Watch.

## 2. Como extrair o byte no NimBLE 2.x

- **Decision**: No `NimBLEAdvertisedDevice` já devolvido pelo scan, chamar `getServiceData(NimBLEUUID((uint16_t)0x180F))`. Aceitar somente `std::string` com `size() == 1` e byte `<= 100`. Não usar o template `getServiceData<uint8_t>`, porque payload ausente devolve `0`, e zero é carga esgotada válida.
- **Rationale**: NimBLE-Arduino `^2.3.7`, já em `platformio.ini`, expõe `haveServiceData()`, `getServiceData(uint8_t index)` e `getServiceData(const NimBLEUUID&)`. O UUID de 16 bits é o mesmo `0x180F` de `BleConfig.hpp` no server. O filtro por UUID evita ler Service Data de outro serviço.
- **Alternatives considered**: `getPayloadByType(0x16)` e parse manual do UUID little-endian. Rejeitado: a API por UUID já entrega só o valor. Novo parser de AD no widget. Rejeitado pela constituição V.

## 3. Ver a carga com a sessão de comando aberta

- **Decision**: A task `WatchBleConnect` continua dona do rádio. Sem sessão, o `connect()` atual (scan ativo de 5 segundos, intervalo de 2 segundos) também grava a amostra, antes de `clearResults()`. Com sessão, a mesma task faz um scan passivo de 1 segundo a cada 10 segundos, casa só o endereço do peer conectado, não chama `disconnect()`, e limpa os resultados ao terminar. `setDuplicateFilter(0)` nos dois scans, para um percentual novo não ficar preso no primeiro pacote do ciclo.
- **Rationale**: O requisito é atualizar em até 30 segundos também com a sessão ativa. Hoje a task só escaneia quando `isConnected()` é falso, então a carga congelaria depois do connect. O byte está no pacote principal, então o scan passivo enxerga a carga sem mandar scan request e sem disputar o scan response do UUID de comando. Um segundo de scan mais dez de espera cabe na janela de 30 segundos e não ocupa o loop LVGL. O server segue anunciando depois do connect.
- **Alternatives considered**: Scan ativo contínuo. Rejeitado: gasta rádio e a constituição II pede sono e sessão estáveis. Observar só antes de conectar. Rejeitado: falha o cenário com a sessão aberta. Subscribe GATT enquanto conectado. Rejeitado na decisão 1.

## 4. Qual anúncio é a prótese desta sessão

- **Decision**: Sem sessão, o mesmo critério de `connect()`: nome `J1-Prosthesis` ou anúncio do UUID `12345678-1234-1234-1234-1234567890ab`. Vale o primeiro resultado que casar, como a conexão já faz. Com sessão, só o endereço de `client->getPeerAddress()`.
- **Rationale**: A spec pede a prótese da sessão de comando, não um terceiro com o mesmo nome. Antes da sessão ainda não há endereço pareado; repetir o critério de descoberta evita uma segunda regra.
- **Alternatives considered**: Aceitar qualquer Service Data `0x180F` no ar. Rejeitado: outro aparelho com Battery Service padrão entraria na home.

## 5. Frescor, retirada e zero

- **Decision**: `WatchBleClient` guarda `known`, `percent` e `seenAtMs`. O getter aplica a política e é a única leitura da UI.
  - Byte válido (0–100): `known = true`, grava o percentual e `seenAtMs = millis()`.
  - A prótese desta observação foi vista e o Service Data `0x180F` não está presente, ou o tamanho não é 1: `known = false` na hora. A publicação foi retirada.
  - Byte maior que 100: ignora. A amostra anterior segue até um byte válido ou até a janela expirar.
  - A prótese não apareceu neste scan: não mexe na amostra. O getter devolve indisponível quando `millis() - seenAtMs >= 30000`.
  - O valor 0 é válido e aparece como `0%`.
- **Rationale**: A spec separa "saiu de alcance" (até 30 segundos) de "retirou a publicação" (indisponível assim que o anúncio da prótese chega sem o byte). O server tira o AD `0x16` de `0x180F` em `clearAdvertisedBattery()` e mantém o nome. Tratar ausência como zero mentiria a carga.
- **Alternatives considered**: Sempre esperar 30 segundos, inclusive quando o anúncio sem byte foi visto. Rejeitado: a spec pede indisponível quando a prótese está por perto e retirou a publicação. Guardar o último número até o reboot. Rejeitado: percentual congelado.

## 6. Onde a home desenha

- **Decision**: Widget novo `widget_prosthesis_battery.h`, iniciado por `screen_home`, que passa a receber `WatchBleClient*`. Label no canto superior esquerdo (`LV_ALIGN_TOP_LEFT`, deslocamento `10, 10`), fonte padrão, texto `P 42%` ou `P --`. A task do widget só lê o getter a cada 1 segundo (stack 2048, prioridade 1). `widget_watch_battery` permanece em `LV_ALIGN_TOP_RIGHT`, `-10, 10`, com barra, percentual e ícone de carga.
- **Rationale**: O relógio da bateria local já ocupa o canto direito da home. O pedido é o canto esquerdo, na mesma faixa. A fonte grande do relógio fica no centro; a fonte padrão cabe no canto sem cobrir a hora. O prefixo `P` distingue a prótese da porcentagem da direita. Parse fora do widget, como a constituição V pede.
- **Alternatives considered**: Reusar o widget da bateria do relógio com uma segunda instância. Rejeitado: ele lê `ttgo->power` e desenha carga do T-Watch. Overlay em todos os tiles. Rejeitado: a spec limita o indicador à home. Gráfico ou alerta. Rejeitado: fora do pedido.

## 7. O que não muda

- **Decision**: `platformio.ini`, UUIDs, payload de LED, telas de personagem, sono/despertar, widget da bateria do relógio e o repositório `j-prosthesis-server` ficam intactos. Sem log periódico do percentual no serial.
- **Rationale**: O anúncio já existe. Esta feature só observa e mostra. Percentual no serial, a cada ciclo, não ajuda o uso e faz barulho na bancada.
- **Alternatives considered**: Flag de build no relógio para simular a carga. Rejeitado: a injeção `BATTERY_LEVEL_INJECT` já existe no server e não deve ser copiada para o client.
