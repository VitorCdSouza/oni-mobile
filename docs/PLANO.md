# oni-mobile — porte nativo do Oxygen Not Included para Android

Uso pessoal, cópia própria do jogo (Steam, build U59, Unity 6000.3.5f2, backend Mono).
Alvo: Poco X5 5G (Snapdragon 695, 2x Cortex-A78 + 6x A55, Adreno 619, 8 GB de RAM).
Nada deste repositório (código decompilado, assets, binários do jogo) é distribuído.

## Por que é difícil

| Peça | Situação na build Linux | O que o Android exige |
| --- | --- | --- |
| Scripts C# (`Assembly-CSharp.dll`, 14 MB) | IL rodando em Mono JIT | ARM64 no Android só existe com IL2CPP (AOT) |
| `libSimDLL.so` (simulação de gás, líquido, calor) | C++ fechado, só x86_64, sem versão ARM nem no Mac | Reimplementar em C++ e compilar para arm64-v8a |
| Shaders | Compilados para GL Core / Vulkan desktop | GLES 3 / Vulkan mobile |
| AssetBundles em `StreamingAssets` (1,1 GB) | Construídos para StandaloneLinux64 | Rebuild para Android |
| FMOD, Resonance Audio | `.so` x86_64 | Versões Android da mesma versão do FMOD |
| Steamworks | Jogo fecha sem Steam | Stub |
| Interface | Mouse e teclado | Toque |

## Fase 0 — provas de viabilidade (decide se o projeto continua)

1. **SimDLL**: decompilar com Ghidra, medir o tamanho real do código, e montar um harness de replay:
   gravar as chamadas `SIM_*` de uma partida real no PC (shim via `LD_PRELOAD`) e reproduzir
   contra a lib original e contra a reimplementação, comparando os buffers de saída byte a byte.
2. **IL2CPP**: reconstruir o projeto Unity com AssetRipper (modo DLL, mantendo as assemblies originais)
   e gerar primeiro uma build IL2CPP para **Linux**, para separar problemas de AOT dos problemas de Android.
3. **Shaders**: extrair os shaders customizados (a build Linux guarda GLSL) e testar a conversão para GLES 3.

### Resultados da Fase 0 até aqui

- Decompilação: 425 funções da simulação (~34 mil linhas de pseudo-C do Ghidra), 100 do profiler
  embutido, 418 de STL/runtime e 163 sem nome.
- Interface com o C#: ~30 funções exportadas, a maior parte por mensagens (`SIM_HandleMessage`).
- Gravação de referência: `sim/recordings/porAgua2-c426.bin`. É o save "Por Agua Abaixo 2" no ciclo
  426, com uns 4 minutos em velocidade 3, 3182 frames de simulação e 3 saves (750 MB, não versionado).
- **A simulação é determinística**: dois replays da mesma gravação dão os mesmos hashes nas 5549
  observações. O resultado também não muda com 1, 8 ou 28 núcleos (só existe uma `SimThread`).
- A única diferença entre rodadas está no primeiro blob de save: ~478 floats de lixo vindos de
  memória não inicializada. O `compare.py` compara saves só pelo tamanho.

### Auditoria dos assets (provas 2 e 3)

`tools/venv/bin/python -I tools/scripts/asset_audit.py <OxygenNotIncluded_Data> build/audit/assets.json`
lista texturas e shaders dos arquivos principais e dos AssetBundles de `StreamingAssets` (UnityPy
1.25.4, 12 s). Os números abaixo são da build U59 instalada.

Texturas: 4326, ~3,4 GB de dados de imagem.

| Formato | Quantidade | MB |
| --- | --- | --- |
| DXT5 | 3996 | 3019 |
| RGBA32 | 122 | 181 |
| DXT1 | 128 | 114 |
| Alpha8 | 65 | 108 (inclui 3 atlas CJK de 4096x4096) |
| RGB24 | 14 | 18 |

- Por origem: 2130 MB nos `sharedassets`/`resources`, 874 MB nos bundles de DLC, 437 MB nos
  bundles `hires_*` (dá para não levar estes ao celular).
- Maior textura: 4096x4096. Os atlas de kanim são DXT5 de 2048 a 4096.
- GPUs Adreno não leem BC/DXT. A Unity descomprime para RGBA na carga, 4x o tamanho: inviável em
  8 GB. Recomprimir é obrigatório. Estimativa sem os `hires_*`: ASTC 6x6 ~1,4 GB, ASTC 8x8 ~0,8 GB.
  Falta confirmar no aparelho quais formatos o driver anuncia.

Shaders: 84, todos compilados **só para `OpenGLCore`**, ~60 da Klei. Descomprimindo os blobs
(LZ4) saem 413 programas GLSL (`build/audit/glsl/`):

- 349 em `#version 150` e 64 em `#version 420`. Os de 420 são só as variantes de instancing
  procedural dos shaders de partícula (SSBO).
- Extensões: `GL_ARB_explicit_attrib_location`, `GL_ARB_shader_bit_encoding`,
  `GL_ARB_shading_language_420pack`, `GL_ARB_shader_storage_buffer_object`. Todas viram recurso
  nativo do GLES 3.0/3.1.
- Nenhum uso de `sampler1D`, `double`, `textureQueryLod`, `textureGather`, `gl_ClipDistance`,
  `noperspective` ou image load/store.
- A conversão para GLES 3 parece mecânica: trocar o cabeçalho, pôr qualificadores de precisão,
  tirar as diretivas de extensão e regravar o blob com a plataforma `GLES3Plus` (9).

Consequência para a Fase 2: reconstruir o projeto com AssetRipper perde os shaders. Ele não recupera
ShaderLab a partir de GLSL compilado, então os ~60 shaders da Klei teriam de ser reescritos à mão.
O caminho que sobra é a **troca de dados**:

1. Projeto Unity 6000.3.5f2 vazio, com as DLLs originais de `Managed/` como plugins, stripping
   desligado e build Android IL2CPP. Disso só se aproveitam `libil2cpp.so`, `global-metadata.dat` e
   o player.
2. Os arquivos de dados da build Linux, convertidos: `m_TargetPlatform` trocado para Android,
   texturas recomprimidas em ASTC e blobs de shader GLSL convertidos para GLES 3.
3. Plugins nativos para arm64 (ver Fase 2).

Riscos que este caminho ainda precisa provar: o player Android aceitar o `globalgamemanagers` de
outra plataforma, e o IL2CPP resolver os `MonoScript` das cenas pelas assemblies pré-compiladas.

## Fase 1 — SimDLL arm64

Reimplementação em C++ a partir do decompilado, validada pelo replay.
Compilada com `-ffp-contract=off` para não divergir do x86 por FMA.
O profiler embutido (`kprofiler_*`, sockets, HTTP) vira stub.

Método incremental: cada função reescrita é injetada na própria lib original com um detour
(salto absoluto no início da função original para a versão nova, compilada para x86_64 com as
mesmas estruturas). As funções ainda não portadas continuam sendo chamadas pelo endereço original.
Assim cada função é validada isoladamente contra a gravação, e a lib original é aposentada só
quando todas as funções tiverem sido substituídas. Aí o mesmo código é compilado para arm64-v8a.

Ferramentas do ciclo:

- `sim/layout/layout.txt`: structs do original por offset. `tools/redecomp.sh` aplica no Ghidra,
  reexporta o decompilado com nomes de campo e gera `port/src/sim_types.h` (structs C++ com
  `static_assert` de offset).
- `make -C sim/port hybrid`: compila o port. `SIMPORT=sim/build/libsimport.so sim/build/replay ...`
  roda o replay com as funções portadas; `compare.py` contra `sim/recordings/orig.txt`.
- Os bytes de padding das structs de evento são ignorados no hash: o original deixa lixo da pilha
  neles e o jogo não lê.
- `make -C sim/port coverage` + replay + `make -C sim/port coverage-report`: lista as linhas do
  port que nenhuma gravação executou.

### Funções portadas

| Função | Validação |
| --- | --- |
| `ConduitTemperatureManager::CalculateEnergyFlow` | replay idêntico, 100% das linhas |
| `DoStateTransition` | replay idêntico, 68% das linhas |
| `DoSublimation` | replay idêntico, 84% das linhas |
| `DoPartialHeatTransition` (as duas sobrecargas) | replay idêntico, 64% das linhas |
| `PostProcessCell` (com a lambda `FUN_0016f980`) | replay idêntico, 91% das linhas |
| `SimEvents::ChangeSubstance`, `SimEvents::SpawnFX` | replay idêntico, 100% das linhas |
| `AddMassAndUpdateTemperature`, `DoDisplacement`, `DisplaceGas`, `DisplaceLiquid`, `DisplaceLiquidSimple`, `Evaporate` | replay idêntico, 88% das linhas (o resto é caminho de assert) |
| `DoDensityDisplacement`, `DoPressureBreak` (as duas sobrecargas) | replay idêntico, 98% das linhas |
| `DoPartialMelt` (as duas sobrecargas), `DoUnstableCheckBasic` | replay idêntico, 48% das linhas |
| `SimData::ClearCell`, `SimData::GetStableTicksRemaining`, `Disease::AddDiseaseToCell` | replay idêntico, 90% das linhas |
| `DoUnstableCheckWithDiagonals`, `HeadlessUnstableFallSimpleSwap` | replay idêntico, **0% das linhas** (opção desligada no save gravado) |
| `UpdateLiquid` (com a função interna de mover líquido), `IsLiquidPermeable` | replay idêntico, 96% das linhas |
| `UpdateTemperature` | replay idêntico, 76% das linhas (o resto é assert); escrita a partir da disassembly, porque o original usa `double` no meio da conta e o decompilado mostra `float` |

| `SimUpdateTemperatureTask`, `UpdateCellSOATask`, `UpdateBackwallTask`, `UpdateCellTask<Vector4<float>>` (`InternalDoTask`) | replay idêntico, 97% das linhas |
| `UpdateTemperatureForBackwallExchange` | replay idêntico; também em `double`, a partir da disassembly |

| `BuildingHeatExchange::Update`, `BuildingToBuildingHeatExchange::Update` | replay idêntico, 86% das linhas |
| `ConduitTemperatureManager::Update`, `ClampEnergyTransfer`, `ModifyTemperature` | replay idêntico |
| `UpdatePressure`, `DoGasPressureDisplacement`, `DoLiquidPressureDisplacement`, `Displace*Directional` | replay idêntico |
| `Disease::UpdateCells`, `Disease::GetDiffusionScale` | replay idêntico |
| `SimBase::UpdateData` (o tick inteiro: temperatura, pressão, líquidos, germes, radiação, pós-processamento) | replay idêntico, 97% das linhas |

| `SimData::UpdateComponents` (despacho), `ElementConsumer::Update`, `GetReachableCells`, `FloodRemoved` | replay idêntico, 98% |
| `ElementEmitter::Update` (e o thunk clonado `_ZThn128_`) | replay idêntico, 74% |
| `DiseaseEmitter::Update` | replay idêntico, **20%** (nenhum emissor de germe ativo na gravação) |
| `ElementChunk::Update` | replay idêntico, 95% |
| `RadiationEmitter::Update` (com `Line`, rasterizador de elipse e callbacks) | replay idêntico, **42%** (modos de pulso não exercitados) |
| `UpdateDataListOnly` dos componentes (e os thunks clonados), `SimData::UpdateComponentsDataListOnly` | replay idêntico (frames pausados) |
| `SimBase::Step`, `SimBase::ConsolidateEvents`, `SimBase::CopySimDataToGame` | replay idêntico, 91% |
| `UpdateFlowTexture`, `UpdateLiquidPropertyTexture`, `UpdateExposedToSunPropertyTexture` (e `LerpColor`, `GetEstimatedLiquidGradientValue`) | replay idêntico, 94%; texturas entraram no hash do replay |
| `Disease::PostProcess`, `CalculateRangeHalfLife`, `HalfLifeToGrowthRate`, `GetDiseaseIndex`, `GetElementIndex` | replay idêntico, 90% |
| `SimFrameManager`: `NewFrame`, `Begin/EndFrameProcessing`, `ProcessNextFrame`, `HandleMessage`, `ProcessFrame` | replay idêntico, 73-75% |
| Register / Modify / Unregister de todos os componentes, `InitializeFromMessageData` | replay idêntico, 69% |
| `ProcessCellModifications`, `AddGas/Liquid/Solid`, `RemoveGas/Liquid/Solid`, `ReplaceElement`, `ReplaceAndDisplaceElement`, `RectangularRemoved`, `SimData::ClearBackwall` | replay idêntico, 66% |

Com isso o caminho inteiro de um frame (mensagens, tick, componentes, cópia para o jogo) roda no
port. O que ainda chama o original: assert, relatório de crash, logger e `Math::FloatConstants::PI`.
Contagem: 124 funções do original substituídas, ~108 KB de 208 KB (contando o código da STL
embutido). Falta a camada de carga e API: `SIM_*`, `FrameSync`, `Sim::Main`, construtor e
destrutor do `SimData`, `CreateElementsTable`, tabela de germes, `Load`/`Save`, `Start`,
`PrepareGameData(Update)` e o `GameData`.

Achados dos componentes:

- O thunk de interface de `ElementEmitter::Update` (o que a vtable chama) é uma cópia inteira da
  função no original, não um salto; o port define os dois. `DiseaseEmitter::UpdateDataListOnly` e
  `BuildingHeatExchange::UpdateDataListOnly` também têm thunks clonados.
- Várias funções devolvem structs por ponteiro oculto (`Handle`, `EmittedMassInfo`, a contagem de
  germes): ficam como funções internas do port, sem patch por nome.
- O rasterizador de elipse do `RadiationEmitter` usa `rsqrtss`, cuja aproximação muda entre Intel e
  AMD: nem o original é bit a bit igual entre PCs. O port usa a mesma instrução no x86 e `1/sqrt` no
  ARM.
- O laço de linhas do emissor contínuo termina em `2 * radiusX` (não `radiusY`): replicado.
- `RadiationEmitter::Register` calcula a duração do pulso com `emitRads` no lugar de `emitRate`
  (o `Modify` usa o campo certo): replicado.
- `BuildingToBuildingHeatExchange::Register` publica o handle novo no callback, e o
  `ProcessFrame` publica de novo: o jogo recebe dois eventos. Replicado.
- `Unregister` só olha o bit de handle inválido, não a versão. `ElementEmitter::Register` zera o
  slot de emissão pelo índice denso, não pelo índice do handle.
- `Handle` tem construtor de cópia próprio no original, então é devolvido por ponteiro oculto e
  passado por referência invisível. O port declara o mesmo para chamar e ser chamado pelo original.

Achados do frame e da cópia para o jogo:

- `ConsolidateEvents` junta os eventos `buildingNoLongerOverheated` em `buildingOverheat`.
- O minério gerado é ordenado com `std::sort` antes de juntar duplicatas. Como `std::sort` não é
  estável, o resultado depende do algoritmo: o port leva uma cópia do introsort da libstdc++
  (`stl_sort.h`), testada contra o `std::sort` do PC, para dar o mesmo resultado com a libc++ do
  Android.
- `UpdateLiquidPropertyTexture` não escreve a última linha do mapa; usa `powf`, e
  `HalfLifeToGrowthRate` usa `exp2f` (mesma ressalva da libm no Android).
- `SetDebugProperties` muda a escala de troca de calor dos prédios (o jogo manda 100, não o
  padrão 0,001) e a flag de edição de debug.

Achados do loop principal:

- `SimBase::GetNumWorkers` devolve 1: a fila de tarefas tem um único worker, então o original já roda
  as tarefas em série. O port faz o mesmo e continua exato.
- Os dois passes de deslocamento por pressão começam na linha `min(minY, 3)` (provavelmente era para
  ser `max`): para regiões acima do fundo do mapa eles varrem a partir da linha 3. Replicado.
- A radiação usa um kernel 5x5 fixo (inicializado na primeira chamada) e uma grade de "luz do céu"
  em `SimData+0x40`, propagada de cima para baixo.

Teste de sensibilidade: trocar a constante `(double)0.2f` por `0.2` exato em `UpdateTemperature`
(diferença relativa de 3e-9) diverge na observação 247. O replay pega erro de último bit.

Atenção para o Android: `UpdateTemperature` chama `logf`/`expf` (células sem isolamento dos dois
lados). A libm do Android (bionic) pode diferir da glibc no último bit, então ali o resultado no
celular pode não ser idêntico ao do PC. Não afeta jogabilidade; afeta só a comparação bit a bit.

Ferramenta extra: `python3 sim/gen/rodata.py sim/libSimDLL.x86_64.so --symbol <nome mangled>` lista
as constantes que uma função lê da memória com o valor exato. O Ghidra arredonda os literais
(1/32767 aparece como `3.051851e-05`, mas o original usa `0x1.0002p-15f`).

### Build standalone

`make -C sim/port standalone` gera `sim/build/standalone/libSimDLL.so` só com o código do port: a
camada de carga e API (`SIM_*`, `ConduitTemperatureManager_*`, `FrameSync`, thread da sim, `Load`,
`SIM_BeginSave`, construção do `SimData`/`GameData`) foi escrita direto para ele. O replay roda
com essa lib no lugar da original:

    sim/build/replay sim/build/standalone/libSimDLL.so sim/recordings/porAgua2-c426.bin

Resultado: as 5549 observações idênticas às do original, e os saves byte a byte iguais (menos o
primeiro, que já tem lixo de memória no próprio original).

Achados:

- `CellSOA` nasce com elemento 0xffff, isolamento 0xff, resistência 0x84 e sem germe.
- O slot novo de `ElementChunkInfo` não é inicializado no original (o construtor padrão não faz
  nada): o primeiro `deltaKJ` de um detrito novo é lixo do heap somado à energia do passo (deu NaN
  na gravação). O port usa 0; o replay mascara esse campo como padding. O jogo só usa o valor numa
  média de kW exibida.
- A thread da sim liga flush-to-zero (`MXCSR |= 0x8000`). No ARM o bit FZ do FPCR também zera
  denormais na entrada, o que o x86 não faz: possível diferença no celular.
- `DoGasInteraction` (e a tabela de interações) e `ShouldDoLiquidConversion` são código morto no
  original.

### arm64

`tools/arm64/Dockerfile` monta um container com `g++-aarch64-linux-gnu` e `qemu-user`. Dentro dele:

    make -C sim/port standalone CXX=aarch64-linux-gnu-g++ BUILD=../build/arm64
    aarch64-linux-gnu-gcc -O2 -o sim/build/arm64/replay sim/replay/replay.c -ldl
    qemu-aarch64 -L /usr/aarch64-linux-gnu sim/build/arm64/replay sim/build/arm64/standalone/libSimDLL.so <gravação>

Resultado: as 5549 observações idênticas às do original x86 (37 min sob qemu). Isso usa a libm da
glibc; a do Android (bionic) ainda pode diferir no último bit em `logf`/`expf`/`exp2f`/`powf`.

Desempenho no PC (replay inteiro, Xeon E5-2690 v4): original 69,8 s, port 62,7 s.

### Gravações que faltam

A gravação de 4 minutos não passa por todos os ramos. Vai ser preciso um save de laboratório no
modo sandbox, montado para exercitar mecânicas específicas. Lista até agora:

- transições que soltam minério (água poluída fervendo, magma esfriando);
- tiles construídos que derretem (aviso `NotifyOnMelt`);
- congelamento de massa grande que vira tile em vez de detrito;
- tile inteiro sublimando (pouca massa sobrando, ex.: alvejante quase no fim);
- fervura parcial de líquido virando vapor ao lado de algo muito quente (ex.: água sobre magma);
- sólido que some todo por sublimação (`Evaporate`);
- gás muito quente encostado num tile sólido, derretendo 5 kg por vez (`DoPartialMelt`; a gravação
  atual nunca passa das verificações iniciais);
- um save com a opção "areia caindo na diagonal" (`ENABLE_DIAGONAL_FALLING_SAND`) ligada e areia,
  terra ou regolito sem apoio (`DoUnstableCheckWithDiagonals`, hoje sem nenhuma cobertura);
- um emissor de radiação pulsante (ex.: colmeia de Beeta) e um emissor de germes ativo (latrina
  em uso, Morb) dentro da área visível.
- minério gerado (`spawnOre`): a gravação atual não gera nenhum, então a ordenação e a junção de
  duplicatas em `CopySimDataToGame` estão sem cobertura;
- `AddSolid` com subtipo 0 (sólido caindo e empilhando), `AddLiquid` em célula sólida, `Dig` de
  parede de fundo.

## Fase 2 — Build Android

IL2CPP arm64, stub de Steam, FMOD Android, AssetBundles refeitos, shaders convertidos.
Primeiro objetivo: chegar ao menu principal no aparelho.

Plugins nativos do jogo e o que cada um precisa no Android:

- `libSimDLL.so`: o port (pronto, arm64 validado).
- `libfmodstudio.so`: FMOD 2.03.12 (`FMOD.VERSION.number = 0x20312` no FMODUnity.dll); a build
  Android sai do site da FMOD (precisa de conta).
- `libresonanceaudio.so`: plugin de áudio espacial do Google para FMOD, código aberto, tem build
  Android.
- `cimgui.so`: Dear ImGui 1.82 em C (menu de debug), código aberto, compila para arm64.
- `libsteam_api.so`, `libsdkencryptedappticket.so`: substituir por stubs (Steamworks.NET chama via
  DllImport).
- O C# decompilado também declara `DllImport` de `rail_api64` (4979, WeGame), `EOSSDK-Win64-Shipping.dll`
  (360, Epic Online Services) e `kernel32` (2). O IL2CPP só resolve a biblioteca na primeira chamada,
  então basta garantir que esses caminhos não rodem no Android; se rodarem, precisam de stub.

## Fase 3 — Jogabilidade

Controles de toque (pan, pinch zoom, toque longo como clique direito), escala de UI,
ajustes de desempenho (resolução, limite de fps, mundo pequeno).

## Expectativa de desempenho

No Xeon E5-2690 v4 o save "Sustentaveis" (ciclo ~2924) roda a ~23 fps com a thread principal em 94%.
Um A78 a 2,2 GHz deve render algo entre 60% e 70% disso em single-thread. Começo e meio de jogo devem
ficar jogáveis; colônias grandes vão ficar lentas.
