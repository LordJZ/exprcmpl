#ifndef _NUMBEREXPRESSION_H
#define _NUMBEREXPRESSION_H

#include "util.h"
#include "Expression.h"

class NumberExpression : public Expression
{
public:
    NumberExpression(double value)
        : Expression()
    {
        m_value = value;
    }

#ifdef _ENABLE_EXPR_TOSTRING
    virtual int ToString(char* str, int len) const
    {
        return sprintf_s(str, len, "%.3f", m_value);
    }
#endif

#ifdef _ENABLE_EXPR_EMIT
    virtual int Emit(ByteBuffer& buf, pIdentifierInfoCallback callback) const
    {
        // Emitting the NumberExpression pushes the value onto the fpu stack

        // Simple Cases
        static const double values[] =
        {
            +1.0000000000000000,
            +3.3219280948873626,    // log2(10)
            M_LOG2E,                // log2(e)
            M_PI,
            +0.30102999566398114,   // log10(2)
            M_LN2,                  // ln(2)
            +0.0000000000000000,
        };

        static const uint16 opcodes[] =
        {
            0xE8D9,
            0xE9D9,
            0xEAD9,
            0xEBD9,
            0xECD9,
            0xEDD9,
            0xEED9,
        };

        STATIC_ASSERT(sizeof(values)/sizeof(values[0]) == sizeof(opcodes)/sizeof(opcodes[0]), "values count differs from opcodes count");

        for (int i = 0; i < sizeof(values)/sizeof(values[0]); ++i)
        {
            if (eqdbl(m_value, values[i]))
            {
                if (!buf.append_16(opcodes[i]))
                    return ERR_OUTPUT_BUFFER_TOO_SMALL;

                return buf.pos();
            }
        }

        uint32* value_parts = (uint32*)&m_value;

        if (!buf.append_8(0x68) ||                  // push imm32 (higher bits)
            !buf.append_32(value_parts[1]) ||
            !buf.append_8(0x68) ||                  // push imm32 (lower bits)
            !buf.append_32(value_parts[0]) ||
            !buf.append_8(0xDD) ||                  // fld qword ptr [esp]
            !buf.append_8(0x04) ||
            !buf.append_8(0x24) ||
            !buf.append_8(0x83) ||                  // add esp, 8
            !buf.append_8(0xC4) ||
            !buf.append_8(0x08))
            return ERR_OUTPUT_BUFFER_TOO_SMALL;

        return buf.pos();
    }

    virtual MarshallingInfo GetMarshallingInfo() const
    {
        MarshallingInfo ret;
        ret.Type = MARSHALLING_IMM;
        ret.Imm = m_value;
        return ret;
    }
#endif
};

#endif
