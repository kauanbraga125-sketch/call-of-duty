# CoD 1 para PS4 — experimento 0.6: visualizar geometria de um mapa original

O aplicativo 0.5 foi **testado no PS4**: iniciou, exibiu menus, permitiu controlar a câmera 3D e voltar ao menu. Na v0.6, mantemos o boot e adicionamos um leitor simples de malha de mapa, **sem copiar arquivos comerciais para o GitHub**.

## O que esta versão pode fazer

- Se existir \`/data/codps4/maps/preview.c4m\`, carrega até 1200 triângulos convertidos de um mapa **original da sua própria instalação**.
- O primeiro botão do menu vira **ABRIR MAPA ORIGINAL**; apresenta a geometria de parte do mapa em linhas coloridas e a câmera se move com os analógicos.
- Sem o arquivo de mapa, oferece o teste 3D da v0.5.
- Se houver erro no arquivo, escreve no log \`/data/codps4/boot.log\` e volta ao cenário de teste.

**Limitações:** não há texturas, materiais de fato, iluminação, objetos, colisões, IA, scripts, salvamento ou Single Player. Esta é uma primeira prova de leitura da geometria de **mapas BSP originais IBSP 59**, não um jogo completo. Como o renderizador é por CPU, limitamos o número de triângulos.

## Como extrair a geometria no PC (Windows)

1. Instale ou tenha instalada sua cópia legítima do Call of Duty 1. Os arquivos normalmente estão em \`C:\Program Files (x86)\Call Of Duty\main\`.
2. Baixe o arquivo \`ps4/map_preview/convert_map.py\` deste repositório.
3. Abra o Prompt de Comando (cmd) e veja quais mapas foram encontrados:

\`\`\`bat
py convert_map.py --game-dir "C:\Program Files (x86)\Call Of Duty\main" --list
\`\`\`

4. Escolha o nome de um mapa listado (idealmente uma missão sem prefixo \`mp_\`; exemplos variam por edição) e gere o arquivo:

\`\`\`bat
py convert_map.py --game-dir "C:\Program Files (x86)\Call Of Duty\main" --map training --output preview.c4m
\`\`\`

Se \`training\` não estiver na lista, substitua pelo nome que foi mostrado em \`--list\`. Sem \`--map\` o conversor seleciona automaticamente um mapa não-multiplayer, caso exista.

5. Com o **GoldHEN FTP ativado**, conecte por um cliente FTP no PC (IP do PS4; porta configurada pelo GoldHEN, frequentemente 2121), crie \`/data/codps4/maps\` e copie **somente** \`preview.c4m\` para \`/data/codps4/maps/preview.c4m\`. **Não é necessário enviar nenhum PK3 para o console**, e não copie os arquivos do jogo para um repositório público.
6. Instale o novo PKG v0.6, abra e use **ABRIR MAPA ORIGINAL**. O pacote pode ser instalado antes ou depois de enviar o arquivo por FTP.

### Se o conversor não encontra mapas

Confira se a pasta informada realmente contém arquivos \`.pk3\` e se eles têm mapas \`maps/*.bsp\`. Em algumas instalações a versão é diferente da v1.1 e o mapa pode estar em outro diretório. Envie apenas a saída de \`--list\` (nomes de mapas), não os dados comerciais.

### Referências técnicas

- [vcod — CoD1 IBSP 59](https://github.com/janost/vcod/blob/main/docs/research/bsp-ibsp59-format.md) documenta o diretório de 33 lumps, drawverts de 44 bytes, índices u16, soups de 16 bytes e modelo 0 como geometria do mundo. **Nosso conversor foi escrito de forma independente**, sem copiar código GPL de terceiros.
- [OpenOrbis PS4 Toolchain](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain) — build nativo PS4.
- O formato local \`C4M1\` é diagnóstico e nosso: cabeçalho \`<4sIffff\` seguido de N triângulos \`<9fI\`. Coordenadas centradas num ponto de spawn para evitar perda de precisão.

Próximas etapas: testar **um mapa real no hardware**, trabalhar na culling/otimização de malha, materiais, texturas, colisões e depois nos scripts da campanha. A existência de mapa visível ainda **não** prova viabilidade de todos os scripts de Single Player.
