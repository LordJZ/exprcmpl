#ifndef _EXPRCMPL_H
#define _EXPRCMPL_H

// Expression Compiler

#if COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1600      // Maybe other compilers support this too
# define STATIC_ASSERT(EXPR, ERROR) static_assert(EXPR, ERROR)
# define CHECK_SIZE(TYPE, SIZE) STATIC_ASSERT(sizeof(TYPE) == SIZE, "Incorrect size of '" #TYPE "'.")
#else
# define STATIC_ASSERT(EXPR, ERROR)
# define CHECK_SIZE(TYPE, SIZE)
#endif

typedef signed char         int8;
typedef unsigned char       uint8;
typedef signed short        int16;
typedef unsigned short      uint16;
typedef signed int          int32;
typedef unsigned int        uint32;

CHECK_SIZE(int8, 1);
CHECK_SIZE(uint8, 1);
CHECK_SIZE(int16, 2);
CHECK_SIZE(uint16, 2);
CHECK_SIZE(int32, 4);
CHECK_SIZE(uint32, 4);

enum IdentifierType
{
    IDENTIFIER_NONE     = 0,
    IDENTIFIER_INT32    = 1,
    IDENTIFIER_FLOAT32  = 2,
    IDENTIFIER_FLOAT64  = 3,
    IDENTIFIER_FUNC     = 4,
};

enum Error
{
    ERR_SUCCESS                 =  1,
    ERR_UNKNOWN                 =  0,
    ERR_INVALID_INPUT           = -1,       // Input variables are off
    ERR_COMPILATION_FAILED      = -2,       // Error compiling AST into machine code
    ERR_PARSING_FAILED          = -3,       // Error parsing expression into AST (syntax error)
    ERR_OUTPUT_BUFFER_TOO_SMALL = -4,       // A larget output buffer is required
    ERR_IMM_BINARY_COMPUTE_ERR  = -5,       // [Internal Error]
    ERR_UNKNOWN_IDENTIFIER      = -6,       // Found unknown identifier
    ERR_IDENTIFIER_MISUSE       = -7,       // Variable is used like a function of vice-versa
    ERR_ARGC_DOESNT_MATCH       = -8,       // Number of passed arguments to a function is off
    ERR_UNKNOWN_OPERAND         = -9,       // [Internal Error]
    ERR_ARG_TYPE_ERR            =-10,       // Argument of a func is of an unsupported type
    ERR_RET_TYPE_ERR            =-11,       // Return type of a func is not supported
    // other errors
};

#pragma pack(push, 1)

struct Identifier
{
    uint8 Type;
    uint8        func_rtype;        // IdentifierType enum
    void*        ptr;               // ptr to imm value or function
    const uint8* func_argtypes;     // 0-terminated array of IdentifierType enum
};

#pragma pack(pop)

CHECK_SIZE(Identifier, 1+1+4+4);

typedef int(__stdcall *pIdentifierInfoCallback)(const char* identifier, int identifierLen, Identifier* info);

extern "C"
{
    // Parses an expression into AST.
    // Args:
    //  expr: pointer to expression in ASCII
    //  expr_len: length of expr in bytes
    //  exprPtr: pointer to pointer to parsed expression.
    //           set if returned value is 1
    //
    // Returns:
    //   1 = OK
    // <=0 = error
    int __declspec(dllexport) __stdcall ParseExpression(const char* expr, int expr_len, void** exprPtr);

    // Prints the parsed expression.
    // Args:
    //  exprPtr: pointer to parsed expression
    //  store: pointer to an array of bytes
    //  store_len: length of store in bytes
    //
    // Returns:
    //  >0 = number of stored ASCII characters
    // <=0 = error
    int __declspec(dllexport) __stdcall PrintExpression(const void* exprPtr, char* store, int store_len);

    // Compiles the parsed expression into 80x86 machine code.
    // Args:
    //  exprPtr: pointer to parsed expression
    //  output: pointer to an array of bytes
    //  output_length: length of output in bytes
    //  identifierInfoCallback: pointer to callback function
    //
    // Returns:
    //  >0 = number of emitted bytes
    // <=0 = error
    int __declspec(dllexport) __stdcall CompileExpression(const void* exprPtr, uint8* output, int output_length, pIdentifierInfoCallback identifierInfoCallback);

    // Releases the parsed expression.
    // Args:
    //  expr: pointer to parsed expression
    //
    // Returns:
    //   1 = OK
    // <=0 = error
    int __declspec(dllexport) __stdcall ReleaseExpression(void* exprPtr);
}

#endif
