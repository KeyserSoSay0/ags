#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "cc_compiledscript.h"
#include "script/script_common.h"   // macro definitions
#include "cc_symboltable.h"         // SymbolTable
#include "script/cc_options.h"      // ccGetOption
#include "script/cc_error.h"

void AGS::ccCompiledScript::WriteLineno(size_t lno)
{
    if (EmitLineNumbers)
        WriteCmd(SCMD_LINENUM, lno);
    LastEmittedLineno = lno;
}

void AGS::ccCompiledScript::push_reg(CodeCell regg)
{
    WriteCmd(SCMD_PUSHREG, regg);
    OffsetToLocalVarBlock += SIZE_OF_STACK_CELL;
}

void AGS::ccCompiledScript::pop_reg(CodeCell regg)
{
    WriteCmd(SCMD_POPREG, regg);
    OffsetToLocalVarBlock -= SIZE_OF_STACK_CELL;
}

AGS::ccCompiledScript::ccCompiledScript(bool emit_line_numbers)
{
    globaldata = NULL;
    globaldatasize = 0;
    code = NULL;
    codesize = 0;
    codeallocated = 0;
    strings = NULL;
    stringssize = 0;
    OffsetToLocalVarBlock = 0;
    fixups = NULL;
    fixuptypes = NULL;
    numfixups = 0;
    numimports = 0;
    numexports = 0;
    numSections = 0;
    importsCapacity = 0;
    imports = NULL;
    exportsCapacity = 0;
    exports = NULL;
    export_addr = NULL;
    capacitySections = 0;
    sectionNames = NULL;
    sectionOffsets = NULL;

    LastEmittedLineno = INT_MAX;
    EmitLineNumbers = emit_line_numbers;
    AX_Vartype = 0;
    AX_ScopeType = ScT::kGlobal;
    Functions = {};
    ImportIdx = {};

    EmitLineNumbers = emit_line_numbers;
}

AGS::ccCompiledScript::~ccCompiledScript()
{
    FreeExtra();
}

// [fw] Note: Existing callers expected this function to return < 0 on overflow
AGS::GlobalLoc AGS::ccCompiledScript::AddGlobal(size_t siz, void *vall)
{
    // The new global variable will be moved to &(globaldata[offset])
    GlobalLoc offset = globaldatasize;

    // Extend global data to make space for the new variable; 
    // note that this may change globaldata
    globaldatasize += siz;

    void *new_memoryspace = realloc(globaldata, globaldatasize);
    if (!new_memoryspace)
    {
        // Memory overflow. Note, STL says: "The old memory block is not freed"
        free((void *)globaldata);
        globaldata = nullptr;
        return -1;
    }
    globaldata = (char *)new_memoryspace;

    if (vall != NULL)
    {
        memcpy(&(globaldata[offset]), vall, siz); // move the global into the new space
    }
    else
    {
        memset(&(globaldata[offset]), 0, siz); // fill the new space with 0-bytes
    }

    return offset;
}

AGS::StringsLoc AGS::ccCompiledScript::AddString(std::string const &literal)
{
    // Note: processing  of '\\' and '[' combinations moved to the scanner
    // because the scanner must deal with '\\' anyway.
    size_t const literal_len = literal.size() + 1; // length including the terminating '\0'

    strings = (char *) realloc(strings, stringssize + literal_len);
    size_t const start_of_new_string = stringssize;

    memcpy(&strings[start_of_new_string], literal.c_str(), literal_len);
    stringssize += literal_len;
    return start_of_new_string;
}

void AGS::ccCompiledScript::AddFixup(CodeLoc where, FixupType ftype)
{
    fixuptypes = (char *) realloc(fixuptypes, numfixups + 5);
    fixups = static_cast<CodeLoc *>(realloc(
        fixups,
        (numfixups * sizeof(CodeLoc)) + 10));
    fixuptypes[numfixups] = ftype;
    fixups[numfixups] = where;
    numfixups++;
}

AGS::CodeLoc AGS::ccCompiledScript::AddNewFunction(std::string const &func_name, size_t num_of_parameters)
{
    FuncProps fp;
    fp.Name = func_name;
    fp.CodeOffs = codesize;
    fp.NumOfParams = num_of_parameters;
    Functions.push_back(fp);
    return codesize;
}

int AGS::ccCompiledScript::FindOrAddImport(std::string const &import_name)
{
    if (0u < ImportIdx.count(import_name))
        return ImportIdx[import_name];

    if (numimports >= importsCapacity)
    {
        importsCapacity += 1000;
        imports = static_cast<char **>(realloc(imports, sizeof(char *) * importsCapacity));
    }
    imports[numimports] = static_cast<char *>(malloc(import_name.size() + 12));
    strcpy(imports[numimports], import_name.c_str());
    return (ImportIdx[import_name] = numimports++);
}

int AGS::ccCompiledScript::AddExport(std::string const &name, CodeLoc location, size_t num_of_arguments)
{
    bool const is_function = (INT_MAX != num_of_arguments);
    // Exported functions get the number of parameters appended
    std::string const export_name =
        is_function ? name + "$" + std::to_string(num_of_arguments) : name;

    if (0u < ExportIdx.count(export_name))
        return ExportIdx[export_name];

    if (numexports >= exportsCapacity)
    {
        exportsCapacity += 1000;
        exports = static_cast<char **>(realloc(exports, sizeof(char *) * exportsCapacity));
        export_addr = static_cast<int32_t *>(realloc(export_addr, sizeof(int32_t) * exportsCapacity));
    }
    if (location >= 0x00ffffff)
    {
        cc_error("export offset too high; script data size too large?");
        return -1;
    }
    
    size_t const entry_size = export_name.size() + 1u;
    exports[numexports] = static_cast<char *>(malloc(entry_size));
    strncpy(exports[numexports], export_name.c_str(), entry_size);
    export_addr[numexports] =
        location |
        static_cast<CodeLoc>(is_function? EXPORT_FUNCTION : EXPORT_DATA) << 24L;
    return (ExportIdx[export_name] = numexports++);
}

void AGS::ccCompiledScript::WriteCode(CodeCell cell)
{
    if (codesize >= codeallocated - 2)
    {
        codeallocated += 500;
        code = static_cast<int32_t *>(realloc(code, codeallocated * sizeof(int32_t)));
    }
    code[codesize] = cell;
    codesize++;
}

std::string AGS::ccCompiledScript::start_new_section(std::string const &name)
{
    if ((numSections == 0) ||
        (codesize != sectionOffsets[numSections - 1]))
    {
        if (numSections >= capacitySections)
        {
            capacitySections += 100;
            sectionNames = static_cast<char **>(realloc(sectionNames, sizeof(char *) * capacitySections));
            sectionOffsets = static_cast<int32_t *>(realloc(sectionOffsets, sizeof(int32_t) * capacitySections));
        }
        sectionNames[numSections] = (char *)malloc(name.size() + 1);
        strcpy(sectionNames[numSections], name.c_str());
        sectionOffsets[numSections] = codesize;

        numSections++;
    }
    else
    {
        // nothing was in the last section, so overwrite it with this new one
        free(sectionNames[numSections - 1]);
        sectionNames[numSections - 1] = (char *)malloc(name.size() + 1);
        strcpy(sectionNames[numSections - 1], name.c_str());
    }

    return sectionNames[numSections - 1];
}

// free the extra bits that ccScript doesn't have
void AGS::ccCompiledScript::FreeExtra()
{
    Functions.clear();
    Functions.shrink_to_fit();
    ImportIdx.clear();
    ExportIdx.clear();
}
