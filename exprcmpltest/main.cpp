
#include "../exprcmpl/exprcmpl.h"
#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <fstream>

static const char* error_messages[] =
{
    "Unknown Error",
    "Invalid Input Variables",
    "Error compiling AST into machine code",
    "Error parsing expression into AST (syntax error)",
    "A larger output buffer is required",
    "Failed to compute immediate value (internal error)",
    "Found unknown identifier",
    "Variable is used like a function of vice-versa",
    "Argument count doesn't match the expected number",
    "Found unknown operand (internal error)",
    "Argument of a custom function is of an unsupported type",
    "Return type of a custom function is not supported"
};

int __stdcall IdentifierInfoCallback(const char* identifier, int identifierLen, Identifier* info)
{
    return 0;
}

int printErr(const char* name, int res)
{
    printf("%s => %d", name, res);
    if (res <= 0)
        printf(" (%s)\n", error_messages[-res]);
    else
        printf("\n");

    return 1;
}

int main(int argc, char** args)
{
    char s[1024+1];
    gets_s(s);

    void* expr;
    int res = ParseExpression(s, 1024, &expr);
    printErr("ParseExpression", res);
    if (res <= 0)
        return 1;

    res = PrintExpression(expr, s, 1024);
    printErr("PrintExpression", res);
    if (res <= 0)
        return 1;
    s[res] = 0;
    printf("%s\n", s);

    uint8 output[1024];
    res = CompileExpression(expr, output, 1024, IdentifierInfoCallback);
    printErr("CompileExpression", res);
    if (res <= 0)
        return 1;

    std::ofstream f("output.bin");
    f.write((char*)output, res);
    f.close();

    res = ReleaseExpression(expr);
    printErr("ReleaseExpression", res);
    if (res <= 0)
        return 1;

    return 0;
}
