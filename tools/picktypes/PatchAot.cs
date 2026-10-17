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

// Resolves references by simple name from the game's Managed directory, whatever
// version they ask for: Newtonsoft.Json 7.0.1 still references mscorlib 2.0.
class NameResolver : DefaultAssemblyResolver
{
    readonly string dir;

    public NameResolver(string dir) { this.dir = dir; }

    public override AssemblyDefinition Resolve(AssemblyNameReference name)
    {
        var path = System.IO.Path.Combine(dir, name.Name + ".dll");
        if (System.IO.File.Exists(path))
            return AssemblyDefinition.ReadAssembly(path, new ReaderParameters { AssemblyResolver = this });
        return base.Resolve(name);
    }
}

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
