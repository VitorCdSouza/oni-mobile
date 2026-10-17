// Prints, for each assembly given on the command line, one public top-level
// non-generic type as a C# typeof() expression. Used to generate the player
// project's Keep.cs, so that the compiler emits a reference to every game
// assembly and Unity includes them in the IL2CPP build.
using System;
using System.Linq;
using Mono.Cecil;

static class PickTypes
{
    static int Main(string[] args)
    {
        foreach (var path in args)
        {
            var module = ModuleDefinition.ReadModule(path);
            var type = module.Types.FirstOrDefault(t => t.IsPublic && !t.HasGenericParameters && !t.IsNested && t.Name != "<Module>");
            if (type == null)
            {
                Console.WriteLine("            // no public type: " + module.Assembly.Name.Name);
                continue;
            }
            var name = string.IsNullOrEmpty(type.Namespace) ? type.Name : type.Namespace + "." + type.Name;
            Console.WriteLine("            typeof(global::" + name + "), // " + module.Assembly.Name.Name);
        }
        return 0;
    }
}
