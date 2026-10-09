# Call of Duty (2003) — estudo de port para PS4

> **Estado atual:** preparação técnica; ainda **não** é um port jogável, nem produz um PKG de Call of Duty.

Este repositório organiza um projeto **não oficial** de pesquisa de compatibilidade do **Call of Duty original de 2003 (PC)** com homebrew de PS4. A etapa inicial é identificar os arquivos locais do jogo e mapear os componentes técnicos necessários. O código aqui disponibilizado é independente, criado para inventariar arquivos; não contém código, modelos, mapas, texturas, áudio, executáveis ou outras propriedades intelectuais do Call of Duty.

## Comece aqui

1. Tenha uma instalação legítima do jogo original no seu PC. **Não publique os arquivos dela neste repositório.**
2. Se você tem um `.rar`, extraia **localmente** com 7-Zip/WinRAR. O utilitário deste projeto não descompacta RAR.
3. Com Python 3.10+, gere um relatório dos seus arquivos sem enviá-los para a internet:

   ```bash
   python tools/inventory.py "C:/Jogos/Call of Duty" --output inventario.json
   ```

   Para reconhecer somente o formato de um RAR:

   ```bash
   python tools/inventory.py "C:/Downloads/call_of_duty.rar"
   ```

4. Rode os testes: `python -m unittest discover -s tests -v`.

O relatório enumera extensões, número de arquivos e nomes de pacotes `.pk3` com suas contagens. **Não** copia nem extrai dados proprietários. Trate inventários de arquivos privados com cuidado e não inclua caminhos pessoais se for compartilhar o relatório.

## Referências de pesquisa

- [OpenCoD1](https://github.com/opencod1/opencod1): reconstrução em andamento do multiplayer de Call of Duty (2003), atualmente orientada a Windows/Visual Studio. **Não** foi incorporada ao repositório; verifique licenças e permissões antes de reutilizar código.
- [OpenOrbis PS4 Toolchain](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain): ferramentas homebrew e exemplos para PS4, incluindo SDL2.
- [ioquake3](https://github.com/ioquake/ioq3): estudo de arquitetura de engine derivada de id Tech 3; **não é** uma implementação de Call of Duty.

## Etapas do projeto

1. Identificar a edição e os arquivos do PC, sem distribuição pública do jogo.
2. Avaliar o código-fonte realmente reutilizável e suas licenças.
3. Criar um protótipo independente PS4 que inicialize, apresente vídeo, processe o controle e carregue **assets próprios de teste**.
4. Somente depois, avaliar compatibilidade com formatos do jogo e uma eventual camada de engine.
5. Testar no PS4 desbloqueado, registrar logs e empacotar um **protótipo independente**, se viável.

Leia [docs/PLANO-PS4.md](docs/PLANO-PS4.md) para limitações, riscos e critérios verificáveis.

## Aviso

Projeto comunitário não oficial, sem vínculo com Activision ou Infinity Ward. **Call of Duty** é marca de seus respectivos titulares. O repositório não hospeda nem fornece arquivos do jogo e não promete port completo ou PKG jogável. Não compartilhe mídia, executáveis, chaves ou outros materiais protegidos sem autorização.
