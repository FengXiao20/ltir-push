#include "ltir/evaluator.hpp"
#include "ltir/parser.hpp"     // lineTextOf（裁决 C：错误附整行原文）
#include "ltir/rational.hpp"
#include <array>
#include <charconv>
#include <cstdint>
#include <string>
#include <variant>

namespace ltir {
namespace {

std::string i64ToString(std::int64_t v) {
  std::array<char, 32> buf{};
  const auto r = std::to_chars(buf.data(), buf.data() + buf.size(), v);
  return std::string(buf.data(), r.ptr);   // 整数重载：locale 无关
}

// 无符号幅值：INT64_MIN 亦安全（实现注释 1：禁止 -p 直写）
std::uint64_t absU64(std::int64_t v) noexcept {
  return v < 0 ? static_cast<std::uint64_t>(-(v + 1)) + 1u
               : static_cast<std::uint64_t>(v);
}

} // namespace

bool applyBinary(BinaryExpr::Op op, const Value& lhs, const Value& rhs,
                 Value& out, SourcePos op_pos, EvalError* err) {
  bool ok = false;
  switch (op) {
    case BinaryExpr::Op::Add: ok = ratAdd(lhs, rhs, out); break;
    case BinaryExpr::Op::Sub: ok = ratSub(lhs, rhs, out); break;
    case BinaryExpr::Op::Mul: ok = ratMul(lhs, rhs, out); break;
    case BinaryExpr::Op::Div: ok = ratDiv(lhs, rhs, out); break;
  }
  if (ok) return true;
  const bool divZero = (op == BinaryExpr::Op::Div && rhs.p == 0);
  if (err) {
    err->pos = op_pos;
    err->code = divZero ? "LTIR-E0020" : "LTIR-E0018";
    err->message = divZero ? "division by zero" : "checked rational overflow";
    err->line_text.clear();     // Evaluator::eval 用 source_ 补齐
  }
  return false;
}

bool applyNeg(const Value& v, Value& out, SourcePos op_pos, EvalError* err) {
  if (ratNeg(v, out)) return true;
  if (err) {
    err->pos = op_pos;
    err->code = "LTIR-E0019";
    err->message = "checkedNeg failed on INT64_MIN";
    err->line_text.clear();
  }
  return false;
}

Evaluator::Evaluator(std::string_view source) noexcept : source_(source) {}

bool Evaluator::eval(const Expr& e, Value& out, EvalError* out_error) const {
  if (const auto* n = std::get_if<NumberExpr>(&e.node)) {
    out = n->value;
    return true;
  }
  if (const auto* u = std::get_if<UnaryExpr>(&e.node)) {
    Value v;
    if (!eval(*u->operand, v, out_error)) return false;
    if (!applyNeg(v, out, u->op_pos, out_error)) {
      if (out_error) out_error->line_text =
          std::string(lineTextOf(source_, u->op_pos.line));
      return false;
    }
    return true;
  }
  const auto& b = std::get<BinaryExpr>(e.node);
  Value l, r;
  if (!eval(*b.lhs, l, out_error)) return false;
  if (!eval(*b.rhs, r, out_error)) return false;
  if (!applyBinary(b.op, l, r, out, b.op_pos, out_error)) {
    if (out_error) out_error->line_text =
        std::string(lineTextOf(source_, b.op_pos.line));
    return false;
  }
  return true;
}

std::string formatRational(const Rational& r) {
  // 档 1：分母为 1，纯整数串（无类型后缀）
  if (r.q == 1) return i64ToString(r.p);

  // 档 2 判定：约分后分母仅含素因子 2/5，且有限小数位数 ≤ 18
  std::int64_t d = r.q;
  int a = 0, b = 0;
  while (d % 2 == 0) { d /= 2; ++a; }
  while (d % 5 == 0) { d /= 5; ++b; }
  const int n = (a > b) ? a : b;

  if (d == 1 && n <= 18) {
    std::int64_t scale = 1;
    for (int k = 0; k < n; ++k) scale *= 10;   // n≤18 → ≤1e18 < INT64_MAX
    const std::int64_t f = scale / r.q;        // q | 10^n，整除
    const std::uint64_t ap = absU64(r.p);
    const std::uint64_t q = static_cast<std::uint64_t>(r.q);
    const std::uint64_t ip = ap / q;
    const std::uint64_t rem = ap % q;
    const std::uint64_t frac = rem * static_cast<std::uint64_t>(f);

    std::string s = i64ToString(static_cast<std::int64_t>(frac));
    if (s.size() < static_cast<std::size_t>(n))
      s.insert(0, static_cast<std::size_t>(n) - s.size(), '0');
    while (!s.empty() && s.back() == '0') s.pop_back();   // 去尾零

    std::string out;
    if (r.p < 0) out.push_back('-');
    out += i64ToString(static_cast<std::int64_t>(ip));
    if (!s.empty()) { out.push_back('.'); out += s; }
    return out;
  }

  // 档 3：p/q
  // ===== 实现注释 2（阶段三定稿）=====
  // formatRational 的 p/q 档恰好是合法 LTIR 表达式（/ 为真除），可回灌自测；
  // 此为表示巧合，非契约：换输出档、改小数位数上限或扩语法均可破坏。
  // 可用于黄金用例，不承诺、不写入用户文档、不据此设计任何依赖。
  return i64ToString(r.p) + "/" + i64ToString(r.q);
}

std::string formatValue(const Value& v) { return formatRational(v); }

bool valuesEqual(const Value& a, const Value& b) noexcept {
  // ℚ 域典范比较：canonical 唯一 → 逐字段整数 ==
  // 无 epsilon、无跨型提升、零近似（宪法 11）
  return a.p == b.p && a.q == b.q;
}

} // namespace ltir
