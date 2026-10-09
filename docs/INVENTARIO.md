# Inventário técnico — Call of Duty (2003) para estudo de PS4

Data: 2026-10-09

## Fonte e método

Análise **somente do índice de arquivos** de um arquivo 7z de múltiplos volumes, fornecido de forma privada pelo proprietário da instalação. Os quatro volumes foram encontrados e reagrupados para leitura local do índice. Não foram incluídos arquivos comerciais, conteúdo de jogo, executáveis, chaves, texturas ou áudio no repositório.

**Nota de integridade:** a leitura completa do índice teve sucesso, mas não constitui teste CRC integral de todos os conteúdos.

## Resumo verificado

- 4 volumes: 190 MiB + 190 MiB + 190 MiB + aproximadamente 62 MiB
- 14.901 entradas (incluindo diretórios)
- Cerca de 2,05 GiB em tamanhos descompactados declarados pelo índice
- 14.510 arquivos contidos no diretório `Main/`
- 45 mapas `.bsp`, sendo 12 em `Main/maps/MP/`
- 799 scripts `.gsc`
- 2.463 texturas `.dds`
- 942 imagens `.tga`
- 2.789 sons `.wav`
- 473 arquivos `.map`, principalmente modelos de colisão
- 70 interfaces `.menu` e 34 `.shader`
- Executáveis identificados: `CoDSP.exe`, `CoDMP.exe`
- Bibliotecas identificadas: `gamex86.dll`, `cgamex86.dll`, `uix86.dll`, além de variantes multiplayer sob `Main/`
- O índice não contém pacotes `.pk3`; parece ser uma instalação com dados já extraídos

Diretórios representativos: `Main/maps`, `Main/xanim`, `Main/xmodel`, `Main/xmodelparts`, `Main/xmodelsurfs`, `Main/skins`, `Main/textures`, `Main/weapons`, `Main/sound`, `Main/scripts`.

## Implicações

1. Os mapas `.bsp` são um alvo interessante para **pesquisa de cabeçalhos e formatos**.
2. Os recursos gráficos e de áudio são dados do jogo original, **não código-fonte da engine**.
3. O número de arquivos não informa se todos os recursos são compatíveis com um port; o código Windows `EXE/DLL` não roda diretamente no PS4.
4. Não distribuir os recursos comerciais, instaladores, arquivos de crack, executáveis ou chaves. O projeto GitHub publica apenas código independente e documentação técnica.

## Próxima etapa proposta

- Criar um verificador somente-leitura para cabeçalhos de `.bsp`, `.dds` e arquivos de modelos, executado pelo próprio proprietário da instalação no PC.
- Implementar um protótipo independente PS4 via OpenOrbis, inicialmente sem qualquer recurso comercial.
- Avaliar limites de memória, shaders, entradas de controle e caminhos de renderização.
- Considerar a reconstrução aberta [OpenCoD1](https://github.com/opencod1/opencod1) **somente após** validação técnica e das permissões de reutilização.

**Situação atual: inventário concluído; port para PS4 e PKG jogável ainda não existem.**
