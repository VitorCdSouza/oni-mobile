// Decompiles every function in the current program and writes one C-like file per function,
// plus an index with address, size and demangled name.
// Usage: analyzeHeadless <proj_dir> <proj_name> -import <bin> -postScript ExportDecomp.java <out_dir>
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;

public class ExportDecomp extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        File outDir = new File(args.length > 0 ? args[0] : "decomp");
        File funcDir = new File(outDir, "functions");
        funcDir.mkdirs();

        DecompInterface decomp = new DecompInterface();
        DecompileOptions options = new DecompileOptions();
        decomp.setOptions(options);
        decomp.openProgram(currentProgram);

        try (PrintWriter index = new PrintWriter(new FileWriter(new File(outDir, "index.tsv")))) {
            index.println("address\tsize\texternal\tname");
            FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
            while (it.hasNext() && !monitor.isCancelled()) {
                Function f = it.next();
                long size = f.getBody().getNumAddresses();
                String addr = f.getEntryPoint().toString();
                boolean external = f.isExternal() || f.isThunk();
                index.printf("%s\t%d\t%s\t%s%n", addr, size, external, f.getName(true));
                if (external) {
                    continue;
                }
                DecompileResults res = decomp.decompileFunction(f, 120, monitor);
                String code = res.decompileCompleted()
                        ? res.getDecompiledFunction().getC()
                        : "// decompile failed: " + res.getErrorMessage() + "\n";
                try (PrintWriter w = new PrintWriter(new FileWriter(new File(funcDir, addr + ".c")))) {
                    w.println("// " + f.getName(true));
                    w.print(code);
                }
            }
        }
        decomp.dispose();
    }
}
