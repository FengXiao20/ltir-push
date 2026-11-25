#include "ltir/parser.hpp"
#include "ltir/rational.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace ltir {

std::string_view lineTextOf(std::string_view source,
                            std::uint32_t line) noexcept {
  if (line == 0) return {};
  const std::size_t n = source.size();
  std::size_t off = 0;
  std::uint32_t cur = 1;
  while (cur < line) {
    const std::size_t e = source.find('\n', off);
    if (e == std::string_view::npos) return {};
    off = e + 1;
    ++cur;
  }
  if (off > n) return {};
  const std::size_t e = source.find('\n', off);
  std::string_view r = (e == std::string_view::npos)
      ? source.substr(off) : source.substr(off, e - off);
  if (!r.empty() && r.back() == '\r') r.remove_suffix(1);
  return r;
}

namespace {

bool fail(ParseError* err, std::string_view source, SourcePos pos,
          const char* code, const char* message) {
  if (err) {
    err->pos = pos;
    err->code = code;
    err->message = message;
    err->line_text = std::string(lineTextOf(source, pos.line));
  }
  return false;
}

// 文件层 Invalid 归类（打回一：归因上提 lexer，parser 短路单码）
bool classifyFileInvalid(ParseError* err, std::string_view source,
                         const Token& t) {
  const std::string_view src = lineTextOf(source, t.pos.line);
  const bool indented = !src.empty() && (src[0] == ' ' || src[0] == '\t');
  if (indented) {
    return fail(err, source, t.pos, "LTIR-E0002",
        "indented tag: move to column 1 and check spelling/quotes/domain");
  }
  // 顶格：合法标签前缀但整行不精确 → 「未独占一行」同样归 E0002
  static constexpr std::string_view kPrefixes[] = {
      "<subject", "</subject", "<answer", "</answer"};
  for (const auto p : kPrefixes) {
    if (t.text.size() >= p.size() && t.text.substr(0, p.size()) == p) {
      return fail(err, source, t.pos, "LTIR-E0002",
          "tag must occupy its own line at column 1");
    }
  }
  return fail(err, source, t.pos, "LTIR-E0001",
      "unknown top-level line (not one of the four tags)");
}

// 表达式层 Invalid 归类：数字形状 → E0016，其余 → E0012
bool classifyExprInvalid(ParseError* err, std::string_view source,
                         const Token& t) {
  const bool badNumber = !t.text.empty() &&
      ((t.text[0] >= '0' && t.text[0] <= '9') || t.text[0] == '.');
  return fail(err, source, t.pos,
      badNumber ? "LTIR-E0016" : "LTIR-E0012",
      badNumber ? "bad numeric literal" : "unsupported LaTeX/character");
}

} // namespace

Expr cloneExpr(const Expr& e) {
  if (const auto* n = std::get_if<NumberExpr>(&e.node)) return Expr{*n};
  if (const auto* u = std::get_if<UnaryExpr>(&e.node)) {
    return Expr{UnaryExpr{std::make_unique<Expr>(cloneExpr(*u->operand)),
                          u->op_pos}};
  }
  const auto& b = std::get<BinaryExpr>(e.node);
  return Expr{BinaryExpr{b.op,
      std::make_unique<Expr>(cloneExpr(*b.lhs)),
      std::make_unique<Expr>(cloneExpr(*b.rhs)), b.op_pos}};
}

Parser::Parser(std::string_view source) noexcept : source_(source) {}

bool Parser::expect(TokenKind k, const std::vector<Token>& ts, std::size_t& i,
                    ParseError* err) const {
  (void)err;   // 具体码由调用方按上下文给，expect 只做消费判定
  if (i < ts.size() && ts[i].kind == k) { ++i; return true; }
  return false;
}

bool Parser::collectBody(const std::vector<Token>& ts, std::size_t& i,
                         TokenKind closer, std::vector<Token>& math_lines,
                         ParseError* err) const {
  const bool subjectPhase = (closer == TokenKind::SubjectClose);
  for (;;) {
    if (i >= ts.size()) {
      return fail(err, source_, SourcePos{1, 1},
          subjectPhase ? "LTIR-E0006" : "LTIR-E0008", "missing closing tag");
    }
    const Token& t = ts[i];
    if (t.kind == closer) { ++i; break; }
    if (t.kind == TokenKind::End) {
      return fail(err, source_, t.pos,
          subjectPhase ? "LTIR-E0006" : "LTIR-E0008", "missing closing tag");
    }
    if (t.kind == TokenKind::Invalid) {
      return classifyFileInvalid(err, source_, t);   // 短路单码
    }
    if (t.kind == TokenKind::MathLine) {
      math_lines.push_back(t);
      ++i;
      continue;
    }
    return fail(err, source_, t.pos,
        subjectPhase ? "LTIR-E0006" : "LTIR-E0008",
        "unexpected structural tag before closing tag");
  }
  if (math_lines.empty()) {
    return fail(err, source_, ts[i - 1].pos, "LTIR-E0009", "empty body");
  }
  return true;
}

bool Parser::parseNumber(const Token& t, Value& out, ParseError* err) const {
  if (!parseDecimalToRational(t.text, out)) {
    return fail(err, source_, t.pos, "LTIR-E0017",
        "literal out of int64 range after stripping trailing zeros");
  }
  return true;
}

bool Parser::parseFactor(const std::vector<Token>& ts, std::size_t& i,
                         Expr& out, ParseError* err) const {
  if (i >= ts.size()) {
    return fail(err, source_, SourcePos{1, 1}, "LTIR-E0013", "expected expression");
  }
  const Token& t = ts[i];
  if (t.kind == TokenKind::Invalid) return classifyExprInvalid(err, source_, t);

  if (t.kind == TokenKind::Number) {
    Value v;
    if (!parseNumber(t, v, err)) return false;
    out = Expr{NumberExpr{v, t.pos}};
    ++i;
    return true;
  }
  if (t.kind == TokenKind::Minus) {
    const SourcePos op = t.pos;
    ++i;
    Expr operand;
    if (!parseFactor(ts, i, operand, err)) return false;
    out = Expr{UnaryExpr{std::make_unique<Expr>(std::move(operand)), op}};
    return true;
  }
  if (t.kind == TokenKind::LParen) {
    ++i;
    Expr inner;
    if (!parseExpr(ts, i, inner, err)) return false;
    if (i >= ts.size() || ts[i].kind != TokenKind::RParen) {
      const SourcePos p = (i < ts.size()) ? ts[i].pos : t.pos;
      return fail(err, source_, p, "LTIR-E0014", "expected ')'");
    }
    ++i;
    out = std::move(inner);
    return true;
  }
  return fail(err, source_, t.pos, "LTIR-E0013", "expected expression");
}

bool Parser::parseTerm(const std::vector<Token>& ts, std::size_t& i,
                       Expr& out, ParseError* err) const {
  Expr lhs;
  if (!parseFactor(ts, i, lhs, err)) return false;
  while (i < ts.size() &&
         (ts[i].kind == TokenKind::Star || ts[i].kind == TokenKind::Slash)) {
    const auto op = (ts[i].kind == TokenKind::Star)
        ? BinaryExpr::Op::Mul : BinaryExpr::Op::Div;
    const SourcePos op_pos = ts[i].pos;
    ++i;
    Expr rhs;
    if (!parseFactor(ts, i, rhs, err)) return false;
    lhs = Expr{BinaryExpr{op,
        std::make_unique<Expr>(std::move(lhs)),
        std::make_unique<Expr>(std::move(rhs)), op_pos}};
  }
  out = std::move(lhs);
  return true;
}

bool Parser::parseExpr(const std::vector<Token>& ts, std::size_t& i,
                       Expr& out, ParseError* err) const {
  Expr lhs;
  if (!parseTerm(ts, i, lhs, err)) return false;
  while (i < ts.size() &&
         (ts[i].kind == TokenKind::Plus || ts[i].kind == TokenKind::Minus)) {
    const auto op = (ts[i].kind == TokenKind::Plus)
        ? BinaryExpr::Op::Add : BinaryExpr::Op::Sub;
    const SourcePos op_pos = ts[i].pos;
    ++i;
    Expr rhs;
    if (!parseTerm(ts, i, rhs, err)) return false;
    lhs = Expr{BinaryExpr{op,
        std::make_unique<Expr>(std::move(lhs)),
        std::make_unique<Expr>(std::move(rhs)), op_pos}};
  }
  out = std::move(lhs);
  return true;
}

bool Parser::parseExprTokens(const std::vector<Token>& tokens, Expr& out,
                             ParseError* err) const {
  std::size_t i = 0;
  Expr e;
  if (!parseExpr(tokens, i, e, err)) return false;
  if (i < tokens.size()) {
    const Token& t = tokens[i];
    if (t.kind == TokenKind::Invalid) return classifyExprInvalid(err, source_, t);
    if (t.kind != TokenKind::End) {
      return fail(err, source_, t.pos, "LTIR-E0015", "trailing token");
    }
  }
  out = std::move(e);
  return true;
}

bool Parser::parseFile(LtirFile& out, ParseError* err) const {
  const Lexer lx(source_);
  const std::vector<Token> ts = lx.tokenizeFile();
  std::size_t i = 0;

  if (i >= ts.size()) {
    return fail(err, source_, SourcePos{1, 1}, "LTIR-E0005", "empty file");
  }
  if (ts[i].kind == TokenKind::Invalid) return classifyFileInvalid(err, source_, ts[i]);
  if (ts[i].kind != TokenKind::SubjectOpen) {
    return fail(err, source_, ts[i].pos, "LTIR-E0005", "missing <subject ...>");
  }
  const Token& so = ts[i];
  if (so.domain.empty()) {
    return fail(err, source_, so.pos, "LTIR-E0003", "missing/wrong domain attribute");
  }
  Domain dom = Domain::Math;
  if      (so.domain == "math")      dom = Domain::Math;
  else if (so.domain == "physics")   dom = Domain::Physics;
  else if (so.domain == "chemistry") dom = Domain::Chemistry;
  else return fail(err, source_, so.pos, "LTIR-E0004", "unknown domain value");
  ++i;

  std::vector<Token> sml;
  if (!collectBody(ts, i, TokenKind::SubjectClose, sml, err)) return false;
  Expr subject;
  if (!parseExprTokens(lx.tokenizeExpr(sml), subject, err)) return false;

  if (i >= ts.size() || ts[i].kind == TokenKind::End) {
    const SourcePos p = (i < ts.size()) ? ts[i].pos : so.pos;
    return fail(err, source_, p, "LTIR-E0007", "missing <answer> block");
  }
  if (ts[i].kind == TokenKind::Invalid) return classifyFileInvalid(err, source_, ts[i]);
  if (ts[i].kind != TokenKind::AnswerOpen) {
    return fail(err, source_, ts[i].pos, "LTIR-E0010", "wrong tag order / duplicate");
  }
  const SourcePos answer_pos = ts[i].pos;
  ++i;

  std::vector<Token> aml;
  if (!collectBody(ts, i, TokenKind::AnswerClose, aml, err)) return false;
  Expr answer;
  if (!parseExprTokens(lx.tokenizeExpr(aml), answer, err)) return false;

  if (i < ts.size()) {
    if (ts[i].kind == TokenKind::Invalid) return classifyFileInvalid(err, source_, ts[i]);
    if (ts[i].kind == TokenKind::MathLine) {
      return fail(err, source_, ts[i].pos, "LTIR-E0011",
          "trailing content after </answer>");
    }
    if (ts[i].kind != TokenKind::End) {
      return fail(err, source_, ts[i].pos, "LTIR-E0010", "duplicate/reordered tag");
    }
  }

  out.domain = dom;
  out.domain_pos = so.pos;
  out.subject = std::move(subject);
  out.answer = std::move(answer);
  out.answer_pos = answer_pos;
  return true;
}

} // namespace ltir
