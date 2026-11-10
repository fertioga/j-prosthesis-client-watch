# Contract: Observação da carga publicada pela prótese

**Contract version**: `1.1.0` (consumo; o schema é o do server)  
**Status**: o relógio passa a ler um anúncio que o server já transmite  
**Fonte**: `j-prosthesis-server/specs/001-battery-level-advertising/contracts/battery-level-advertising.md`

Este arquivo não redefine o anúncio. Se o server mudar o layout, a versão MAJOR daquele contrato muda junto, e este consumo acompanha.

## O que o relógio aceita

No pacote principal do anúncio da prótese desta sessão:

| Campo | Regra |
|-------|--------|
| Service Data UUID | `0x180F` |
| Tamanho | 1 byte |
| Valor | inteiro 0–100 |

Exemplo já fixado pelo server para 42%: AD `04 16 0F 18 2A`.

| Situação no anúncio da prótese | Leitura do relógio |
|--------------------------------|--------------------|
| Service Data `0x180F` com 1 byte de 0 a 100 | esse inteiro |
| Sem esse Service Data | indisponível |
| Tamanho diferente de 1 | indisponível |
| Byte maior que 100 | ignora esta observação |
| Prótese ausente do scan por 30 segundos (sem sessão) | indisponível |

`0` é carga esgotada. Não é sentinela de ausência.

## Quando a observação acontece

| Estado da sessão | Canal | Período |
|------------------|-------|---------|
| Desconectada | Anúncio, no scan ativo de descoberta de `connect()` (5 segundos). Prótese = nome `J1-Prosthesis` ou UUID `12345678-1234-1234-1234-1234567890ab` | A cada tentativa de conexão (pausa de 2 segundos) |
| Conectada | Characteristic `0x2A19` do serviço `0x180F`: um read logo após o `secureConnection()` e depois notify | A cada mudança publicada pelo server |

O relógio não lê o anúncio durante a sessão: o controlador do central não entrega o anúncio do peer conectado.

Na characteristic vale a mesma regra do Service Data: 1 byte de 0 a 100 é a carga; qualquer outro tamanho é indisponível; byte maior que 100 é ignorado. Assinado e conectado, não há expiração de 30 segundos (o server só notifica quando muda). Se a characteristic faltar ou a assinatura falhar, a sessão de comando continua e a carga cai na expiração de 30 segundos.

## Fora deste contrato

- Payload de LED, passkey e bonding.
- Bateria do T-Watch (`ttgo->power`).
- Qualquer mudança em `j-prosthesis-server`.
