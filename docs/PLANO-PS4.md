# Plano técnico PS4

## Objetivo
Estudar um possível port **não oficial** de Call of Duty (2003) para PS4, sem incluir o jogo comercial no repositório.

## Situação verificável
- A existência de arquivos do PC não significa que temos código-fonte do jogo.
- OpenCoD1 é uma reconstrução **parcial de multiplayer**, não um port PS4 e não necessariamente inclui uma licença clara; não copiar sem validar permissão.
- OpenOrbis oferece ferramentas e exemplos de homebrew PS4, mas não converte jogos Windows automaticamente.
- Um executável Windows não pode ser transformado em PKG de PS4 por mera conversão de extensão.

## Fases e critérios

### Fase 0 — inventário (atual)
- Identificar edição/versão legalmente obtida, executáveis, bibliotecas e pacotes.
- Registrar relatório local de inventário sem publicar arquivos comerciais.

### Fase 1 — prova de vida PS4
- Projeto C/C++ mínimo no OpenOrbis com janela/tela, logging e entrada DualShock.
- Compilar apenas assets próprios e testar em console de desenvolvimento/homebrew adequado.
- Aceite: executável independente abre e encerra sem erro.

### Fase 2 — desenho da engine
- Verificar alternativas de código aberto e suas licenças.
- Mapear sistema de arquivos, renderização, áudio, entrada, temporização, memória e rede.
- Aceite: protótipo próprio de geometria 3D com controle e áudio.

### Fase 3 — compatibilidade
- Estudar formatos e especificidades de Call of Duty; não presumir equivalência com Quake III/ioquake3.
- Testar carregamento de assets somente a partir da cópia local legítima do proprietário, sem redistribuição.
- Separar modo história e multiplayer; compatibilidade é independente em cada módulo.

### Fase 4 — integração
- Só se existir implementação adequada e licença de reutilização: avaliar empacotamento PKG e desempenho no PS4.
- Nada aqui garante jogabilidade ou port final.

## Riscos
Dependência de código original não liberado, dependências de Windows/DirectX, arquitetura, formatos proprietários e questões de direitos autorais.
