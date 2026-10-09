# CoD 1 PS4 — boot nativo experimental v0.1

Esta pasta contem um **aplicativo independente de teste**, não uma porta da campanha e não o executavel comercial CoDSP.exe.

## Funcionalidades previstas no codigo

- Homebrew nativo x86_64 PS4, icone original, TITLE_ID \`CODP40001\`.
- VideoOut com framebuffer 1920x1080, sem dependencias de bibliotecas graficas de shaders (nenhum Piglet/Shacc necessario).
- Menu controlado pelo DualShock 4 (direcional + X, bola/Options para voltar).
- Teste de cena 3D desenhado por projetor geometrico simples de CPU: grade no solo, caixas/cubos, camera movel com dois analogicos.
- Painel de controles e botao sair.
- Log persistido, quando acessivel, em \`/data/codps4/boot.log\`.

**Nao inclui:** campanha original, mapas proprietarios, musica, audio, armas, missões, IA, motor real do CoD, gameplay Single Player. O objetivo da v0.1 e apenas demonstrar inicializacao e entradas no hardware.

## Compilacao

Requer OpenOrbis SDK v0.5.4 (ou compatível), clang/lld e PkgTool.Core.
Na raiz deste subprojeto:

\`\`\`sh
export OO_PS4_TOOLCHAIN=/caminho/SDK
make
\`\`\`

Para automatizar no GitHub Actions, veja \`.github/workflows/ps4-prototype.yml\` na raiz do repositorio.
O artefato esperado se a compilacao passar e \`IV0000-CODP40001_00-COD1PS4PROTOTYPE.pkg\`.
Esse nome **nao prova** que o arquivo foi gerado ou que inicializa no seu PS4.

## Teste de console

1. Usar um PS4 com GoldHEN funcionando no firmware adequado.
2. Se o GitHub Actions publicar um arquivo \`.pkg\`, instalar pelo Package Installer.
3. Abrir \`COD1 PS4 - Prototipo\` no menu; verificar video e menu.
4. Pressionar **X** em \`INICIAR TESTE 3D\`; mover com analógico esquerdo e olhar com direito.
5. Pressionar **bola** para voltar; \`SAIR\` fecha o aplicativo.
6. Em falha, anotar codigo CE-xxxx, tela exibida, e opcionalmente ler \`/data/codps4/boot.log\` pelo FTP do console.

O sistema do console e suas bibliotecas variam, portanto **nenhuma compatibilidade e prometida sem testes reais**.

## Base tecnica e creditos

As sequencias de inicializacao do VideoOut e Pad tomam como referencia os exemplos
do SDK [OpenOrbis](https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain)
(\`samples/graphics\`, \`samples/input\`). O planejamento futuro de engine
usa como referencia [Mayo1970/ioQuake3-PS4](https://github.com/Mayo1970/ioQuake3-PS4),
sem reutilizar neste prototipo o codigo ou assets da engine Quake 3.

Nao adicionar conteudo de Call of Duty, cracks, executaveis comerciais ou SDK binarios a este repositorio publico.
