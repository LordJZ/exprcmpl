#ifndef _VARIABLEEXPRESSION_H
#define _VARIABLEEXPRESSION_H

#include "util.h"
#include "Expression.h"

class VariableExpression : public Expression
{
public:
    VariableExpression(const char* name, int nameLen)
        : Expression()
    {
        m_identifier = name;
        m_identifierLen = nameLen;
    }

#ifdef _ENABLE_EXPR_TOSTRING
    virtual int ToString(char* str, int len) const
    {
        if (len < m_identifierLen)
            return 0;

        memcpy_s(str, len, m_identifier, m_identifierLen);

        return m_identifierLen;
    }
#endif

#ifdef _ENABLE_EXPR_EMIT
    virtual int Emit(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        Identifier ident;
        if (!identifierInfoCallback(m_identifier, m_identifierLen, &ident))
            return ERR_UNKNOWN_IDENTIFIER;

        switch (ident.Type)
        {
            case IDENTIFIER_INT32:
                // fild dword ptr [addr]
                if (!buf.append_8(0xDB) ||
                    !buf.append_8(0x05) ||
                    !buf.append_32(uint32(ident.ptr)))
                    return ERR_OUTPUT_BUFFER_TOO_SMALL;
                break;
            case IDENTIFIER_FLOAT32:
                // fld dword ptr [addr]
                if (!buf.append_8(0xD9) ||
                    !buf.append_8(0x05) ||
                    !buf.append_32(uint32(ident.ptr)))
                    return ERR_OUTPUT_BUFFER_TOO_SMALL;
                break;
            case IDENTIFIER_FLOAT64:
                // fld qword ptr [addr]
                if (!buf.append_8(0xDD) ||
                    !buf.append_8(0x05) ||
                    !buf.append_32(uint32(ident.ptr)))
                    return ERR_OUTPUT_BUFFER_TOO_SMALL;
                break;
            default:
                return ERR_IDENTIFIER_MISUSE;
        }

        return buf.pos();
    }

    virtual MarshallingInfo GetMarshallingInfo() const
    {
        MarshallingInfo info;
        info.Type = MARSHALLING_PTR;
        return info;
    }
#endif
};

#endif
