# Feature Specification: Bateria da prótese no relógio

**Feature Branch**: `003-prosthesis-battery`

**Created**: 2026-11-10

**Status**: Draft

**Input**: User description: "Fazer a leitura do percentual da bateria baseado no server e colocar do lado esquerdo na parte de cima do relógio."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Ver a carga da prótese na home (Priority: P1)

Na tela inicial, o usuário vê a carga da prótese no canto superior esquerdo, em percentual, distinta da carga do próprio relógio, que permanece no canto superior direito.

**Why this priority**: Sem esse indicador o usuário não sabe a carga do equipamento que está usando. É o resultado pedido.

**Independent Test**: Com a prótese por perto publicando uma carga válida, abrir a home do relógio e ler o percentual no canto superior esquerdo. O número coincide com a carga publicada pela prótese e o indicador do relógio no canto superior direito continua mostrando a bateria do relógio.

**Acceptance Scenarios**:

1. **Given** a prótese está ao alcance e publica uma carga válida entre 0 e 100, **When** a home do relógio está visível, **Then** o canto superior esquerdo mostra esse mesmo inteiro seguido de `%`.
2. **Given** a home está visível, **When** a prótese passa a publicar outro percentual, **Then** o canto superior esquerdo passa a mostrar o novo valor em até 30 segundos, sem o usuário trocar de tela nem enviar comando.
3. **Given** a home está visível e a sessão de comando com a prótese está ativa, **When** a prótese publica uma carga nova, **Then** o indicador continua acompanhando essa publicação.
4. **Given** a home está visível, **When** o usuário olha os dois cantos superiores, **Then** a esquerda é a carga da prótese e a direita continua sendo a carga do relógio, incluindo o estado de carregamento do relógio.

---

### User Story 2 - Saber quando a carga da prótese não está disponível (Priority: P1)

Se a prótese não está por perto ou deixou de publicar a carga, o canto superior esquerdo não finge um número.

**Why this priority**: Um percentual congelado ou um zero falso leva o usuário a confiar numa carga que não existe.

**Independent Test**: Anotar o percentual exibido, afastar a prótese ou fazer a prótese retirar a publicação, e confirmar que o mesmo lugar passa a mostrar indisponível em até 30 segundos.

**Acceptance Scenarios**:

1. **Given** a home mostrava um percentual, **When** a prótese sai de alcance, **Then** em até 30 segundos o canto superior esquerdo mostra indisponível e não mostra `0%` no lugar da ausência.
2. **Given** a prótese está ao alcance mas retirou a publicação da carga, **When** a home está visível, **Then** o indicador mostra indisponível.
3. **Given** a carga da prótese está indisponível, **When** a prótese volta a publicar um percentual válido, **Then** o canto superior esquerdo volta a mostrar esse percentual em até 30 segundos.

---

### User Story 3 - Usar o relógio sem a carga da prótese (Priority: P2)

Relógio, navegação, sono da tela e comandos das telas de personagem continuam funcionando quando a carga da prótese não pode ser lida.

**Why this priority**: A carga da prótese é status operacional. A falta dela não pode travar o controle nem a interface local.

**Independent Test**: Com a prótese fora de alcance, usar a home, deslizar até uma tela de personagem, deixar a tela apagar e acordar de novo. Hora, bateria do relógio, navegação e sono seguem utilizáveis, e o canto superior esquerdo da home permanece indisponível.

**Acceptance Scenarios**:

1. **Given** a carga da prótese está indisponível, **When** o usuário permanece na home, **Then** a hora e a bateria do relógio continuam atualizando.
2. **Given** a carga da prótese está indisponível, **When** o usuário desliza para Leonardo, Donatelo, Rafael ou Michelangelo, **Then** essas telas continuam no lugar e no comportamento atuais.
3. **Given** a tela apagou por inatividade, **When** o usuário acorda o relógio e volta à home, **Then** o indicador mostra o último estado ainda válido: o percentual se uma publicação fresca existir, ou indisponível se não existir.

---

### Edge Cases

- Publicação em 0: o indicador mostra `0%`. Zero publicado é carga vazia; ausência de publicação é indisponível.
- Publicação em 100: o indicador mostra `100%`.
- Valor fora de 0 a 100: o indicador ignora essa publicação e mantém o estado anterior até receber um valor válido ou até esgotar a janela de 30 segundos.
- Duas próteses ou outro anunciante com o mesmo nome: o relógio usa somente o dispositivo de comando já reconhecido como a prótese pareada desta sessão. Não mostra a carga de um terceiro.
- A publicação não muda por vários minutos: o indicador mantém o percentual enquanto a prótese continuar publicando esse mesmo valor. A janela de 30 segundos conta desde a última publicação observada, não desde a última mudança de número.
- A tela não é a home: o indicador não aparece nas telas de personagem. Ao voltar para a home, mostra o estado atual.
- A leitura da carga não pode atrasar o toque, o sono da tela nem o envio de comando de LED.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Na home, o relógio MUST mostrar a carga da prótese no canto superior esquerdo.
- **FR-002**: O valor MUST ser o inteiro de 0 a 100 publicado pela prótese, apresentado com o símbolo `%`.
- **FR-003**: O indicador MUST ser distinguível da bateria do relógio. A bateria do relógio MUST permanecer no canto superior direito da home, com o comportamento atual, inclusive carregamento.
- **FR-004**: O indicador MUST atualizar sozinho enquanto a prótese estiver ao alcance, com ou sem sessão de comando ativa. Uma publicação nova MUST aparecer em até 30 segundos.
- **FR-005**: Se nenhuma publicação válida for observada por 30 segundos, ou se a prótese retirar a publicação, o mesmo lugar MUST mostrar indisponível. MUST NOT substituir esse estado por `0%` nem por um percentual antigo.
- **FR-006**: Uma publicação válida depois do estado indisponível MUST recolocar o percentual em até 30 segundos.
- **FR-007**: O relógio MUST consumir a publicação já definida pela prótese. Esta feature MUST NOT alterar a forma como a prótese mede, valida ou publica a carga, nem inventar outro significado para o percentual.
- **FR-008**: Hora, bateria do relógio, navegação entre telas, sono e despertar da tela e comandos de LED MUST continuar disponíveis quando a carga da prótese estiver indisponível.
- **FR-009**: Atualizar o indicador MUST NOT bloquear o uso do toque nem manter a tela acordada além do tempo de sono já existente.
- **FR-010**: O indicador da prótese MUST aparecer somente na home.

### Key Entities

- **Carga publicada da prótese**: percentual inteiro de 0 a 100, ou ausência de publicação. Origem é a prótese. O relógio só observa.
- **Indicador da prótese**: o que a home mostra no canto superior esquerdo. Ou o último percentual ainda dentro da janela de 30 segundos, ou indisponível.
- **Bateria do relógio**: carga do próprio relógio, já exibida no canto superior direito da home. Fora do escopo desta feature, exceto para permanecer intacta.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Com a prótese ao alcance e publicando uma carga válida, o usuário lê esse mesmo percentual no canto superior esquerdo da home em até 30 segundos, sem abrir outra tela e sem enviar comando.
- **SC-002**: Quando a carga publicada muda, o valor exibido acompanha a mudança em até 30 segundos, inclusive com a sessão de comando ativa.
- **SC-003**: Ao sair de alcance ou ao retirar a publicação, o indicador mostra indisponível em até 30 segundos e não apresenta `0%` nem o número anterior.
- **SC-004**: Em uma observação da home, o usuário distingue a carga da prótese da carga do relógio sem trocar de tela.
- **SC-005**: Com a carga da prótese indisponível, hora, bateria do relógio, troca de telas, sono da tela e um comando de LED em uma tela de personagem continuam concluíveis.

## Assumptions

- O indicador fica na home, no canto superior esquerdo, no mesmo tipo de área em que a bateria do relógio já ocupa o canto superior direito. As telas Leonardo, Donatelo, Rafael e Michelangelo não ganham esse indicador.
- A prótese já é a fonte do percentual: publica um inteiro de 0 a 100 quando tem leitura válida e deixa de publicar quando a leitura permanece inválida. O relógio acompanha essa publicação. Não há medição nova no relógio nem mudança no server.
- "Indisponível" é um marcador estável no mesmo lugar do percentual, legível como ausência de dado, não como zero.
- A janela de frescor é 30 segundos desde a última publicação válida observada. Enquanto a prótese seguir anunciando o mesmo percentual, o número permanece.
- Não há alerta sonoro, histórico, gráfico nem indicação de carregamento da prótese. A prótese não publica se está carregando.
- O usuário desta tela é quem veste o relógio durante o uso da prótese.
