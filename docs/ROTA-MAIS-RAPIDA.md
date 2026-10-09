# Caminho mais curto para um Call of Duty 1 funcional no PS4

Estado em 2026-10-09: **planejamento técnico**. Não existe, neste repositório, PKG de Call of Duty jogável.

## Decisão

**Usar a camada PS4 de [Mayo1970/ioQuake3-PS4](https://github.com/Mayo1970/ioQuake3-PS4) em conjunto com a engine de compatibilidade [PrawyCoD1/iocod](https://github.com/PrawyCoD1/iocod).**

Esses projetos são derivados do ioquake3, sob GPL-2.0. Códigos de terceiros devem manter avisos de copyright, fonte correspondente e cumprimento de suas licenças. O [OpenCoD1](https://github.com/opencod1/opencod1) fica como referência de comportamento, **sem copiar seu código** até conferir uma licença explícita.

### Revisões de referência (para reprodutibilidade)

- `iocod` main: `3e2af443f82a7b777f094df339a12169731c9645`
- `ioQuake3-PS4` main: `d4c7912af73c3b0eaa57195391ad76f21e79a910`

Os projetos evoluem; não presumir que esses SHAs tenham sido compilados ou testados neste repositório.

## Por que esse caminho

- **iocod:** carrega, ou está desenvolvendo suporte para carregar, formatos específicos de CoD 1: `code/renderergl2/tr_bsp_cod1.c` (BSP 59), `tr_model_xmodel.c`, `tr_model_xanim.c`, `tr_image_dds.c`. Seu README deixa claro que multiplayer é o foco, o single-player **não está implementado** e a compatibilidade **não está completa**.
- **ioQuake3-PS4:** já funciona no hardware PS4 em variantes Quake 3/OpenArena, com `code/ps4/` (sistema, gamepad, rede, som, EGL/Piglet, memória), compilação OpenOrbis e geração PKG. Isso **não** significa que ele rode os recursos do CoD sem portar código.
- O caminho gráfico requer trabalho: o ioQuake3-PS4 usa **GLES2/Piglet com shaders pré-compilados**, e os renderizadores de CoD podem precisar de variantes/shaders adicionais. Copiar apenas as pastas não funciona.

## Marcos em ordem (cada um requer evidência)

### P0 — **Teste Windows do iocod** (primeiro bloqueio)

- Acionar o workflow manual `Teste iocod (Windows)` em **Actions**.
- Baixar o artefato gerado e testar com a instalação local legítima de CoD 1 **multiplayer 1.1** no PC; guardar o jogo original antes de qualquer modificação.
- Se possível, inicializar o menu e carregar **um mapa multiplayer** usando `+set fs_basepath`; salvar o log (sem enviar arquivos proprietários).
- Critério: compilação bem-sucedida, processo inicia com arquivos reais e evidência do que funciona. **Compilar não equivale a jogar.**

### P1 — Base nativa comprovada de PS4

- Primeiro verificar um build público/independente **ioQuake3-PS4/OpenArena** no console, sem conter recursos do Call of Duty, para confirmar gráficos, som e DualShock.
- Versão de firmware na documentação testada pode diferir da do usuário; validar no console específico.
- Critério: tela 3D e entrada no PS4, sem travamento.

### P2 — Integrar iocod e PS4 (port de engine)

- Tomar **iocod** como referência de compatibilidade e incorporar cuidadosamente a camada de plataforma `code/ps4/` do ioQuake3-PS4 em uma árvore própria GPL-2.0.
- Resolver APIs e `#ifdef __ORBIS__`, fontes e linker OpenOrbis; adaptar acesso à instalação privada do jogo fora do pacote (ex.: `/data/cod1/main/` — diretório proposto, não implementado).
- Validar memória: configurações de hunk, zone e Piglet devem ser testadas, não copiadas cegamente.
- Critério: PKG **da engine** inicializa e registra log PS4; mesmo sem carregar mapas.

### P3 — Primeiro mapa de CoD carregado

- Integrar BSP59, DDS, xmodel e animação do iocod.
- Rever formatos GL/GLSL para GLES2/Piglet; o conjunto de shaders pré-compilados para Quake 3 **não garante** suporte aos shaders novos.
- Critério: mapa CoD1 renderizado, câmera/controlador respondendo, logs de erros legíveis.

### P4 — Primeiro multiplayer jogável

- Progredir para armas, colisão, áudio, rede e interface; testar primeiro sessão LAN simples.
- Critério: movimentar, mirar, atirar e concluir uma partida com teste real no PS4.

### P5 — Campanha (depois do MP)

- O iocod não implementa o caminho standalone single-player. Não prometer campanha a partir do port multiplayer.
- Criar projeto separado após MP demonstrável.

## Experimento de maior retorno imediato

O workflow em `.github/workflows/iocod-windows.yml` **compila a engine para PC no GitHub Actions**, não gera nem simula um PKG PS4. Se o executável não conseguir iniciar/carregar dados de CoD 1 no PC, investigar esse problema *antes* de investir no port nativo.

## Licença e distribuição

Nunca adicionar ou publicar `.rar`, `.7z`, `.pk3`, `.bsp`, `.dds`, `.wav`, EXE/DLL do jogo, arquivos de crack, imagens, chaves ou outros dados comerciais. Os assets ficam numa instalação privada e devem ser legalmente obtidos. Qualquer redistribuição de fontes/derivados de ioquake3 deve acompanhar termos GPL e créditos adequados.
