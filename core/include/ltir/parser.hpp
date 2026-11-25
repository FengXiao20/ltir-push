// ltir/parser.hpp —— 阶段二·第 2 步：两套递归下降接口（已过审，不改）
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include "ast.hpp"
#include "lexer.hpp"

namespace ltir {

// 裁决 C 的跨层公共件：1-based 行号取整行原文（字节切分，不解码）
// LexError / ParseError / EvalError 共用
std::string_view lineTextOf(std::string_view source,
                            std::uint32_t line) noexcept;

struct ParseError {
  SourcePos   pos;
  std::string code;        // 稳定机器码；码表第 12 条冻结
  std::string message;
  std::string line_text;   // 整行原文（裁决 C）
};

class Parser {
public:
  explicit Parser(std::string_view source) noexcept;

  // 文件层下降：tokenizeFile → 结构校验 → 两个 body 交表达式层
  bool parseFile(LtirFile& out, ParseError* out_error = nullptr) const;

  // 表达式层公开入口：消费 tokenizeExpr 的扁平流（须含 End）
  // CLI eval 与 parseFile 共用，保证两通道同一棵 AST 语义
  bool parseExprTokens(const std::vector<Token>& tokens, Expr& out,
                       ParseError* out_error = nullptr) const;

private:
  bool expect(TokenKind k, const std::vector<Token>& ts, std::size_t& i,
              ParseError* err) const;
  bool collectBody(const std::vector<Token>& ts, std::size_t& i,
                   TokenKind closer, std::vector<Token>& math_lines,
                   ParseError* err) const;

  bool parseExpr  (const std::vector<Token>& ts, std::size_t& i,
                   Expr& out, ParseError* err) const;  // term {(+|-) term}
  bool parseTerm  (const std::vector<Token>& ts, std::size_t& i,
                   Expr& out, ParseError* err) const;  // factor {(*|/) factor}
  bool parseFactor(const std::vector<Token>& ts, std::size_t& i,
                   Expr& out, ParseError* err) const;  // NUMBER | - factor | ( expr )

  // '.' → 分母 10^n，否则整数；剥尾零后超 int64 → ParseError(E0017)
  bool parseNumber(const Token& t, Value& out, ParseError* err) const;

  std::string_view source_;   // 取原文用；Lexer 按需局部构造，无状态
};

} // namespace ltir
