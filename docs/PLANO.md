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

## Fase 1 — SimDLL arm64

Reimplementação em C++ a partir do decompilado, módulo por módulo, validada pelo replay.
Compilada com `-ffp-contract=off` para não divergir do x86 por FMA.
O profiler embutido (`kprofiler_*`, sockets, HTTP) vira stub.

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
