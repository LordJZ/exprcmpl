
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

int IdentifierInfoCallback(const char* identifier, int identifierLen, Identifier* info)
{
    return 0;
}

int main(int argc, char** args)
{
    char s[1024];
    std::cin >> s;

    uint8 output[1024];

    int res = CompileExpression(s, 1024, output, 1024, IdentifierInfoCallback);
    printf("Result Id = %d", res);
    if (res > 0)
        printf(" bytes emitted\n");
    else
        printf(" (%s)", error_messages[-res]);

    if (res > 0)
    {
        std::ofstream f("output.bin");
        f.write((char*)output, res);
        f.close();
    }

    return 0;
}
