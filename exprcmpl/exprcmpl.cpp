
#include "exprcmpl.h"
#include "util.h"
#include "AstParser.h"

int __declspec(dllexport) __stdcall ParseExpression(const char* expr, int expr_len, void** exprPtr)
{
    if (!expr || expr_len <= 0 || !exprPtr)
        return ERR_INVALID_INPUT;

    AstParser parser(expr, expr_len);

    Expression* abstractExpression = parser.GetExpression();
    if (!abstractExpression)
        return ERR_PARSING_FAILED;

    *(Expression**)exprPtr = abstractExpression;

    return ERR_SUCCESS;
}

int __declspec(dllexport) __stdcall PrintExpression(const void* exprPtr, char* store, int store_len)
{
    if (!store || store_len <= 0 || !exprPtr)
        return ERR_INVALID_INPUT;

    return ((const Expression*)exprPtr)->ToString(store, store_len);
}

int __declspec(dllexport) __stdcall CompileExpression(const void* exprPtr, uint8* output, int output_len, pIdentifierInfoCallback identifierInfoCallback)
{
    if (!output || output_len <= 0 || !exprPtr || !identifierInfoCallback)
        return ERR_INVALID_INPUT;

    const Expression* abstractExpression = (const Expression*)exprPtr;

    ByteBuffer buf(output, output_len);
    int emitted = abstractExpression->Emit(buf, identifierInfoCallback);
    if (!emitted)
        return ERR_COMPILATION_FAILED;
    else if (emitted < 0)
        return emitted;

    if (!buf.append_8(0xC3))
        return ERR_OUTPUT_BUFFER_TOO_SMALL;

    return buf.pos();
}

int __declspec(dllexport) __stdcall ReleaseExpression(void* exprPtr)
{
    if (!exprPtr)
        return ERR_INVALID_INPUT;

    delete exprPtr;

    return 1;
}
