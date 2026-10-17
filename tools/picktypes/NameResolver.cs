using Mono.Cecil;

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
