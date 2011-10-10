#ifndef _ASTPARSER_H
#define _ASTPARSER_H

#include "util.h"
#include "Expression.h"
#include "VariableExpression.h"
#include "CallExpression.h"
#include "NumberExpression.h"
#include "BinaryExpression.h"

enum AstTokens
{
    AST_TOKEN_ERROR         = 0,
    AST_TOKEN_IDENTIFIER    = -1,
    AST_TOKEN_NUMBER        = -2,
    AST_TOKEN_EOF           = -3,
};

#ifndef EOF
# define EOF (-1)
#endif

class AstParser
{

public:
    AstParser(const char* str, int length)
        : m_input(str), m_inputLen(length), m_inputPos(0),
        m_currentToken(0), m_identifierLen(0), m_numericValue(0.0),
        m_lastChar(' ')
    {
        m_identifierStr[0] = 0;
    };

private:
    const char* m_input;
    int m_inputLen;
    int m_inputPos;

    int m_currentToken;
    char m_identifierStr[128];
    int m_identifierLen;
    double m_numericValue;

    int m_lastChar;

    inline int GetChar()
    {
        if (m_inputLen <= m_inputPos)
            return -1;

        return m_input[m_inputPos++];
    }

    inline int PeekChar()
    {
        if (m_inputLen <= m_inputPos)
            return -1;

        return m_input[m_inputPos];
    }

    int GetToken()
    {
        // Skip any whitespace.
        while (is_whitespace_char(m_lastChar))
            m_lastChar = GetChar();

        if (is_identifier_char(m_lastChar) && !is_digit_char(m_lastChar))
        {
            // identifier: [a-zA-Z][a-zA-Z0-9]*
            m_identifierLen = 0;
            m_identifierStr[m_identifierLen++] = char(m_lastChar);
            while (is_char(m_lastChar = GetChar()) && is_identifier_char(m_lastChar) && m_identifierLen < 128)
                m_identifierStr[m_identifierLen++] = char(m_lastChar);

            return AST_TOKEN_IDENTIFIER;
        }

        if (is_digit_char(m_lastChar) || (m_lastChar == '.' && is_digit_char(PeekChar())))
        {
            // Number: [0-9.]+
            m_numericValue = 0.0;
            bool places = false;
            double qplaces = 1.0;
            do
            {
                if (m_lastChar == '.')
                {
                    if (places)
                        return AST_TOKEN_ERROR;

                    places = true;
                }
                else if (is_digit_char(m_lastChar))
                {
                    if (places)
                    {
                        qplaces /= 10.0;
                        m_numericValue += double(m_lastChar - '0') * qplaces;
                    }
                    else
                    {
                        m_numericValue *= 10.0;
                        m_numericValue += double(m_lastChar - '0');
                    }
                }
                else
                    return AST_TOKEN_ERROR;

                m_lastChar = GetChar();

            } while (is_digit_char(m_lastChar) || m_lastChar == '.');

            return AST_TOKEN_NUMBER;
        }

        // Check for end of file.  Don't eat the EOF.
        if (m_lastChar == EOF)
            return AST_TOKEN_EOF;

        // Otherwise, just return the character as its ascii value.
        int ThisChar = m_lastChar;
        m_lastChar = GetChar();
        return ThisChar;
    }

    inline int GetNextToken()
    {
        return m_currentToken = GetToken();
    }

    /// identifierexpr
    ///   ::= identifier
    ///   ::= identifier '(' expression* ')'
    Expression* ParseIdentifierExpr()
    {
        char* IdName = new char[m_identifierLen+1];
        int IdLen = m_identifierLen;
        memcpy(IdName, m_identifierStr, IdLen);
        IdName[IdLen] = 0;

        GetNextToken();  // eat identifier.

        if (m_currentToken != '(') // Simple variable ref.
            return new VariableExpression(IdName, IdLen);

        // Call.
        GetNextToken();  // eat (
        const int maxArgs = 32;
        Expression** args = new Expression*[maxArgs];
        int nArg = 0;
        if (m_currentToken != ')')
        {
            while (nArg < maxArgs)
            {
                Expression* arg = ParseExpression();
                if (arg == NULL)
                {
                    delete[] IdName;
                    return NULL;
                }

                args[nArg++] = arg;

                if (m_currentToken == ')')
                    break;

                if (m_currentToken != ',')
                {
                    delete[] IdName;
                    return NULL;
                }

                GetNextToken();
            }
        }

        // Eat the ')'.
        GetNextToken();

        return new CallExpression(IdName, IdLen, args, nArg);
    }

    /// parenexpr ::= '(' expression ')'
    Expression* ParseParenExpr()
    {
        GetNextToken();  // eat (.
        Expression* V = ParseExpression();
        if (V == NULL)
            return NULL;

        if (m_currentToken != ')')
            return NULL;

        GetNextToken();  // eat ).
        return V;
    }

    /// primary
    ///   ::= identifierexpr
    ///   ::= numberexpr
    ///   ::= parenexpr
    Expression* ParsePrimary()
    {
        if (m_currentToken == AST_TOKEN_IDENTIFIER)
            return ParseIdentifierExpr();
        else if (m_currentToken == AST_TOKEN_NUMBER)
            return ParseNumberExpr();
        else if (m_currentToken == '(')
            return ParseParenExpr();
        else
            return NULL;
    }

    int GetTokPrecedence()
    {
        switch (m_currentToken)
        {
            case '+': return 20;
            case '-': return 20;
            case '*': return 40;
            case '/': return 40;
        }

        return -1;
    }

    /// numberexpr ::= number
    Expression* ParseNumberExpr()
    {
        Expression* Result = new NumberExpression(m_numericValue);
        GetNextToken(); // consume the number
        return Result;
    }

    /// binoprhs
    ///   ::= ('+' primary)*
    Expression* ParseBinOpRHS(int ExprPrec, Expression* LHS)
    {
        // If this is a binop, find its precedence.
        while (true)
        {
            int TokPrec = GetTokPrecedence();

            // If this is a binop that binds at least as tightly as the current binop,
            // consume it, otherwise we are done.
            if (TokPrec < ExprPrec)
                return LHS;

            // Okay, we know this is a binop.
            int BinOp = m_currentToken;
            GetNextToken();  // eat binop

            // Parse the primary expression after the binary operator.
            Expression* RHS = ParsePrimary();
            if (RHS == NULL)
                return NULL;

            // If BinOp binds less tightly with RHS than the operator after RHS, let
            // the pending operator take RHS as its LHS.
            int NextPrec = GetTokPrecedence();
            if (TokPrec < NextPrec)
            {
                RHS = ParseBinOpRHS(TokPrec + 1, RHS);
                if (RHS == NULL)
                    return NULL;
            }

            // Merge LHS/RHS.
            LHS = new BinaryExpression(BinOp, LHS, RHS);
        }
    }

    /// expression
    ///   ::= primary binoprhs
    ///
    Expression* ParseExpression()
    {
        Expression* LHS = ParsePrimary();
        if (LHS == NULL)
            return NULL;

        return ParseBinOpRHS(0, LHS);
    }
public:
    Expression* GetExpression()
    {
        GetNextToken();
        return ParseExpression();
    }

    int GetInputPos()
    {
        return m_inputPos;
    }
};

#endif
