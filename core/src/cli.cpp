#include "cli.hpp"
#include "templates.hpp"
#include "ltir/ast.hpp"
#include "ltir/evaluator.hpp"
#include "ltir/lexer.hpp"
#include "ltir/parser.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace ltir {
namespace {

// Diag 视图（CLI 层，阶段三）：三类错误一条序列化路径；字段名就此冻结
struct Diag {
  std::string code;
  std::string slug;
  SourcePos   pos;
  std::string line_text;
  std::string caret;
  std::string dev;
  std::string hint;
  std::string tag;
  std::string expected_set;
  std::string got_kind;
};

const ErrorTemplate* findTpl(std::string_view code) noexcept {
  for (int i = 0; i < kErrorTemplateCount; ++i) {
    if (code == kErrorTemplates[i].code) return &kErrorTemplates[i];
  }
  return nullptr;
}

std::string renderTpl(std::string_view t, const Diag& d) {
  std::string out;
  for (std::size_t i = 0; i < t.size();) {
    if (t[i] == '{') {
      const std::size_t e = t.find('}', i);
      if (e != std::string_view::npos) {
        const std::string_view key = t.substr(i + 1, e - i - 1);
        if      (key == "line")          out += std::to_string(d.pos.line);
        else if (key == "col")           out += std::to_string(d.pos.col);
        else if (key == "tag")           out += d.tag;
        else if (key == "expected_set")  out += d.expected_set;
        else if (key == "got_kind")      out += d.got_kind;
        else out += std::string(t.substr(i, e - i + 1));  // 静态扫描会抓未知槽
        i = e + 1;
        continue;
      }
    }
    out += t[i++];
  }
  return out;
}

Diag makeDiag(std::string code, SourcePos pos, std::string line_text) {
  const ErrorTemplate* t = findTpl(code);
  Diag d;
  d.code = std::move(code);
  d.slug = t ? t->slug : "";
  d.pos = pos;
  d.line_text = std::move(line_text);
  d.caret.assign(pos.col ? pos.col - 1 : 0, ' ');   // 按字节，裁决 C
  d.caret += "^";
  // 结构性占位符默认值：稳定、可逐字比对、绝不来自求值值
  d.tag = "-";
  d.expected_set = "closed grammar";
  d.got_kind = "Invalid";
  d.dev = renderTpl(t ? t->dev : "", d);
  d.hint = renderTpl(t ? t->hint : "", d);
  return d;
}

std::string jsonEscape(std::string_view s) {
  static constexpr char kHex[] = "0123456789abcdef";
  std::string o;
  for (const unsigned char c : s) {
    switch (c) {
      case '"':  o += "\\\""; break;
      case '\\': o += "\\\\"; break;
      case '\n': o += "\\n";  break;
      case '\r': o += "\\r";  break;
      case '\t': o += "\\t";  break;
      default:
        if (c < 0x20) {
          o += "\\u00";
          o += kHex[c >> 4];
          o += kHex[c & 15];
        } else {
          o += static_cast<char>(c);
        }
    }
  }
  return o;
}

// 宪法 4：人类可读结果走 stdout；错误（诊断）走 stderr。--json 同样分通道。
void emitDiag(const Diag& d, bool json) {
  if (json) {
    std::cerr
        << "{\"ok\":false,\"exit\":2,\"error\":{"
        << "\"code\":\"" << jsonEscape(d.code) << "\","
        << "\"pos\":{\"line\":" << d.pos.line << ",\"col\":" << d.pos.col << "},"
        << "\"line_text\":\"" << jsonEscape(d.line_text) << "\","
        << "\"caret\":\"" << jsonEscape(d.caret) << "\","
        << "\"dev\":\"" << jsonEscape(d.dev) << "\","
        << "\"hint\":\"" << jsonEscape(d.hint) << "\""
        << "}}\n";
  } else {
    std::cerr
        << "code: "  << d.code  << "\n"
        << "slug: "  << d.slug  << "\n"
        << "pos: "   << d.pos.line << ":" << d.pos.col << "\n"
        << "line: "  << d.line_text << "\n"
        << "caret: " << d.caret << "\n"
        << "dev: "   << d.dev  << "\n"
        << "hint: "  << d.hint << "\n";
  }
}

int failWith(std::string code, SourcePos pos, std::string line_text, bool json) {
  emitDiag(makeDiag(std::move(code), pos, std::move(line_text)), json);
  return 2;
}

int failWith(const ParseError& e, bool json) {
  return failWith(e.code, e.pos, e.line_text, json);
}
int failWith(const EvalError& e, bool json) {
  return failWith(e.code, e.pos, e.line_text, json);
}

} // namespace

int runCli(int argc, char** argv) noexcept {
  bool json = false;
  std::vector<std::string> args;
  for (int i = 1; i < argc; ++i) {
    const std::string_view a(argv[i]);
    if (a == "--json") json = true;
    else args.emplace_back(argv[i]);
  }

  if (args.size() != 2) {
    return failWith("LTIR-E0021", SourcePos{1, 1}, "", json);
  }
  const std::string& verb = args[0];

  if (verb == "eval") {
    const std::string& expr = args[1];
    // eval 通道：argv 串包成单元素 MathLine → 与 check 共用同一表达式 parser
    const std::vector<Token> ml{
        Token{TokenKind::MathLine, SourcePos{1, 1}, std::string_view(expr), {}}};
    const std::vector<Token> toks = Lexer(expr).tokenizeExpr(ml);
    Expr tree;
    ParseError pe;
    if (!Parser(expr).parseExprTokens(toks, tree, &pe)) return failWith(pe, json);
    Value v;
    EvalError ee;
    if (!Evaluator(expr).eval(tree, v, &ee)) return failWith(ee, json);

    const std::string result = formatValue(v);
    if (json) {
      std::cout << "{\"ok\":true,\"exit\":0,\"result\":\""
                << jsonEscape(result) << "\"}\n";
    } else {
      std::cout << result << "\n";
    }
    return 0;
  }

  if (verb == "check") {
    std::ifstream f(args[1], std::ios::binary);
    if (!f) return failWith("LTIR-E0022", SourcePos{1, 1}, "", json);
    const std::string src((std::istreambuf_iterator<char>(f)),
                          std::istreambuf_iterator<char>());

    LtirFile file;
    ParseError pe;
    if (!Parser(src).parseFile(file, &pe)) return failWith(pe, json);

    // exit 2>1>0：任一值求不出就报错，不得先比后报 FAIL（宪法 11）
    const Evaluator ev(src);
    Value sv, av;
    EvalError ee;
    if (!ev.eval(file.subject, sv, &ee)) return failWith(ee, json);
    if (!ev.eval(file.answer,  av, &ee)) return failWith(ee, json);

    const bool pass = valuesEqual(sv, av);   // ℚ 域典范比较，零近似
    if (json) {
      std::cout << "{\"ok\":true,\"exit\":" << (pass ? 0 : 1)
                << ",\"status\":\"" << (pass ? "PASS" : "FAIL") << "\"}\n";
    } else {
      std::cout << (pass ? "PASS\n" : "FAIL\n");   // 永不输出答案（宪法 3）
    }
    return pass ? 0 : 1;
  }

  return failWith("LTIR-E0021", SourcePos{1, 1}, "", json);
}

} // namespace ltir
