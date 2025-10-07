#include <cctype>
#include <sstream>
#include <stdexcept>

#include "parser.h"

Parser::Parser(SymbolTable& symbols) : symbols(symbols), current_token(0)
{
}

bool Parser::is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool Parser::is_identifier_char(char c)
{
    return std::isalnum(c) || c == '_' || c == '-' || c == '+' || c == '*' || c == '/' || c == '?' || c == '=';
}

std::vector<Token> Parser::tokenize(const std::string& input)
{
    std::vector<Token> result;
    size_t i = 0;

    while (i < input.length())
    {
        // Skip whitespace
        if (is_whitespace(input[i]))
        {
            i++;
            continue;
        }

        // Left parenthesis
        if (input[i] == '(')
        {
            result.push_back(Token(TokenType::LPAREN, "(", i));
            i++;
            continue;
        }

        // Right parenthesis
        if (input[i] == ')')
        {
            result.push_back(Token(TokenType::RPAREN, ")", i));
            i++;
            continue;
        }

        // Identifier (operator or variable)
        if (is_identifier_char(input[i]))
        {
            size_t start = i;
            std::string identifier;

            while (i < input.length() && is_identifier_char(input[i]))
            {
                identifier += input[i];
                i++;
            }

            result.push_back(Token(TokenType::IDENTIFIER, identifier, start));
            continue;
        }

        // Unknown character
        std::ostringstream oss;
        oss << "Unexpected character '" << input[i] << "' at position " << i;
        throw std::runtime_error(oss.str());
    }

    result.push_back(Token(TokenType::END_OF_INPUT, "", input.length()));
    return result;
}

Token& Parser::peek()
{
    return tokens[current_token];
}

Token& Parser::advance()
{
    if (!at_end())
    {
        current_token++;
    }
    return tokens[current_token - 1];
}

bool Parser::at_end()
{
    return peek().type == TokenType::END_OF_INPUT;
}

void Parser::expect(TokenType type, const std::string& message)
{
    if (peek().type != type)
    {
        throw std::runtime_error(message);
    }
}

std::shared_ptr<Expr> Parser::parse_expr()
{
    Token& token = peek();

    // Variable: ?identifier
    if (token.type == TokenType::IDENTIFIER && token.value[0] == '?')
    {
        advance();
        if (token.value.length() == 1)
        {
            throw std::runtime_error("Variable name cannot be empty after '?'");
        }
        // Remove the '?' prefix and intern the variable name
        std::string var_name = token.value.substr(1);
        Symbol var_sym = symbols.intern(var_name);
        return Expr::make_variable(var_sym);
    }

    // Operator: (op arg1 arg2 ...)
    if (token.type == TokenType::LPAREN)
    {
        advance(); // consume '('

        // Expect operator name
        expect(TokenType::IDENTIFIER, "Expected operator name after '('");
        Token& op_token = advance();

        // Check if it's a variable (starts with ?)
        if (op_token.value[0] == '?')
        {
            throw std::runtime_error("Operator name cannot start with '?'");
        }

        Symbol op_sym = symbols.intern(op_token.value);

        // Parse arguments
        Vec<std::shared_ptr<Expr>> children;
        while (peek().type != TokenType::RPAREN && !at_end())
        {
            children.push_back(parse_expr());
        }

        expect(TokenType::RPAREN, "Expected ')' to close expression");
        advance(); // consume ')'

        if (children.empty())
        {
            // Nullary operator
            return Expr::make_operator(op_sym);
        }
        else
        {
            // N-ary operator
            return Expr::make_operator(op_sym, children);
        }
    }

    // Bare identifier without '?' is an error
    if (token.type == TokenType::IDENTIFIER)
    {
        std::ostringstream oss;
        oss << "Unexpected identifier '" << token.value
            << "'. Variables must start with '?', operators must be wrapped in parentheses.";
        throw std::runtime_error(oss.str());
    }

    // Anything else is an error
    std::ostringstream oss;
    oss << "Unexpected token at position " << token.position;
    throw std::runtime_error(oss.str());
}

std::shared_ptr<Expr> Parser::parse_sexpr(const std::string& input)
{
    if (input.empty())
    {
        throw std::runtime_error("Cannot parse empty string");
    }

    // Tokenize
    tokens = tokenize(input);
    current_token = 0;

    // Parse
    auto expr = parse_expr();

    // Ensure all tokens are consumed
    if (!at_end())
    {
        std::ostringstream oss;
        oss << "Unexpected tokens after expression at position " << peek().position;
        throw std::runtime_error(oss.str());
    }

    return expr;
}
