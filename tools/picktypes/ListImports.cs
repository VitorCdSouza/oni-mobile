// Prints the entry points an assembly imports from one native library.
//
// Usage: ListImports.exe <assembly.dll> <game Managed dir> <library name>...
//
// Used to generate do-nothing native stubs (tools/stubs) for libraries that do
// not exist on Android, such as Steam's.
using System;
using System.Linq;
using Mono.Cecil;

static class ListImports
{
    static int Main(string[] args)
    {
        var module = ModuleDefinition.ReadModule(args[0], new ReaderParameters { AssemblyResolver = new NameResolver(args[1]) });
        var libraries = args.Skip(2).ToArray();
        var names = module.GetTypes()
            .SelectMany(t => t.Methods)
            .Where(m => m.HasPInvokeInfo && libraries.Contains(m.PInvokeInfo.Module.Name))
            .Select(m => string.IsNullOrEmpty(m.PInvokeInfo.EntryPoint) ? m.Name : m.PInvokeInfo.EntryPoint)
            .Distinct()
            .OrderBy(n => n, StringComparer.Ordinal);
        foreach (var name in names)
            Console.WriteLine(name);
        return 0;
    }
}
