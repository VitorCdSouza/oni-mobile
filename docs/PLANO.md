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
  terra ou regolito sem apoio (`DoUnstableCheckWithDiagonals`, hoje sem nenhuma cobertura).

## Fase 2 — Build Android

IL2CPP arm64, stub de Steam, FMOD Android, AssetBundles refeitos, shaders convertidos.
Primeiro objetivo: chegar ao menu principal no aparelho.

## Fase 3 — Jogabilidade

Controles de toque (pan, pinch zoom, toque longo como clique direito), escala de UI,
ajustes de desempenho (resolução, limite de fps, mundo pequeno).

## Expectativa de desempenho

No Xeon E5-2690 v4 o save "Sustentaveis" (ciclo ~2924) roda a ~23 fps com a thread principal em 94%.
Um A78 a 2,2 GHz deve render algo entre 60% e 70% disso em single-thread. Começo e meio de jogo devem
ficar jogáveis; colônias grandes vão ficar lentas.
