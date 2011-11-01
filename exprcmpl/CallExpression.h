#ifndef _CALLEXPRESSION_H
#define _CALLEXPRESSION_H

#include "util.h"
#include "Expression.h"
#include "NumberExpression.h"

class CallExpression : public Expression
{
    struct BuiltInFunct
    {
        const char* name;
        int argc;
        int(CallExpression::*handler)(ByteBuffer&, pIdentifierInfoCallback) const;
        double(CallExpression::*folder)() const;
    };

    static const BuiltInFunct s_builtInFuncts[];

    const BuiltInFunct* m_builtInFunct;
    bool m_isBuiltInOverload;
    MarshallingInfo m_info;

public:
    CallExpression(const char* name, int nameLen, Expression const* const* args, int argc)
        : Expression()
    {
        char* ident = new char[nameLen+1];
        memcpy(ident, name, nameLen);
        ident[nameLen] = 0;
        m_identifier = ident;
        m_identifierLen = nameLen;

        m_args = args;
        m_argc = argc;

        m_builtInFunct = NULL;
        m_isBuiltInOverload = false;

        const BuiltInFunct* funct = s_builtInFuncts;
        while (funct->name)
        {
            if (!strcmp(funct->name, m_identifier))
            {
                m_isBuiltInOverload = true;

                if (m_argc == funct->argc)
                    m_builtInFunct = funct;

                break;
            }

            ++funct;
        }

        m_info.Type = MARSHALLING_ST0;

#ifdef _ENABLE_EXPR_FOLDING
        if (m_builtInFunct)
        {
            bool ok = true;
            for (int i = 0; i < m_argc; ++i)
            {
                if (m_args[i]->GetMarshallingInfo().Type != MARSHALLING_IMM)
                {
                    ok = false;
                    break;
                }
            }

            if (ok)
            {
                m_info.Type = MARSHALLING_IMM;
                m_info.Imm = (this->*m_builtInFunct->folder)();
            }
        }
#endif
    }

#ifdef _ENABLE_EXPR_TOSTRING
    virtual int ToString(char* str, int len) const
    {
        int OrigLen = len;

        if (len < m_identifierLen+1 + 2*m_argc + 1)
            return 0;

        memcpy_s(str, len, m_identifier, m_identifierLen);
        len -= m_identifierLen+1;
        str += m_identifierLen;
        str[0] = '(';
        str += 1;

        int l;
        for (int i = 0; i < m_argc; ++i)
        {
            l = m_args[i]->ToString(str, len);
            if (!l)
                goto ret;
            len -= l;
            str += l;

            if (i + 1 < m_argc)
            {
                l = sprintf_s(str, len, ", ");
                if (!l)
                    goto ret;
                str += l;
                len -= l;
            }
        }

        l = sprintf_s(str, len, ")");
        if (!l)
            goto ret;
        str += l;
        len -= l;

    ret:
        return OrigLen - len;
    }
#endif

#ifdef _ENABLE_EXPR_EMIT
private:
    int CheckArgs(const uint8* args) const
    {
        int argc = 0;
        while (true)
        {
            uint8 type = *++args;
            if (type == IDENTIFIER_NONE)
                break;
            else if (type == IDENTIFIER_FUNC)
                return ERR_ARG_TYPE_ERR;

            ++argc;
            if (argc > m_argc)
                return ERR_ARGC_DOESNT_MATCH;
        }

        if (argc != m_argc)
            return ERR_ARGC_DOESNT_MATCH;

        return 1;
    }

    int EmitSin(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fsin
            !buf.append_8(0xFE))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    double FoldSin() const
    {
        return sin(m_args[0]->GetMarshallingInfo().Imm);
    }

    int EmitCos(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fcos
            !buf.append_8(0xFF))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    double FoldCos() const
    {
        return cos(m_args[0]->GetMarshallingInfo().Imm);
    }

    int EmitAbs(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fabs
            !buf.append_8(0xE1))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    double FoldAbs() const
    {
        return fabs(m_args[0]->GetMarshallingInfo().Imm);
    }

    int EmitChs(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fchs
            !buf.append_8(0xE0))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    double FoldChs() const
    {
        return -m_args[0]->GetMarshallingInfo().Imm;
    }

    int EmitTan(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // ftan
            !buf.append_8(0xF2) ||
            !buf.append_8(0xDD) ||      // ffree st(0)
            !buf.append_8(0xC0))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    double FoldTan() const
    {
        return tan(m_args[0]->GetMarshallingInfo().Imm);
    }

    int EmitCot(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // ftan
            !buf.append_8(0xE0) ||
            !buf.append_8(0xDE) ||      // fdivrp
            !buf.append_8(0xF1))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    double FoldCot() const
    {
        return 1.0/tan(m_args[0]->GetMarshallingInfo().Imm);
    }

    int EmitSqrt(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fsqrt
            !buf.append_8(0xFA))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    double FoldSqrt() const
    {
        return sqrt(m_args[0]->GetMarshallingInfo().Imm);
    }

    int EmitPi(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        if (!buf.append_8(0xD9) ||      // fldpi
            !buf.append_8(0xEB))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    double FoldPi() const
    {
        return M_PI;
    }

public:
    virtual int Emit(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
#ifdef _ENABLE_EXPR_FOLDING
        // check if the call foldable
        if (m_info.Type == MARSHALLING_IMM)
            return NumberExpression(m_info.Imm).Emit(buf, identifierInfoCallback);
#endif

        // check built-in functions
        if (m_builtInFunct)
            return (this->*m_builtInFunct->handler)(buf, identifierInfoCallback);

        Identifier ident;
        if (!identifierInfoCallback(m_identifier, m_identifierLen, &ident))
            return !m_isBuiltInOverload ? ERR_UNKNOWN_IDENTIFIER : ERR_ARGC_DOESNT_MATCH;

        if (ident.Type != IDENTIFIER_FUNC)
            return ERR_IDENTIFIER_MISUSE;

        // check args
        EXIT_ON_ERR(CheckArgs(ident.func_argtypes));

        // Emit Code
        for (int i = 0; i < m_argc; ++i)
        {
            const Expression* expr = m_args[i];
            MarshallingInfo einfo = expr->GetMarshallingInfo();
#ifdef _ENABLE_EXPR_FOLDING
            if (einfo.Type == MARSHALLING_IMM)
            {
                // push imm value to the stack
                switch (ident.func_argtypes[i])
                {
                    case IDENTIFIER_FLOAT64:
                        if (!buf.append_8(0x68) ||  // push imm32
                            !buf.append_32(*((uint32*)&einfo.Imm + 1)) ||
                            !buf.append_8(0x68) ||  // push imm32
                            !buf.append_32(*((uint32*)&einfo.Imm + 0)))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    case IDENTIFIER_FLOAT32:
                    {
                        float val = float(einfo.Imm);
                        if (!buf.append_8(0x68) ||  // push imm32
                            !buf.append_32(*(uint32*)&val))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    }
                    case IDENTIFIER_INT32:
                    {
                        int32 val = int32(einfo.Imm);
                        if (!buf.append_8(0x68) ||  // push imm32
                            !buf.append_32(*(uint32*)&val))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    }
                    default:
                        return ERR_ARG_TYPE_ERR;
                }
            }
            else
#endif
            {
                EXIT_ON_ERR(expr->Emit(buf, identifierInfoCallback));

                // push st0 to the stack and pop the x87 stack
                switch (ident.func_argtypes[i])
                {
                    case IDENTIFIER_FLOAT64:
                        if (!buf.append_8(0x50) ||  // push eax
                            !buf.append_8(0x50) ||  // push eax
                            !buf.append_8(0xDD) ||  // fstp qword ptr [esp]
                            !buf.append_8(0x1C) ||
                            !buf.append_8(0x24))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    case IDENTIFIER_FLOAT32:
                        if (!buf.append_8(0x50) ||  // push eax
                            !buf.append_8(0xD9) ||  // fstp dword ptr [esp]
                            !buf.append_8(0x1C) ||
                            !buf.append_8(0x24))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    case IDENTIFIER_INT32:
                        if (!buf.append_8(0x50) ||  // push eax
                            !buf.append_8(0xDB) ||  // fistp dword ptr [esp]
                            !buf.append_8(0x1C) ||
                            !buf.append_8(0x24))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    default:
                        return ERR_ARG_TYPE_ERR;
                }
            }

            if (!buf.append_8(0xB8) ||              // mov eax, imm dword
                !buf.append_32(uint32(ident.ptr)) ||
                !buf.append_8(0xFF) ||              // call eax
                !buf.append_8(0xD0))
                return ERR_OUTPUT_BUFFER_TOO_SMALL;

            switch (ident.func_rtype)
            {
                case IDENTIFIER_INT32:
                    if (!buf.append_8(0x50) ||      // push eax
                        !buf.append_8(0xDB) ||      // fild dword ptr [esp]
                        !buf.append_8(0x04) ||
                        !buf.append_8(0x24) ||
                        !buf.append_8(0x58))        // pop eax
                        return ERR_OUTPUT_BUFFER_TOO_SMALL;
                    break;
                case IDENTIFIER_FLOAT32:
                case IDENTIFIER_FLOAT64:
                    // value already in st0
                    break;
                default:
                    return ERR_RET_TYPE_ERR;
            }
        }

        return buf.pos();
    }

    virtual MarshallingInfo GetMarshallingInfo() const
    {
        return m_info;
    }

    virtual int GetExpressionTreeLength() const
    {
        int ret = 1;

        for (int i = 0; i < m_argc; ++i)
            ret += m_args[i]->GetExpressionTreeLength();

        return ret;
    }
#endif
};

const CallExpression::BuiltInFunct CallExpression::s_builtInFuncts[] =
{
    { "sin", 1, &CallExpression::EmitSin, &CallExpression::FoldSin },
    { "cos", 1, &CallExpression::EmitCos, &CallExpression::FoldCos },
    { "abs", 1, &CallExpression::EmitAbs, &CallExpression::FoldAbs },
    { "chs", 1, &CallExpression::EmitChs, &CallExpression::FoldChs },
    { "tan", 1, &CallExpression::EmitTan, &CallExpression::FoldTan },
    { "cot", 1, &CallExpression::EmitCot, &CallExpression::FoldCot },
    { "sqrt", 1, &CallExpression::EmitSqrt, &CallExpression::FoldSqrt },
    { "pi", 0, &CallExpression::EmitPi, &CallExpression::FoldPi },
    { NULL, 0, NULL, NULL }
};

#endif
