// Patches a game assembly for the Android player.
//
// Usage: PatchAndroid.exe <in.dll> <out.dll> <game Managed dir> <OniMobile.Runtime.dll>
//
// 1. StreamingAssets.
// On Android Application.streamingAssetsPath points inside the APK, which
// System.IO cannot read, and the game reads its YAML, codex and bundles with
// System.IO. Every read of Application.streamingAssetsPath goes to
// OniMobile.StreamingAssets.Path (tools/runtime/StreamingAssets.cs), which on
// Android copies those files out of the APK to <persistentDataPath>/StreamingAssets
// and returns that directory, and the usual path elsewhere. FMOD's hardcoded
// "file:///android_asset" is left alone: it reads the banks from the APK.
//
// 2. Steam. SteamManager.Awake quits the game when Steam cannot start, which is
// always the case on Android (libsteam_api.so is a stub there). The quit calls
// are dropped, so the game runs with SteamManager.Initialized false.
// DistributionPlatform.Initialize then logs "Steam not initialized in time." as
// an error, which opens the crash screen; it is logged as a warning instead.
//
// 3. Crash screen. KCrashReporter opens on any logged error. The PC data has no
// Hidden/VideoDecodeAndroid shader, so the main menu video logs errors that are
// harmless (the video just does not show); they join KCrashReporter.IgnoreStrings.
//
// 4. Native callbacks. IL2CPP only marshals a managed method to a native function
// pointer when the method carries an attribute named MonoPInvokeCallback; Mono on
// the PC does not need it, so the game lacks it (Sim.DLL_MessageHandler, passed to
// SIM_Initialize, fails worldgen). Every static method that becomes a delegate of
// a type passed to a P/Invoke, or marked [UnmanagedFunctionPointer], gets an
// injected OniMobile.MonoPInvokeCallbackAttribute.
//
// 5. Touch. Calls to UnityEngine.Input go to OniMobile.TouchInput (the method of
// the same name and signature, tools/runtime/TouchInput.cs), which turns touch
// gestures into a mouse on Android. KCanvasScaler's default UI scale comes from
// OniMobile.Platform.DefaultUIScale.
//
// 6. Null delegate targets. Mono lets `new Func<T>(obj.Method)` through when obj
// is null (it only fails if the delegate is called); IL2CPP throws on creation.
// ManagementMenu does this with the Spaced Out cluster map screen in a base game
// save, which aborts its setup. Such creations now give a null delegate instead.
//
// 7. Render resolution. Calls to Screen.SetResolution go to OniMobile.Display
// (tools/runtime/Display.cs), which caps the height on Android.
//
// 8. Idle UI raycast. StandaloneInputModule.ProcessMouseEvent(int) returns right
// away while OniMobile.TouchInput.PointerIdle() is true (no finger down, see
// TouchInput.cs), instead of raycasting the whole UI every frame for hover.
//
// 9. Core count. Reads of SystemInfo.processorCount go to OniMobile.SystemInfo, which
// falls back to the OS count when Unity's is lower (1 under the emulator's translation,
// where the game's JobManager cannot start).
//
// 10. Render work with nothing new (tools/runtime/Render.cs). PropertyTextures uploads
// the sim's textures through OniMobile.SimTextures, which skips them until Game.StepTheSim
// (marked here) runs again; the overlay camera's two post effect components turn off
// while they have no effect to draw, and back on when one is added.
//
// 11. FMOD's Java side. FMODUnity.RuntimeManager.Initialize starts with a call to
// OniMobile.Fmod.InitJava (tools/runtime/Fmod.cs), the org.fmod.FMOD.init call that
// the PC build of FMODUnity compiled out; FMOD needs it to read the banks from the APK.
using System;
using System.Collections.Generic;
using System.Linq;
using Mono.Cecil;
using Mono.Cecil.Cil;
using Mono.Cecil.Rocks;

static class PatchAndroid
{
    static int Main(string[] args)
    {
        var module = ModuleDefinition.ReadModule(args[0], new ReaderParameters { AssemblyResolver = new NameResolver(args[2]) });
        var quits = DropSteamQuit(module);
        var quiet = QuietSteamError(module);
        var ignored = IgnoreErrors(module);
        var callbacks = MarkCallbacks(module);
        var runtime = ModuleDefinition.ReadModule(args[3]);
        var inputs = Redirect(module, runtime, "UnityEngine.Input", "OniMobile.TouchInput");
        var resolutions = Redirect(module, runtime, "UnityEngine.Screen", "OniMobile.Display");
        var cores = Redirect(module, runtime, "UnityEngine.SystemInfo", "OniMobile.SystemInfo");
        var scale = DefaultUIScale(module, runtime);
        var nullSafe = NullSafeDelegates(module);
        var idle = SkipIdlePointer(module, runtime);
        var render = SkipIdleRendering(module, runtime);
        var fmod = InitFmodJava(module, runtime);
        Console.WriteLine($"{module.Assembly.Name.Name}: {quiet} Steam errors made warnings, {ignored} crash screen messages ignored, "
            + $"{callbacks} native callbacks marked, {inputs} Input calls redirected, {resolutions} Screen calls redirected, "
            + $"{cores} core count reads redirected, "
            + $"{scale} UI scale defaults replaced, "
            + $"{nullSafe} delegate creations made null-safe, {idle} idle pointer raycasts skipped, "
            + $"{render} idle render steps skipped, {fmod} FMOD Java inits added");
        var getter = module.GetMemberReferences().OfType<MethodReference>()
            .FirstOrDefault(m => m.Name == "get_streamingAssetsPath" && m.DeclaringType.FullName == "UnityEngine.Application");
        if (getter == null)
        {
            module.Write(args[1]);
            Console.WriteLine($"{module.Assembly.Name.Name}: {quits} Steam quit calls dropped");
            return 0;
        }
        var helper = module.ImportReference(runtime.GetType("OniMobile.StreamingAssets").Methods.First(m => m.Name == "Path"));

        int calls = 0;
        foreach (var type in module.GetTypes())
        {
            foreach (var method in type.Methods.Where(m => m.HasBody))
            {
                foreach (var ins in method.Body.Instructions)
                {
                    if ((ins.OpCode == OpCodes.Call || ins.OpCode == OpCodes.Callvirt) && ins.Operand is MethodReference mr
                        && mr.Name == "get_streamingAssetsPath" && mr.DeclaringType.FullName == "UnityEngine.Application")
                    {
                        ins.OpCode = OpCodes.Call;
                        ins.Operand = helper;
                        calls++;
                    }
                }
            }
        }
        module.Write(args[1]);
        Console.WriteLine($"{module.Assembly.Name.Name}: {calls} streamingAssetsPath reads redirected, {quits} Steam quit calls dropped");
        return 0;
    }

    static int DropSteamQuit(ModuleDefinition module)
    {
        var awake = module.GetType("SteamManager")?.Methods.FirstOrDefault(m => m.Name == "Awake");
        if (awake == null)
            return 0;
        int dropped = 0;
        foreach (var ins in awake.Body.Instructions)
        {
            if (ins.OpCode == OpCodes.Call && ins.Operand is MethodReference mr && mr.DeclaringType.Name == "App")
            {
                // App.Quit() takes nothing; App.QuitCode(int) takes the exit code off the stack.
                ins.OpCode = mr.Parameters.Count == 0 ? OpCodes.Nop : OpCodes.Pop;
                ins.Operand = null;
                dropped++;
            }
        }
        return dropped;
    }

    // Points every call to a static method of `from` (a UnityEngine type) at the method of
    // the same name and signature in the runtime type `to`, when it has one.
    static int Redirect(ModuleDefinition module, ModuleDefinition runtime, string from, string to)
    {
        var replacement = runtime.GetType(to);
        int redirected = 0;
        foreach (var type in module.GetTypes())
        {
            foreach (var method in type.Methods.Where(m => m.HasBody))
            {
                foreach (var ins in method.Body.Instructions)
                {
                    if ((ins.OpCode != OpCodes.Call && ins.OpCode != OpCodes.Callvirt) || !(ins.Operand is MethodReference mr)
                        || mr.DeclaringType.FullName != from || mr.HasThis)
                        continue;
                    var target = replacement.Methods.FirstOrDefault(m => m.IsPublic && m.IsStatic && m.Name == mr.Name
                        && m.Parameters.Count == mr.Parameters.Count
                        && m.Parameters.Select(p => p.ParameterType.FullName).SequenceEqual(mr.Parameters.Select(p => p.ParameterType.FullName)));
                    if (target == null)
                        continue;
                    ins.OpCode = OpCodes.Call;
                    ins.Operand = module.ImportReference(target);
                    redirected++;
                }
            }
        }
        return redirected;
    }

    // RESULT RuntimeManager.Initialize() { Fmod.InitJava(); ... }
    static int InitFmodJava(ModuleDefinition module, ModuleDefinition runtime)
    {
        var initialize = module.GetType("FMODUnity.RuntimeManager")?.Methods
            .FirstOrDefault(m => m.Name == "Initialize" && m.Parameters.Count == 0 && m.HasBody);
        if (initialize == null)
            return 0;
        var init = runtime.GetType("OniMobile.Fmod").Methods.First(m => m.Name == "InitJava");
        var il = initialize.Body.GetILProcessor();
        il.InsertBefore(initialize.Body.Instructions[0], il.Create(OpCodes.Call, module.ImportReference(init)));
        return 1;
    }

    // ProcessMouseEvent(int id) { if (TouchInput.PointerIdle()) return; ... }
    static int SkipIdlePointer(ModuleDefinition module, ModuleDefinition runtime)
    {
        var process = module.GetType("UnityEngine.EventSystems.StandaloneInputModule")?.Methods
            .FirstOrDefault(m => m.Name == "ProcessMouseEvent" && m.Parameters.Count == 1);
        if (process == null)
            return 0;
        var idle = runtime.GetType("OniMobile.TouchInput").Methods.First(m => m.Name == "PointerIdle");
        var il = process.Body.GetILProcessor();
        var first = process.Body.Instructions[0];
        il.InsertBefore(first, il.Create(OpCodes.Call, module.ImportReference(idle)));
        il.InsertBefore(first, il.Create(OpCodes.Brfalse, first));
        il.InsertBefore(first, il.Create(OpCodes.Ret));
        return 1;
    }

    static int SkipIdleRendering(ModuleDefinition module, ModuleDefinition runtime)
    {
        int patched = 0;
        var textures = runtime.GetType("OniMobile.SimTextures");
        var step = module.GetType("Game")?.Methods.FirstOrDefault(m => m.Name == "StepTheSim");
        var update = module.GetType("PropertyTextures")?.Methods.FirstOrDefault(m => m.Name == "UpdateProperty");
        if (step != null && update != null)
        {
            var il = step.Body.GetILProcessor();
            il.InsertBefore(step.Body.Instructions[0],
                il.Create(OpCodes.Call, module.ImportReference(textures.Methods.First(m => m.Name == "Stepped"))));
            patched++;
            foreach (var ins in update.Body.Instructions)
            {
                if (!(ins.Operand is MethodReference mr) || mr.DeclaringType.FullName != "UnityEngine.Texture2D"
                    || (mr.Name != "LoadRawTextureData" && mr.Name != "Apply"))
                    continue;
                // The instance call becomes a static one taking the texture first.
                var helper = textures.Methods.FirstOrDefault(m => m.Name == mr.Name
                    && m.Parameters.Count == mr.Parameters.Count + 1
                    && m.Parameters.Skip(1).Select(x => x.ParameterType.FullName)
                        .SequenceEqual(mr.Parameters.Select(x => x.ParameterType.FullName)));
                if (helper == null)
                    throw new Exception("no OniMobile.SimTextures." + mr.Name + " for " + mr.FullName);
                ins.OpCode = OpCodes.Call;
                ins.Operand = module.ImportReference(helper);
                patched++;
            }
        }
        var effects = runtime.GetType("OniMobile.PostEffects");
        var wake = module.ImportReference(effects.Methods.First(m => m.Name == "Wake"));
        foreach (var names in new[] {
            new[] { "KAnimActivePostProcessingEffects", "EnableEffect", "currentActiveEffects", "SleepIfIdle" },
            new[] { "CustomActiveScreenPostProcessingEffects", "RegisterEffect", "ActiveEffects", "SleepIfEmpty" } })
        {
            string typeName = names[0], add = names[1], field = names[2], sleep = names[3];
            var type = module.GetType(typeName);
            var render = type?.Methods.FirstOrDefault(m => m.Name == "OnRenderImage");
            var adder = type?.Methods.FirstOrDefault(m => m.Name == add);
            var active = type?.Fields.FirstOrDefault(f => f.Name == field);
            if (render == null || adder == null || active == null)
                continue;
            var il = adder.Body.GetILProcessor();
            var first = adder.Body.Instructions[0];
            il.InsertBefore(first, il.Create(OpCodes.Ldarg_0));
            il.InsertBefore(first, il.Create(OpCodes.Call, wake));
            // Each ret becomes "this, this.field, call Sleep..., ret": branches to the old ret
            // land on the new first instruction.
            var sleepRef = module.ImportReference(effects.Methods.First(m => m.Name == sleep));
            il = render.Body.GetILProcessor();
            foreach (var ret in render.Body.Instructions.Where(i => i.OpCode == OpCodes.Ret).ToList())
            {
                ret.OpCode = OpCodes.Ldarg_0;
                ret.Operand = null;
                var load = il.Create(OpCodes.Ldarg_0);
                il.InsertAfter(ret, load);
                var get = il.Create(OpCodes.Ldfld, active);
                il.InsertAfter(load, get);
                var call = il.Create(OpCodes.Call, sleepRef);
                il.InsertAfter(get, call);
                il.InsertAfter(call, il.Create(OpCodes.Ret));
            }
            patched++;
        }
        return patched;
    }

    static int DefaultUIScale(ModuleDefinition module, ModuleDefinition runtime)
    {
        var init = module.GetType("KCanvasScaler")?.Methods.FirstOrDefault(m => m.Name == "OnPrefabInit");
        if (init == null)
            return 0;
        var scale = runtime.GetType("OniMobile.Platform").Methods.First(m => m.Name == "DefaultUIScale");
        int replaced = 0;
        foreach (var ins in init.Body.Instructions)
        {
            if (ins.OpCode == OpCodes.Ldc_R4 && (float)ins.Operand == 1f
                && ins.Next.Operand is MethodReference set && set.Name == "SetUserScale")
            {
                ins.OpCode = OpCodes.Call;
                ins.Operand = module.ImportReference(scale);
                replaced++;
            }
        }
        return replaced;
    }

    // [target] ldftn M; newobj D(object, native int)
    // becomes
    // [target] stloc t; ldloc t; brtrue make; ldnull; br end; make: ldloc t; ldftn M; newobj D; end:
    static int NullSafeDelegates(ModuleDefinition module)
    {
        int changed = 0;
        foreach (var type in module.GetTypes())
        {
            foreach (var method in type.Methods.Where(m => m.HasBody))
            {
                var sites = method.Body.Instructions.Where(ins => ins.OpCode == OpCodes.Ldftn
                    && ins.Operand is MethodReference target && target.HasThis
                    && ins.Next?.OpCode == OpCodes.Newobj
                    // `this` is never null.
                    && !(method.HasThis && ins.Previous?.OpCode == OpCodes.Ldarg_0)).ToList();
                if (sites.Count == 0)
                    continue;
                method.Body.SimplifyMacros();
                var temp = new VariableDefinition(module.TypeSystem.Object);
                method.Body.Variables.Add(temp);
                var il = method.Body.GetILProcessor();
                foreach (var ldftn in sites)
                {
                    var end = ldftn.Next.Next;
                    var make = il.Create(OpCodes.Ldloc, temp);
                    var store = il.Create(OpCodes.Stloc, temp);
                    // Jumps that landed on ldftn (target pushed on two paths) land on the store now.
                    foreach (var ins in method.Body.Instructions)
                    {
                        if (ins.Operand == ldftn)
                            ins.Operand = store;
                        else if (ins.Operand is Instruction[] targets)
                            for (int i = 0; i < targets.Length; i++)
                                if (targets[i] == ldftn)
                                    targets[i] = store;
                    }
                    il.InsertBefore(ldftn, store);
                    il.InsertBefore(ldftn, il.Create(OpCodes.Ldloc, temp));
                    il.InsertBefore(ldftn, il.Create(OpCodes.Brtrue, make));
                    il.InsertBefore(ldftn, il.Create(OpCodes.Ldnull));
                    il.InsertBefore(ldftn, il.Create(OpCodes.Br, end));
                    il.InsertBefore(ldftn, make);
                    changed++;
                }
                method.Body.OptimizeMacros();
            }
        }
        return changed;
    }

    static int QuietSteamError(ModuleDefinition module)
    {
        var init = module.GetType("DistributionPlatform")?.Methods.FirstOrDefault(m => m.Name == "Initialize");
        if (init == null)
            return 0;
        int changed = 0;
        foreach (var ins in init.Body.Instructions)
        {
            if (ins.OpCode == OpCodes.Ldstr && (string)ins.Operand == "Steam not initialized in time."
                && ins.Next.Operand is MethodReference log && log.Name == "LogError" && log.DeclaringType.Name == "Debug")
            {
                var warning = new MethodReference("LogWarning", log.ReturnType, log.DeclaringType);
                foreach (var p in log.Parameters)
                    warning.Parameters.Add(new ParameterDefinition(p.ParameterType));
                ins.Next.Operand = module.ImportReference(warning);
                changed++;
            }
        }
        return changed;
    }

    static readonly string[] ExtraIgnoreStrings =
    {
        "Could not find material Hidden/VideoDecodeAndroid. Make sure the Video shaders are included in your build, in the Built-in Shader Settings section of the Graphics Settings.",
        "Could not find video decode shader pass RGBAExternal_To_RGBA in shader <not found>",
        "Could not find video decode shader pass RGBASplitExternal_To_RGBA in shader <not found>",
    };

    // Grows the array the static constructor stores in KCrashReporter.IgnoreStrings.
    static int IgnoreErrors(ModuleDefinition module)
    {
        var cctor = module.GetType("KCrashReporter")?.Methods.FirstOrDefault(m => m.Name == ".cctor");
        if (cctor == null)
            return 0;
        var instructions = cctor.Body.Instructions;
        var store = instructions.First(i => i.OpCode == OpCodes.Stsfld && ((FieldReference)i.Operand).Name == "IgnoreStrings");
        var newarr = store.Previous;
        while (newarr.OpCode != OpCodes.Newarr)
            newarr = newarr.Previous;
        var size = newarr.Previous;
        int count = size.OpCode == OpCodes.Ldc_I4_S ? (sbyte)size.Operand : size.OpCode == OpCodes.Ldc_I4 ? (int)size.Operand : size.OpCode.Code - Code.Ldc_I4_0;
        size.OpCode = OpCodes.Ldc_I4;
        size.Operand = count + ExtraIgnoreStrings.Length;
        var il = cctor.Body.GetILProcessor();
        for (int i = 0; i < ExtraIgnoreStrings.Length; i++)
        {
            il.InsertBefore(store, il.Create(OpCodes.Dup));
            il.InsertBefore(store, il.Create(OpCodes.Ldc_I4, count + i));
            il.InsertBefore(store, il.Create(OpCodes.Ldstr, ExtraIgnoreStrings[i]));
            il.InsertBefore(store, il.Create(OpCodes.Stelem_Ref));
        }
        return ExtraIgnoreStrings.Length;
    }

    static int MarkCallbacks(ModuleDefinition module)
    {
        var delegates = new HashSet<string>();
        foreach (var type in module.GetTypes())
        {
            if (type.BaseType?.FullName == "System.MulticastDelegate"
                && type.CustomAttributes.Any(a => a.AttributeType.Name == "UnmanagedFunctionPointerAttribute"))
                delegates.Add(type.FullName);
            foreach (var method in type.Methods.Where(m => m.IsPInvokeImpl))
                foreach (var p in method.Parameters)
                    delegates.Add(p.ParameterType.FullName);
        }

        var targets = new HashSet<MethodDefinition>();
        var delegateOf = new Dictionary<MethodDefinition, TypeReference>();
        foreach (var type in module.GetTypes())
        {
            foreach (var method in type.Methods.Where(m => m.HasBody))
            {
                foreach (var ins in method.Body.Instructions)
                {
                    if (ins.OpCode != OpCodes.Ldftn || !(ins.Operand is MethodReference target) || !(ins.Next?.Operand is MethodReference ctor)
                        || ins.Next.OpCode != OpCodes.Newobj || !IsNativeDelegate(ctor.DeclaringType, delegates))
                        continue;
                    var def = target.Resolve();
                    if (def == null || !def.IsStatic || def.Module != module
                        || def.CustomAttributes.Any(a => a.AttributeType.Name == "MonoPInvokeCallbackAttribute"))
                        continue;
                    if (targets.Add(def))
                        delegateOf[def] = ctor.DeclaringType;
                }
            }
        }
        if (targets.Count == 0)
            return 0;

        var attributeCtor = InjectCallbackAttribute(module);
        var systemType = new TypeReference("System", "Type", module, module.TypeSystem.CoreLibrary);
        foreach (var def in targets)
        {
            var attribute = new CustomAttribute(attributeCtor);
            attribute.ConstructorArguments.Add(new CustomAttributeArgument(systemType, delegateOf[def]));
            def.CustomAttributes.Add(attribute);
        }
        return targets.Count;
    }

    // Delegate types declared in other assemblies (Steamworks' hooks) are recognized by
    // their [UnmanagedFunctionPointer].
    static bool IsNativeDelegate(TypeReference type, HashSet<string> delegates)
    {
        if (delegates.Contains(type.FullName))
            return true;
        TypeDefinition def;
        try { def = type.Resolve(); }
        catch (AssemblyResolutionException) { return false; }
        return def != null && def.CustomAttributes.Any(a => a.AttributeType.Name == "UnmanagedFunctionPointerAttribute");
    }

    // sealed class OniMobile.MonoPInvokeCallbackAttribute : Attribute { ctor(Type) }
    static MethodDefinition InjectCallbackAttribute(ModuleDefinition module)
    {
        var attributeBase = new TypeReference("System", "Attribute", module, module.TypeSystem.CoreLibrary);
        var systemType = new TypeReference("System", "Type", module, module.TypeSystem.CoreLibrary);
        var type = new TypeDefinition("OniMobile", "MonoPInvokeCallbackAttribute",
            TypeAttributes.Public | TypeAttributes.Sealed | TypeAttributes.BeforeFieldInit, attributeBase);
        var ctor = new MethodDefinition(".ctor",
            MethodAttributes.Public | MethodAttributes.HideBySig | MethodAttributes.SpecialName | MethodAttributes.RTSpecialName,
            module.TypeSystem.Void);
        ctor.Parameters.Add(new ParameterDefinition("type", ParameterAttributes.None, systemType));
        var baseCtor = new MethodReference(".ctor", module.TypeSystem.Void, attributeBase) { HasThis = true };
        var il = ctor.Body.GetILProcessor();
        il.Emit(OpCodes.Ldarg_0);
        il.Emit(OpCodes.Call, baseCtor);
        il.Emit(OpCodes.Ret);
        type.Methods.Add(ctor);
        module.Types.Add(type);
        return ctor;
    }
}
