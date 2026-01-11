#pragma once

#include <memory>
#include <string>
#include <vector>

#include "symbol_table.h"
#include "theory.h"

namespace eqsat
{

enum class TokenType
{
    LPAREN,     // (
    RPAREN,     // )
    IDENTIFIER, // operator or variable name
    END_OF_INPUT
};

struct Token
{
    TokenType type;
    std::string value;
    size_t position;

    Token(TokenType t, std::string v, size_t pos)
        : type(t)
        , value(v)
        , position(pos)
    {
    }
};

class Parser
{
  private:
    SymbolTable& symbols;
    std::vector<Token> tokens;
    size_t current_token;

    // Lexer methods
    std::vector<Token> tokenize(const std::string& input);
    bool is_whitespace(char c);
    bool is_identifier_char(char c);

    // Parser methods
    std::shared_ptr<Expr> parse_expr();
    Token& peek();
    Token& advance();
    bool at_end();
    void expect(TokenType type, const std::string& message);

  public:
    Parser(SymbolTable& symbols);

    std::shared_ptr<Expr> parse_sexpr(const std::string& input);
};

} // namespace eqsat
