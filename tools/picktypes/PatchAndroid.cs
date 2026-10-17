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
using System;
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
