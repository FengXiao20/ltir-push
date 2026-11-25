#include "ltir/lexer.hpp"
#include <cstdint>
#include <string_view>
#include <vector>

namespace ltir {
namespace {

bool isAsciiLetter(char c) noexcept {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
bool isWs(char c) noexcept { return c == ' ' || c == '\t'; }

std::string_view rtrimWs(std::string_view s) noexcept {
  while (!s.empty() && isWs(s.back())) s.remove_suffix(1);
  return s;
}

} // namespace

Lexer::Lexer(std::string_view source) noexcept : source_(source) {}

std::vector<Token> Lexer::tokenizeFile(LexError* out_error) const {
  std::vector<Token> out;
  (void)out_error;   // 归因统一走 Invalid token，由 Parser 单点收口（宪法 3）

  const std::size_t n = source_.size();
  std::size_t off = 0;
  std::uint32_t line = 1;

  for (;;) {
    std::size_t e = off;
    while (e < n && source_[e] != '\n') ++e;
    std::string_view raw = source_.substr(off, e - off);
    if (!raw.empty() && raw.back() == '\r') raw.remove_suffix(1);

    std::size_t w = 0;
    while (w < raw.size() && isWs(raw[w])) ++w;
    const std::string_view body = rtrimWs(raw.substr(w));
    const bool indented = (w > 0);

    if (indented) {
      // ===== 归因契约（阶段三打回一定案）=====
      // 第 0 列为空白的行，先剥前导空白得 trimmed。若 trimmed 首字节为 '<'
      // 且第二字节为 '/' 或 ASCII 字母（标签形状），则【不产 MathLine】、
      // 改产 Invalid 并短路：标签永远是文件层的事，不能下沉给 tokenizeExpr
      // 报 E0012「改写为算术式」，那会误导 AI 去修一个本无问题的标签。
      // 边界：'<' 后是空格或数字（如 "  < 2"）不具标签形状 → 仍是 MathLine
      // → 表达式层 E0012；顶格未知标签仍是第 0 列 Invalid → E0001。
      if (body.size() >= 2 && body[0] == '<' &&
          (body[1] == '/' || isAsciiLetter(body[1]))) {
        out.push_back(Token{TokenKind::Invalid,
            SourcePos{line, static_cast<std::uint32_t>(w + 1)}, body, {}});
        break;                                   // 首错短路，禁多错误同现
      }
      if (!body.empty() && body[0] != '#') {    // # 行注释按空白（宪法 2）
        out.push_back(Token{TokenKind::MathLine,
            SourcePos{line, static_cast<std::uint32_t>(w + 1)}, body, {}});
      }
    } else {
      if (body.empty() || body[0] == '#') {
        // 空白行 / 顶格注释：按空白，无 AST 节点
      } else if (body == "</subject>") {
        out.push_back(Token{TokenKind::SubjectClose, SourcePos{line, 1}, body, {}});
      } else if (body == "</answer>") {
        out.push_back(Token{TokenKind::AnswerClose, SourcePos{line, 1}, body, {}});
      } else if (body == "<answer>") {
        out.push_back(Token{TokenKind::AnswerOpen, SourcePos{line, 1}, body, {}});
      } else if (body.size() >= 8 && body.substr(0, 8) == "<subject") {
        // 宽松 subject-open 形状：把「属性写法错」也交给 Parser 判 E0003/E0004，
        // 而不是在词法层误判成顶格未知串 E0001。
        constexpr std::string_view kPrefix = "<subject domain=\"";
        std::string_view dom;
        bool wellFormed = false;
        if (body.size() >= kPrefix.size() &&
            body.substr(0, kPrefix.size()) == kPrefix) {
          const std::string_view rest = body.substr(kPrefix.size());
          const std::size_t quote = rest.find('"');
          if (quote != std::string_view::npos && quote + 2 == rest.size() &&
              rest[quote + 1] == '>') {
            dom = rest.substr(0, quote);
            wellFormed = true;
          }
        }
        if (wellFormed) {
          out.push_back(Token{TokenKind::SubjectOpen, SourcePos{line, 1}, body, dom});
        } else if (!body.empty() && body.back() == '>') {
          // <subject> / domain='math' / <subjectX> → domain 空 → E0003
          out.push_back(Token{TokenKind::SubjectOpen, SourcePos{line, 1}, body, {}});
        } else {
          // 标签后面还挂了内容 → 未独占一行 → E0002
          out.push_back(Token{TokenKind::Invalid, SourcePos{line, 1}, body, {}});
          break;
        }
      } else {
        out.push_back(Token{TokenKind::Invalid, SourcePos{line, 1}, body, {}});
        break;                                   // 首错短路
      }
    }

    if (e == n) break;                           // 无换行，处理完即止
    off = e + 1;
    ++line;
  }

  out.push_back(Token{TokenKind::End, SourcePos{line, 1}, {}, {}});
  return out;
}

std::vector<Token> Lexer::tokenizeExpr(const std::vector<Token>& math_lines,
                                       LexError* out_error) const {
  std::vector<Token> out;
  (void)out_error;

  auto finish = [&math_lines](std::vector<Token> o) {
    SourcePos ep{1, 1};
    if (!math_lines.empty()) {
      const Token& last = math_lines.back();
      ep = SourcePos{last.pos.line,
          static_cast<std::uint32_t>(last.pos.col + last.text.size())};
    }
    o.push_back(Token{TokenKind::End, ep, {}, {}});
    return o;
  };

  auto push = [&out](TokenKind k, const Token& ml, std::size_t j,
                     std::string_view t) {
    out.push_back(Token{k,
        SourcePos{ml.pos.line, static_cast<std::uint32_t>(ml.pos.col + j)}, t, {}});
  };

  for (const Token& ml : math_lines) {
    const std::string_view s = ml.text;
    std::size_t j = 0;
    while (j < s.size()) {
      const char c = s[j];
      if (isWs(c)) { ++j; continue; }

      if (c >= '0' && c <= '9') {
        const std::size_t start = j;
        while (j < s.size() && s[j] >= '0' && s[j] <= '9') ++j;
        bool bad = false;
        if (j < s.size() && s[j] == '.') {
          ++j;
          if (j >= s.size() || s[j] < '0' || s[j] > '9') {
            bad = true;                         // "1."
          } else {
            while (j < s.size() && s[j] >= '0' && s[j] <= '9') ++j;
            if (j < s.size() && s[j] == '.') bad = true;   // "1.2.3"
          }
        }
        if (j < s.size() && s[j] == '.') bad = true;
        push(bad ? TokenKind::Invalid : TokenKind::Number,
             ml, start, s.substr(start, j - start));
        if (bad) return finish(std::move(out));   // 首错短路
        continue;
      }

      if (c == '.') {                            // ".5" → E0016
        const std::size_t start = j;
        while (j < s.size() && (s[j] == '.' || (s[j] >= '0' && s[j] <= '9'))) ++j;
        push(TokenKind::Invalid, ml, start, s.substr(start, j - start));
        return finish(std::move(out));
      }

      TokenKind k = TokenKind::Invalid;
      switch (c) {
        case '+': k = TokenKind::Plus;   break;
        case '-': k = TokenKind::Minus;  break;
        case '*': k = TokenKind::Star;   break;
        case '/': k = TokenKind::Slash;  break;
        case '(': k = TokenKind::LParen; break;
        case ')': k = TokenKind::RParen; break;
        default: break;
      }
      if (k == TokenKind::Invalid) {
        // 不支持的 LaTeX / 比较符 / 行内 # 等 → E0012
        push(TokenKind::Invalid, ml, j, s.substr(j, 1));
        return finish(std::move(out));
      }
      push(k, ml, j, s.substr(j, 1));
      ++j;
    }
  }
  return finish(std::move(out));
}

} // namespace ltir
