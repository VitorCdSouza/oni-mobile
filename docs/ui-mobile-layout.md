# Layout de toque da UI (1600x720)

Implementação do redesenho pedido em `docs/ui-mobile-brief.md`. O código fica em `tools/layout`
(`OniMobile.Layout.dll`) e só liga com `OniMobile.Platform.MobileUI`: no Android sempre, no PC com
`ONI_MOBILE_UI=1`.

## Princípio do design

Falta altura (720 linhas) e sobra largura (20:9). Tudo vai para colunas de 72 px nas bordas; no
centro fica só o mapa. Os painéis grandes abrem como folhas laterais, nunca embaixo.

- Referência: 1600x720. 1 px = 0,0965 mm, 1 mm = 10,4 px. Em 1333x600 os px escalam por 0,833 e
  os mm ficam iguais (o código mede tudo em px de referência, `Ui.U`).
- Alvo padrão de 72 px (6,9 mm, ~48 dp); Construir 96 px, Sair e Girar 92 px (~9 mm). O brief
  pedia 9 mm, que são 93 px e não 53: com 93 px em tudo cada coluna lateral teria só 6 botões.
- Margem de 56 px à esquerda (furo da câmera, 48 px livres) e de 8 px nas outras bordas.
- Os botões novos copiam o botão de categoria de construção do jogo (fundo, sombra, fonte do
  rótulo), lidos em tempo de execução.
- Fonte de corpo ~20 px (1,9 mm), nunca abaixo de 18 px: é a escala de 175% do jogo a 720 linhas.

## Elementos

Posição x, y do canto superior esquerdo, em px de referência.

| Código | Elemento | x, y | L x A | Original do jogo |
| --- | --- | --- | --- | --- |
| H1 | Relógio do ciclo | 56, 8 | 88x88 | `DateTime` (AsteroidClockPanel) |
| H2 | Velocidade: pausa, 1x, 2x, 3x | 152, 16 | ~210x52 | `SpeedControlScreen` |
| H3 | Alerta vermelho | após H2 | 52x52 | `RedAlertButton` do `TopLeftControlScreen` |
| — | Sandbox, só o ícone | após H3 | 52x52 | `SandboxToggle` do `TopLeftControlScreen` |
| — | Medidores (duplicantes, estresse, doença, rações) | 152, 74 | altura 36 | `MeterScreen` |
| Q2 | Barra de gestão, só ícones (sem Reports e Codex), com o botão de overlays (olho) na frente | direita 8, y 12 | altura 52 | `ManagementMenu`; o de overlays é cópia de um botão dela |
| H4 | Notificações, a 80% | 56, 118 | | `NotificationScreen` |
| H5 | Construir | 56, 600 | 88x88 | novo (abre B1) |
| H11 | Trilho de ferramentas: 6 + Mais | direita 8, 88 | 72x532 | toggles do `ToolMenu` |
| H12 | Sair da ferramenta | direita 8, 620 | 92x92 | novo (mesmo efeito do botão direito) |
| B1 | Categorias, 2 colunas | 56, 88 | 160x624 | grade do `PlanScreen` |
| B2 | Grade de prédios | 224, 88 | 320x624 | `BuildingGroups` do `PlanScreen` |
| B3 | Painel do prédio | 56, 88 | até 624 de altura | `ProductInfoScreen` |
| B4 | Girar | direita 108, 620 | 92x92 | novo (chama `BuildTool.TryRotate`) |
| P | Barra de prioridade 1-9 + emergência | centro, 640 | 768x72 | `PriorityScreen` do `ToolMenu` |
| — | Filtro da ferramenta | esquerda do trilho, 88 | | `ToolParameterMenu` |
| D1 | Painel de detalhes | direita 8, 88 | até 624 de altura | `RootMenu` com o `DetailsScreen` |
| M1 | Telas de tabela | 56, 88 | largura da tela x 624 | `JobsTableScreen` e afins |
| O1 | Overlays, 5 colunas | direita 8, 88 | | toggles do `OverlayMenu` |
| O2 | Legenda do overlay | 136, de baixo para cima | | `OverlayLegend` |
| Q1 | Diagnóstico e recursos, painéis dobráveis como no PC | esquerda do trilho, 88 | 300 | `ColonyDiagnosticScreen`, `PinnedResourcesPanel` |
| Q3 | Mais ferramentas, 3x2 | direita 84, 432 | | toggles do `ToolMenu` |

Os painéis do jogo são movidos para dentro de holders do HUD (`OniMobile.Hud`, no
`ScreenSpaceOverlayCanvas`), e a posição, o tamanho e a escala são reaplicados a cada quadro, porque
o jogo redefine alguns quando atualiza. O `ManagementMenu` e o `OverlayMenu` destroem e recriam os
toggles no lugar original quando atualizam (troca de dispositivo de entrada, pesquisa concluída); o
HUD adota os novos a cada quadro.

## Regra de convivência

- Slot 0, sempre na tela: H1 a H12. A coluna da esquerda (H4, H5, H6) some quando B1 ou M1 a
  cobrem; o trilho H11 some com construção, seleção ou tela de gestão abertas.
- Slot A, trabalho, um por vez: construção (B1 a B4), ferramenta ativa (H11, P, H12), seleção (D1).
  Abrir um fecha o anterior.
- Slot B, consulta, um por vez: O1, Q3. Q1 fica na tela enquanto o lado direito está livre
  (sem construção, seleção, tela de gestão ou popover). Fecha com toque no mapa (o toque é consumido por uma
  camada transparente e não chega ao jogo), com toque longo ou ao abrir outro. Com um popover
  aberto, a grade B2 e o painel D1 se recolhem até ele fechar.
- Slot C, tela de gestão: fecha A e B.
- Toque longo (botão direito) fecha um nível por vez: o popover, depois o menu de construção; o jogo
  cuida do resto (ferramenta, seleção, tela de gestão).

## Substituto do hover

- Célula: o cartão de hover do jogo (`HoverTextScreen`) fica onde foi o último toque. A mira do
  design (H6, C2) foi feita e retirada a pedido.
- O modo dica (H9, C3) também foi feito e retirado a pedido.
- Rótulo efêmero: ao ativar uma ferramenta, o nome dela aparece por 1,8 s acima da barra P.

## Diferenças em relação ao design

- M1 em folha de 600 px não cabe: a tabela de Priorities tem 2093 px a 175%. As telas de tabela
  abrem com a largura da tela, do x 56 em diante, e as mais largas rolam para o lado.
- M2 (Research, Starmap, Codex, Options em tela cheia com abas e "‹ Mapa"): ficam como no jogo, que
  já abre essas telas em tela cheia com o próprio botão de fechar.
- H3 é o alerta vermelho do canto superior esquerdo do PC; o contador de alertas de colônia (Q5) não
  foi feito. Não há H10: os painéis de diagnóstico e recursos ficam à vista e os
  medidores do PC (`MeterScreen`) ocupam uma segunda linha sob a velocidade.
- H4 é a lista original de notificações, inteira e a 80% (recortada em 72 px ficava ilegível),
  sem o popover Q4.
- B3 começa recolhido (o cabeçalho do próprio painel expande) e encolhe até 70% se não couber.
- Não feito: raio de toque de 36 px para selecionar objetos pequenos, o cartão C1 ao lado do dedo
  por 4 s (o cartão do jogo fica onde foi o último toque), a trava de orientação ou o espelhamento
  da margem da câmera quando a tela gira.

## Como verificar no PC

`tools/scripts/ui_check.sh --mono <save> <saída> "<passos>"` roda o jogo Mono do PC numa janela de
1600x720, com os assemblies do porte e o HUD compilado na hora (~50 s), e o probe
(`tools/layout/Probe.cs`) tira prints e dumps da árvore da UI. Sem `--mono` roda o player IL2CPP de
`build/linux-swap`. Os estados do design ficam em `tools/layout/States.cs` (`idle`, `build`, `place`,
`dig`, `select`, `prio`, `research`, `overlay`) mais `popoverlays`, `popmore`
e `buildmenu`. `make run` deixa o jogo aberto para testar à mão.
