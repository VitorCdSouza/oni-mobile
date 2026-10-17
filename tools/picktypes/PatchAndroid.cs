// Patches a game assembly for the Android player.
//
// Usage: PatchAndroid.exe <in.dll> <out.dll> <game Managed dir>
//
// 1. StreamingAssets.
// On Android Application.streamingAssetsPath points inside the APK, which
// System.IO cannot read, and the game reads its YAML, codex and bundles with
// System.IO. The files are pushed to <persistentDataPath>/StreamingAssets
// instead, and every read of Application.streamingAssetsPath (and FMOD's
// hardcoded "file:///android_asset") goes through an injected helper that
// returns that directory on Android and the usual path elsewhere.
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
using System;
using System.Collections.Generic;
using System.Linq;
using Mono.Cecil;
using Mono.Cecil.Cil;

static class PatchAndroid
{
    const int RuntimePlatformAndroid = 11;

    static int Main(string[] args)
    {
        var module = ModuleDefinition.ReadModule(args[0], new ReaderParameters { AssemblyResolver = new NameResolver(args[2]) });
        var quits = DropSteamQuit(module);
        var quiet = QuietSteamError(module);
        var ignored = IgnoreErrors(module);
        var callbacks = MarkCallbacks(module);
        Console.WriteLine($"{module.Assembly.Name.Name}: {quiet} Steam errors made warnings, {ignored} crash screen messages ignored, {callbacks} native callbacks marked");
        var getter = module.GetMemberReferences().OfType<MethodReference>()
            .FirstOrDefault(m => m.Name == "get_streamingAssetsPath" && m.DeclaringType.FullName == "UnityEngine.Application");
        if (getter == null)
        {
            module.Write(args[1]);
            Console.WriteLine($"{module.Assembly.Name.Name}: {quits} Steam quit calls dropped");
            return 0;
        }
        var helper = InjectHelper(module, getter);

        int calls = 0, literals = 0;
        foreach (var type in module.GetTypes())
        {
            foreach (var method in type.Methods.Where(m => m.HasBody && m != helper))
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
                    else if (ins.OpCode == OpCodes.Ldstr && (string)ins.Operand == "file:///android_asset")
                    {
                        ins.OpCode = OpCodes.Call;
                        ins.Operand = helper;
                        literals++;
                    }
                }
            }
        }
        module.Write(args[1]);
        Console.WriteLine($"{module.Assembly.Name.Name}: {calls} streamingAssetsPath reads, {literals} android_asset literals redirected, {quits} Steam quit calls dropped");
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
                        || ins.Next.OpCode != OpCodes.Newobj || !delegates.Contains(ctor.DeclaringType.FullName))
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

    // static string OniMobile.Paths::StreamingAssets()
    // {
    //     if (Application.platform == RuntimePlatform.Android)
    //         return Application.persistentDataPath + "/StreamingAssets";
    //     return Application.streamingAssetsPath;
    // }
    static MethodDefinition InjectHelper(ModuleDefinition module, MethodReference streamingAssetsPath)
    {
        var application = streamingAssetsPath.DeclaringType;
        var platformType = new TypeReference("UnityEngine", "RuntimePlatform", module, application.Scope, true);
        var getPlatform = new MethodReference("get_platform", platformType, application);
        var getPersistent = new MethodReference("get_persistentDataPath", module.TypeSystem.String, application);
        var concat = new MethodReference("Concat", module.TypeSystem.String, module.TypeSystem.String);
        concat.Parameters.Add(new ParameterDefinition(module.TypeSystem.String));
        concat.Parameters.Add(new ParameterDefinition(module.TypeSystem.String));

        var paths = new TypeDefinition("OniMobile", "Paths",
            TypeAttributes.Public | TypeAttributes.Abstract | TypeAttributes.Sealed | TypeAttributes.BeforeFieldInit,
            module.TypeSystem.Object);
        var method = new MethodDefinition("StreamingAssets",
            MethodAttributes.Public | MethodAttributes.Static | MethodAttributes.HideBySig, module.TypeSystem.String);
        var il = method.Body.GetILProcessor();
        var elsewhere = il.Create(OpCodes.Call, streamingAssetsPath);
        il.Emit(OpCodes.Call, getPlatform);
        il.Emit(OpCodes.Ldc_I4, RuntimePlatformAndroid);
        il.Emit(OpCodes.Bne_Un, elsewhere);
        il.Emit(OpCodes.Call, getPersistent);
        il.Emit(OpCodes.Ldstr, "/StreamingAssets");
        il.Emit(OpCodes.Call, concat);
        il.Emit(OpCodes.Ret);
        il.Append(elsewhere);
        il.Emit(OpCodes.Ret);
        paths.Methods.Add(method);
        module.Types.Add(paths);
        return method;
    }
}
