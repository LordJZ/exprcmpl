
#include "exprcmpl.h"
#include "util.h"
#include "AstParser.h"

int __stdcall CompileExpression(const char* expr, int expr_len, uint8* output, int output_len, pIdentifierInfoCallback identifierInfoCallback)
{
    if (!expr || expr_len <= 0 || !output || output_len <= 0 || !identifierInfoCallback)
        return ERR_INVALID_INPUT;

    AstParser parser(expr, expr_len);

    Expression* abstractExpression = parser.GetExpression();
    if (!abstractExpression)
        return ERR_PARSING_FAILED;

    //{
    //    char str[1024];
    //    int l = abstractExpression->ToString(str, 1024);
    //    printf("%s\n", str);
    //}

    ByteBuffer buf(output, output_len);
    int emitted = abstractExpression->Emit(buf, identifierInfoCallback);
    delete abstractExpression;
    if (!emitted)
        return ERR_COMPILATION_FAILED;
    else if (emitted < 0)
        return emitted;

    if (!buf.append_8(0xC3))
        return ERR_OUTPUT_BUFFER_TOO_SMALL;

    return buf.pos();
}
