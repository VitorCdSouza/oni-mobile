// Parses a C header and installs its structs in the program, replacing any existing type with the
// same name (including the empty placeholders Ghidra creates for C++ classes), so the decompiler
// shows field names. Run before ExportDecomp.java.
// Usage: analyzeHeadless <proj_dir> <proj_name> -process -postScript ApplyTypes.java <header.h>
import ghidra.app.script.GhidraScript;
import ghidra.app.util.cparser.C.CParser;
import ghidra.program.model.data.DataType;
import ghidra.program.model.data.DataTypeConflictHandler;
import ghidra.program.model.data.DataTypeManager;
import ghidra.program.model.data.PointerDataType;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.StandAloneDataTypeManager;
import ghidra.program.model.data.Structure;

import java.io.FileInputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class ApplyTypes extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String header = getScriptArgs()[0];
        StandAloneDataTypeManager parsed = new StandAloneDataTypeManager("layout");
        int tx = parsed.startTransaction("parse");
        try (FileInputStream in = new FileInputStream(header)) {
            new CParser(parsed, true, null).parse(in);
        }
        parsed.endTransaction(tx, true);

        DataTypeManager dtm = currentProgram.getDataTypeManager();
        int replaced = 0, added = 0;
        Iterator<Structure> it = parsed.getAllStructures();
        while (it.hasNext()) {
            Structure s = it.next();
            List<DataType> existing = new ArrayList<>();
            dtm.findDataTypes(s.getName(), existing);
            if (existing.isEmpty()) {
                dtm.addDataType(s, DataTypeConflictHandler.REPLACE_HANDLER);
                added++;
                continue;
            }
            for (DataType old : existing) {
                if (old instanceof Structure) {
                    ((Structure) old).replaceWith(s);
                    replaced++;
                }
            }
        }
        println("ApplyTypes: replaced " + replaced + ", added " + added);

        // "//@global <symbol> <type name> <is pointer>" lines type the global variables.
        int typedGlobals = 0;
        for (String line : Files.readAllLines(Paths.get(header))) {
            if (!line.startsWith("//@global ")) {
                continue;
            }
            String[] parts = line.trim().split("\\s+");
            List<DataType> found = new ArrayList<>();
            dtm.findDataTypes(parts[2], found);
            if (found.isEmpty()) {
                println("ApplyTypes: unknown type " + parts[2]);
                continue;
            }
            DataType type = found.get(0);
            if (parts[3].equals("1")) {
                type = new PointerDataType(type, dtm);
            }
            Symbol sym = getSymbol(parts[1], null);
            if (sym == null) {
                println("ApplyTypes: unknown global " + parts[1]);
                continue;
            }
            Address addr = sym.getAddress();
            clearListing(addr, addr.add(type.getLength() - 1));
            createData(addr, type);
            typedGlobals++;
        }
        println("ApplyTypes: typed " + typedGlobals + " globals");
    }
}
