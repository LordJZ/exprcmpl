#ifndef _CALLEXPRESSION_H
#define _CALLEXPRESSION_H

#include "util.h"
#include "Expression.h"

class CallExpression : public Expression
{
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

    int EmitCos(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fcos
            !buf.append_8(0xFF))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    int EmitAbs(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fabs
            !buf.append_8(0xE1))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    int EmitChs(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fchs
            !buf.append_8(0xE0))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    int EmitTan(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // ftan
            !buf.append_8(0xF2) ||
            !buf.append_8(0xD9) ||      // fincstp
            !buf.append_8(0xF7))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
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

    int EmitSqrt(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fsqrt
            !buf.append_8(0xFA))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    int EmitLog2(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        if (!buf.append_8(0xD9) ||      // fld1
            !buf.append_8(0xE8))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        EXIT_ON_ERR(m_args[0]->Emit(buf, identifierInfoCallback));

        if (!buf.append_8(0xD9) ||      // fyl2x
            !buf.append_8(0xF1))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    int EmitPi(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        if (!buf.append_8(0xD9) ||      // fldpi
            !buf.append_8(0xEB))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    template<int LEN>
    inline bool IsIdentName(const char (&Name)[LEN]) const
    {
        if (m_identifierLen != LEN-1)
            return false;

        for (int i = 0; i < LEN-1; ++i)
        {
            if (m_identifier[i] != Name[i])
                return false;
        }

        return true;
    }

    struct BuiltInFunct
    {
        const char* name;
        int argc;
        int(CallExpression::*handler)(ByteBuffer&, pIdentifierInfoCallback) const;
    };

    static const BuiltInFunct s_builtInFuncts[];

    const BuiltInFunct* GetBuiltInFunct(bool* isOverloaded) const
    {
        const BuiltInFunct* funct = s_builtInFuncts;
        while (funct->name)
        {
            if (!strcmp(funct->name, m_identifier))
            {
                if (isOverloaded)
                    *isOverloaded = true;

                if (m_argc == funct->argc)
                    return funct;

                break;
            }

            ++funct;
        }

        return NULL;
    }

public:
    virtual int Emit(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        // check built-in functions
        bool isBuiltInOverload = false;

        const BuiltInFunct* f = GetBuiltInFunct(&isBuiltInOverload);
        if (f)
            return (this->*f->handler)(buf, identifierInfoCallback);

        Identifier ident;
        if (!identifierInfoCallback(m_identifier, m_identifierLen, &ident))
            return !isBuiltInOverload ? ERR_UNKNOWN_IDENTIFIER : ERR_ARGC_DOESNT_MATCH;

        if (ident.Type != IDENTIFIER_FUNC)
            return ERR_IDENTIFIER_MISUSE;

        // check args
        EXIT_ON_ERR(CheckArgs(ident.func_argtypes));

        // Emit Code
        for (int i = 0; i < m_argc; ++i)
        {
            const Expression* expr = m_args[i];
            MarshallingInfo einfo = expr->GetMarshallingInfo();
            if (einfo.Type == MARSHALLING_IMM)
            {
                // push imm value to the stack
                switch (ident.func_argtypes[i])
                {
                    case IDENTIFIER_FLOAT64:
                        if (!buf.append_8(0x68) ||
                            !buf.append_32(*((uint32*)&einfo.Imm + 1)) ||
                            !buf.append_8(0x68) ||
                            !buf.append_32(*((uint32*)&einfo.Imm + 0)))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    case IDENTIFIER_FLOAT32:
                    {
                        float val = float(einfo.Imm);
                        if (!buf.append_8(0x68) ||
                            !buf.append_32(*(uint32*)&val))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    }
                    case IDENTIFIER_INT32:
                    {
                        int32 val = int32(einfo.Imm);
                        if (!buf.append_8(0x68) ||
                            !buf.append_32(*(uint32*)&val))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    }
                    default:
                        return ERR_ARG_TYPE_ERR;
                }
            }
            else
            {
                EXIT_ON_ERR(expr->Emit(buf, identifierInfoCallback));

                // push st0 to the stack and pop the stack
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
        MarshallingInfo info;

        info.Type = MARSHALLING_ST0;

        return info;
    }
#endif
};

const CallExpression::BuiltInFunct CallExpression::s_builtInFuncts[] =
{
    { "sin", 1, &CallExpression::EmitSin },
    { "cos", 1, &CallExpression::EmitCos },
    { "abs", 1, &CallExpression::EmitAbs },
    { "chs", 1, &CallExpression::EmitChs },
    { "tan", 1, &CallExpression::EmitTan },
    { "cot", 1, &CallExpression::EmitCot },
    { "sqrt", 1, &CallExpression::EmitSqrt },
    { "log2", 1, &CallExpression::EmitLog2 },
    { "pi", 0, &CallExpression::EmitPi },
    { NULL, 0, NULL }
};


#endif
