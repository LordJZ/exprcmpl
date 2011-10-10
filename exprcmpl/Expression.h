#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "util.h"
#include "exprcmpl.h"

#ifdef _ENABLE_EXPR_EMIT
# define EXIT_ON_ERR(...) { int tmp = __VA_ARGS__; if (tmp <= 0) return tmp; }
#endif

enum MarshallingType
{
    MARSHALLING_IMM = 0,        // immediate double value
    MARSHALLING_PTR,            // value in memory
    MARSHALLING_STI,            // 32-bit integer value ontop of the stack
    MARSHALLING_STF,            // 32-bit float value ontop of the stack
    MARSHALLING_STD,            // 64-bit float value ontop of the stack
    MARSHALLING_ST0,            // float value ontop of the x87 stack
    MARSHALLING_EAX,            // 32-bit integer value is in eax
};

struct MarshallingInfo
{
    MarshallingType Type;
    union
    {
        double  Imm;            // MARSHALLING_IMM
        //void*   Ptr;            // MARSHALLING_PR*
    };
};

class Expression
{
protected:
    Expression()
        : m_identifier(NULL), m_identifierLen(0),
        m_op(0), m_value(0.0),
        m_args(NULL), m_argc(0),
        m_lhs(NULL), m_rhs(NULL)
    {
    }

    const char* m_identifier;
    int m_identifierLen;

    char m_op;

    double m_value;

    Expression const* const* m_args;
    int m_argc;

    const Expression* m_lhs;
    const Expression* m_rhs;

public:
    virtual ~Expression()
    {
        if (m_identifier)
            delete[] m_identifier;

        if (m_args)
        {
            for (int i = 0; i < m_argc; ++i)
                delete m_args[i];

            delete[] m_args;
        }

        if (m_lhs)
            delete m_lhs;

        if (m_rhs)
            delete m_rhs;
    }

public:
#ifdef _ENABLE_EXPR_TOSTRING
    virtual int ToString(char* str, int len) const = 0;
#endif

#ifdef _ENABLE_EXPR_EMIT
    virtual int Emit(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const = 0;

    virtual MarshallingInfo GetMarshallingInfo() const = 0;
#endif
};


#endif
