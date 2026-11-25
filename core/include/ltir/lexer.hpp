// ltir/lexer.hpp —— 阶段二·第 1 步：接口 + token 定义（已过审，不改）
#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ltir {

// 1-based：与编辑器/编译器惯例一致，AI 据报错改文件时不用做 -1 换算
struct SourcePos {
  std::uint32_t line = 1;
  std::uint32_t col  = 1;   // 按字节计（裁决 C）
};

enum class TokenKind {
  // 文件层（行级，仅第 0 列识别）
  SubjectOpen,      // <subject domain="...">
  SubjectClose,     // </subject>
  AnswerOpen,       // <answer>
  AnswerClose,      // </answer>
  MathLine,         // 一行缩进的数学源码
  // 表达式层（字符级，由 tokenizeExpr 产出）
  Number, Plus, Minus, Star, Slash, LParen, RParen,
  End,              // 流终止：递归下降的终止条件统一，免查越界
  Invalid           // 顶格未知串 / 非法字符；不 abort，交 Parser 统一报错
};

struct Token {
  TokenKind         kind;
  SourcePos         pos;
  std::string_view  text;    // 零拷贝指向源缓冲区；源串须比 Token 活得久
  std::string_view  domain;  // 仅 SubjectOpen 有效，其余为空；白名单校验归 Parser
};

// 值返回而非异常：错误要能带行列号 + 稳定机器码，且 Emscripten 开异常有体积代价
struct LexError {
  SourcePos   pos;
  std::string code;      // 稳定机器码，供 --json 与 AI 自修正
  std::string message;
};

class Lexer {
public:
  explicit Lexer(std::string_view source) noexcept;

  // 文件层：注释/空行按空白，不产 token（宪法 2）；末尾追加 End
  std::vector<Token> tokenizeFile(LexError* out_error = nullptr) const;

  // 表达式层：逐行词法化；pos = 该 MathLine.pos.line + 行内列偏移
  // 多行自然拼成一条 token 流，无需拼接字符串或偏移表（阶段一 D7）
  std::vector<Token> tokenizeExpr(const std::vector<Token>& math_lines,
                                  LexError* out_error = nullptr) const;

private:
  std::string_view source_;
};

} // namespace ltir
