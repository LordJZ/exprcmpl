#ifndef _BINARYEXPRESSION_H
#define _BINARYEXPRESSION_H

#include "util.h"
#include "Expression.h"

class BinaryExpression : public Expression
{
public:
    BinaryExpression(char op, Expression* left, Expression* right)
        : Expression()
    {
        m_op = op;
        m_lhs = left;
        m_rhs = right;
    }

#ifdef _ENABLE_EXPR_TOSTRING
    virtual int ToString(char* str, int len) const
    {
        int OrigLen = len;
        int l;

        l = sprintf_s(str, len, "(");
        if (!l)
            return 0;
        str += l;
        len -= l;

        l = m_lhs->ToString(str, len);
        if (!l)
            return 0;
        len -= l;
        str += l;

        l = sprintf_s(str, len, " %c ", m_op);
        if (!l)
            return 0;
        len -= l;
        str += l;

        l = m_rhs->ToString(str, len);
        if (!l)
            return 0;
        len -= l;
        str += l;

        l = sprintf_s(str, len, ")");
        if (!l)
            return 0;
        str += l;
        len -= l;

        return OrigLen - len;
    }

    virtual int Emit(ByteBuffer& buf, pIdentifierInfoCallback identifierInfoCallback) const
    {
        MarshallingInfo linfo = m_lhs->GetMarshallingInfo();
        MarshallingInfo rinfo = m_rhs->GetMarshallingInfo();

#ifdef _ENABLE_EXPR_FOLDING
        if (linfo.Type == MARSHALLING_IMM &&
            rinfo.Type == MARSHALLING_IMM)
        {
            // Fold the constant, push the result onto the fpu stack.
            double result;
            if (compute(result))
                return NumberExpression(result).Emit(buf, identifierInfoCallback);

            return ERR_IMM_BINARY_COMPUTE_ERR;
        }
        else
#endif
        {
            if (m_rhs->GetExpressionTreeLength() > m_lhs->GetExpressionTreeLength())
            {
                EXIT_ON_ERR(m_rhs->Emit(buf, identifierInfoCallback));
                EXIT_ON_ERR(m_lhs->Emit(buf, identifierInfoCallback));

                switch (m_op)
                {
                    case '+':
                    case '*':
                    default:
                        goto _operation;
                    case '-':
                        if (!buf.append_16(0xE1DE)) // fsubrp
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    case '/':
                        if (!buf.append_16(0xF1DE)) // fdivrp
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                }
            }
            else
            {
                EXIT_ON_ERR(m_lhs->Emit(buf, identifierInfoCallback));
                EXIT_ON_ERR(m_rhs->Emit(buf, identifierInfoCallback));

                // Operation
            _operation:
                switch (m_op)
                {
                    case '+':
                        if (!buf.append_16(0xC1DE))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    case '-':
                        if (!buf.append_16(0xE9DE))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    case '*':
                        if (!buf.append_16(0xC9DE))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    case '/':
                        if (!buf.append_16(0xF9DE))
                            return ERR_OUTPUT_BUFFER_TOO_SMALL;
                        break;
                    default:
                        // Must never happen
                        return ERR_UNKNOWN_OPERAND;
                }
            }
        }

        return buf.pos();
    }

#ifdef _ENABLE_EXPR_FOLDING
    bool compute(double& result) const
    {
        double one = m_lhs->GetMarshallingInfo().Imm;
        double two = m_rhs->GetMarshallingInfo().Imm;
        switch (m_op)
        {
            case '+': result = one + two; return true;
            case '-': result = one - two; return true;
            case '*': result = one * two; return true;
            case '/': result = one / two; return true;
            default:
                // Must never happen
                return false;
        }
    }
#endif

    virtual MarshallingInfo GetMarshallingInfo() const
    {
        MarshallingInfo info;
        MarshallingInfo linfo = m_lhs->GetMarshallingInfo();
        MarshallingInfo rinfo = m_rhs->GetMarshallingInfo();

#ifdef _ENABLE_EXPR_FOLDING
        if (linfo.Type == MARSHALLING_IMM &&
            rinfo.Type == MARSHALLING_IMM)
        {
            // Fold the constant
            info.Type = MARSHALLING_IMM;
            compute(info.Imm);
        }
        else
#endif
        {
            info.Type = MARSHALLING_ST0;
        }

        return info;
    }

    virtual int GetExpressionTreeLength() const
    {
        return m_lhs->GetExpressionTreeLength() + m_rhs->GetExpressionTreeLength();
    }
#endif
};

#endif
