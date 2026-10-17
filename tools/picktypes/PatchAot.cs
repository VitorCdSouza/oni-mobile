// Rewrites game assemblies for IL2CPP (no JIT, no Reflection.Emit).
//
// Usage: PatchAot.exe <in Newtonsoft.Json.dll> <out Newtonsoft.Json.dll> <game Managed dir>
//
// Newtonsoft.Json 7.0.1 picks DynamicReflectionDelegateFactory (Reflection.Emit)
// when JsonTypeReflector.DynamicCodeGeneration is true, which it always is on
// a full framework. Forcing it to false makes it use the late-bound factory.
using System;
using System.Linq;
using Mono.Cecil;
using Mono.Cecil.Cil;

static class PatchAot
{
    static int Main(string[] args)
    {
        var module = ModuleDefinition.ReadModule(args[0], new ReaderParameters { AssemblyResolver = new NameResolver(args[2]) });
        var type = module.GetType("Newtonsoft.Json.Serialization.JsonTypeReflector");
        var getter = type?.Properties.FirstOrDefault(p => p.Name == "DynamicCodeGeneration")?.GetMethod;
        if (getter == null)
        {
            Console.Error.WriteLine("JsonTypeReflector.DynamicCodeGeneration not found");
            return 1;
        }
        var body = getter.Body;
        body.Instructions.Clear();
        body.ExceptionHandlers.Clear();
        body.Variables.Clear();
        var il = body.GetILProcessor();
        il.Emit(OpCodes.Ldc_I4_0);
        il.Emit(OpCodes.Ret);
        module.Write(args[1]);
        Console.WriteLine("patched " + getter.FullName);
        return 0;
    }
}
