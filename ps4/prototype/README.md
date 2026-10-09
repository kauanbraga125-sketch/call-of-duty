# CoD 1 PS4 — boot nativo experimental v0.5

Esta pasta contem um **aplicativo independente de teste**, não uma porta da campanha e não o executavel comercial CoDSP.exe.

## Funcionalidades previstas no codigo

- Homebrew nativo x86_64 PS4, icone original, TITLE_ID \`CODP40001\`.
- VideoOut com framebuffer 1920x1080, sem dependencias de bibliotecas graficas de shaders (nenhum Piglet/Shacc necessario).
- Menu controlado pelo DualShock 4 (direcional + X, bola/Options para voltar).
- Teste de cena 3D desenhado por projetor geometrico simples de CPU: grade no solo, caixas/cubos, camera movel com dois analogicos.
- Painel de controles e botao sair.
- Log persistido, quando acessivel, em \`/data/codps4/boot.log\`.

**Nao inclui:** campanha original, mapas proprietarios, musica, audio, armas, missões, IA, motor real do CoD, gameplay Single Player. O objetivo da v0.5 e apenas demonstrar inicializacao e entradas no hardware.

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


## Falha anterior: mensagem "dados corrompidos" ao iniciar (v0.1)

O pacote v0.1 foi instalado no console, mas recebeu a mensagem de dados do aplicativo corrompidos ao executar. A causa ainda **não foi confirmada**; não foi fornecido um código de erro.

Um erro conhecido no nosso pacote anterior foi identificado: o ícone gerado tinha PNG com color type 6 (RGBA, 32 bits). A documentação do OpenOrbis exige \`icon0.png\` **512×512, RGB 24 bits, sem transparência**. A v0.5 corrige o gerador para color type 2 e testa esse requisito antes da compilação.

Para testar v0.5: **remover apenas o aplicativo experimental antigo** da tela inicial do PS4; confirmar GoldHEN ativado; instalar o novo PKG e capturar o erro exato, se ainda ocorrer. Não apagar outros jogos, nem reconstruir banco de dados/formatação do console para tentar corrigir este protótipo.

Se persistir, investigar a autenticação do fSELF, metadados de PKG e estado de jailbreak com o código CE/NP.


## Diagnóstico do erro de instalação/executar

Em testes reais, as versões 0.1 e 0.2 instalaram mas mostraram "dados do aplicativo corrompidos" ao abrir, sem código CE. Isso sugere problema anterior à execução do menu, mas não comprova a causa. A v0.5 muda três campos de empacotamento baseando-se em exemplos do ioQuake3-PS4 e GoldHEN Cheats Manager:
- `create-fself --authinfo` (bloco de autenticação esperado pelos exemplos de homebrew);
- `CATEGORY=gde` e `ATTRIBUTE=32` no `param.sfo`;
- versão de aplicativo no padrão `01.03`.

Estas mudanças ainda não foram verificadas no hardware. Caso falhe novamente, parar de repetir instalações e investigar logs GoldHEN/klog, versão e se outros homebrews executam normalmente.


## Diagnostico v0.5 / CE-34878-0

O PKG v0.3 ultrapassou a mensagem de "dados corrompidos", mas o console deu CE-34878-0 ao abrir. A v0.5 adiciona pontos de verificacao antes/depois das chamadas VideoOut, framebuffer, UserService, Pad, desenho inicial e primeira troca de quadro. A funcionalidade nao e ampliada. **Nao sabemos ainda onde falha no PS4.**

Arquivo de log tentado: `/data/codps4/boot.log` (montagem do arquivo depende das permissoes do homebrew e de `main()` ter sido executado). Cada linha tem `BOOT`, `VIDEO`, `USER`, `PAD` ou `FRAME`.

Se o arquivo estiver ausente, nao prova sozinho que main nunca executou: pode haver falha de permissoes. Nesse caso solicitar log kernel/GoldHEN ou testar um hello-world minimo OpenOrbis com o mesmo empacotamento.

## v0.5: por que alteramos a inicializacao?

A v0.4 tambem apresentou CE-34878-0. Agora verificamos os erros com estagios observaveis:
- Carrega `ORBIS_SYSMODULE_INTERNAL_VIDEO_OUT` e registra resultado.
- Executa VideoOut e desenha **um primeiro quadro antes de inicializar o controle**. Deixa quadro visivel por 1,5 segundos.
- Carrega o modulo UserService; antes de usar `scePadInit`, carrega explicitamente `ORBIS_SYSMODULE_INTERNAL_PAD`, como faz o GoldHEN Cheats Manager.
- Remove os retornos normais de `main()`, que podem gerar CE-34878-0 segundo relato de testes de hardware no projeto orbis-compat.
- Na opcao `SAIR`, solicita `sceSystemServiceLoadExec("exit", NULL)` com libSceSystemService.

A presenca e a ultima linha de `/data/codps4/boot.log` agora importam mais do que um novo pacote. Se o arquivo estiver ausente, registrar se a tela chegou a aparecer. Esse codigo ainda necessita teste no hardware.
