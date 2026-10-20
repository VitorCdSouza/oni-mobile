# Brief: UI de Oxygen Not Included para celular

Preciso de um redesenho da interface do Oxygen Not Included (ONI) para jogar num celular em
paisagem, com toque. É um porte pessoal do jogo de PC para Android, não um jogo novo: o
conteúdo das telas é o mesmo do PC. O que muda é onde cada painel fica, o tamanho, o que abre e
fecha e como se interage sem mouse.

## Aparelho

- Poco X5 5G: tela de 6,67" AMOLED, 2400x1080, proporção 20:9, ~395 ppi, só paisagem.
- Furo da câmera frontal numa das bordas curtas, no meio da altura: deixar uma margem livre ali.
- O jogo renderiza abaixo da resolução da tela (hoje 1600x720, talvez 1333x600) e o sistema
  amplia. Use 1600x720 como referência de pixels nos desenhos e dê também os tamanhos em mm.
- Alvo de toque mínimo de ~9 mm (cerca de 53 px na referência de 1600x720).

## Como o toque funciona hoje (fica assim)

- Toque: clique.
- Toque longo (0,5 s): botão direito, que cancela a ferramenta ativa e fecha menus.
- Um dedo arrastando: move a câmera; se começar sobre a UI ou com uma ferramenta ativa (cavar,
  construir), vira arrasto da ferramenta (área de cavar, lista rolando, slider).
- Dois dedos: move e dá zoom com pinça.
- Não existe hover. No PC o jogo mostra muita informação ao passar o mouse (cartão com o que
  está na célula, tooltips dos botões); no celular isso precisa de outro gatilho.

## Elementos da UI do PC (todos precisam continuar acessíveis)

- Barra superior direita, telas de gestão: Vitals, Consumables, Priorities, Schedule, Skills,
  Research, Starmap, Reports, Codex, Options. Cada uma abre uma tela grande (tabela, árvore de
  pesquisa, mapa).
- Barra de Overlays: oxigênio, energia, temperatura, materiais, luz, líquidos, gases, decor,
  germes, plantas, salas e outros; troca a visualização do mapa.
- Canto superior esquerdo: ciclo e hora do dia, controles de velocidade (pausa, 1x, 2x, 3x),
  alertas de colônia.
- Lado direito: Colony Diagnostics (comida, oxigênio, estresse...) e lista de recursos.
- Lado esquerdo: notificações (mensagens, alertas de duplicante).
- Inferior esquerdo: categorias de construção (Base, Oxygen, Power, Food, Plumbing,
  Ventilation, Refinement, Medicine, Furniture, Stations, Utilities, Automation, Shipping,
  Rocketry, Radiation). Uma categoria abre a grade de prédios; escolher um prédio abre o
  painel do prédio (material, efeitos, custo).
- Inferior direito: ferramentas (Dig, Cancel, Deconstruct, Priority, Disinfect, Sweep, Mop,
  Attack, Capture, Harvest, Empty Pipe, Copy Settings) e a barra de prioridade 1 a 9 com
  emergência.
- Painel de detalhes ao selecionar algo no mapa (duplicante, prédio, célula): abas, status,
  configurações do prédio (sliders, chaves, filtros).

## O problema

Em 20:9 e a 720 linhas, com a escala que deixa os botões tocáveis, os painéis se sobrepõem e
cobrem quase todo o mapa: menu de construção, painel do prédio, overlays e diagnósticos abertos
juntos não deixam espaço para construir. A interface também conta com hover e com precisão de
mouse que o dedo não tem.

## O que eu quero

1. Layout de referência em 1600x720 para estes estados:
   - mapa sem nada aberto (o que fica sempre na tela, o que vira ícone);
   - menu de construção aberto, com um prédio escolhido e pronto para posicionar;
   - ferramenta ativa (cavar) com a barra de prioridade;
   - algo selecionado no mapa, com o painel de detalhes;
   - uma tela de gestão aberta (Priorities ou Research);
   - um overlay ativo.
2. Regra de convivência entre painéis: quais podem ficar abertos juntos e quais fecham os
   outros. Com qualquer painel aberto, ao menos ~60% do mapa deve continuar visível.
3. Substituto do hover: como ver o que tem numa célula e ler o tooltip de um botão.
4. Tamanhos de tudo (botões, barras, gavetas, fontes) em px na referência e em mm.
5. Para cada elemento do desenho, qual elemento do jogo original ele é (pela lista acima).

## Restrições de implementação

- A implementação é feita por cima da UI que o jogo já tem (Unity uGUI, telas do próprio jogo):
  dá para mudar âncora, tamanho, escala, ordem e visibilidade dos painéis existentes, e
  acrescentar alguns elementos simples (botões, gavetas, barras laterais) criados por código.
- Não dá para redesenhar o conteúdo interno de cada tela (as tabelas, a árvore de pesquisa, o
  painel do prédio): elas mudam de lugar e de tamanho, não de miolo.
- Manter a identidade visual do ONI: as mesmas cores, fontes e ícones do jogo. Nada de estilo
  novo.
- Sem arte nova que exija ilustração; ícones existentes do jogo e formas simples.
