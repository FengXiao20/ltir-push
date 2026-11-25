// ltir/evaluator.hpp —— 阶段二·第 3 步（重写·宪法 10 v3 ℚ 域，已过审）
#pragma once
#include <string>
#include <string_view>
#include "ast.hpp"        // Value = Rational, Expr, BinaryExpr::Op, SourcePos
#include "rational.hpp"

namespace ltir {

// 与 ParseError 同构（裁决 C）：三类错误一条序列化路径
struct EvalError {
  SourcePos   pos;
  std::string code;
  std::string message;
  std::string line_text;   // 整行原文
};

// 公开分派：ℚ 域四则 + 一元负，checked
// 溢出 / 除零填 *err 返回 false，绝不产出近似值（宪法 10 v3 / 11）
// line_text 留空，由 Evaluator::eval 用 source_ 补齐
bool applyBinary(BinaryExpr::Op op, const Value& lhs, const Value& rhs,
                 Value& out, SourcePos op_pos, EvalError* err);
bool applyNeg(const Value& v, Value& out,
              SourcePos op_pos, EvalError* err);

class Evaluator {
public:
  explicit Evaluator(std::string_view source) noexcept;

  // 纯函数：同样 (source, Expr) → 同样 Value；无 I/O / 时间 / 随机 / 全局状态
  bool eval(const Expr& e, Value& out, EvalError* out_error = nullptr) const;

private:
  std::string_view source_;   // 仅为填 EvalError::line_text
};

// 三档输出：整数 / 有限小数(≤18 位) / 分数 p/q
// 全程整数运算 + to_chars(int64)，无浮点、无 locale 依赖
std::string formatRational(const Rational& r);

// CLI 唯一出口薄封装，委托 formatRational
std::string formatValue(const Value& v);

// ℚ 域典范比较：canonical 唯一 → 逐字段整数 ==
// R/S 之争撤销；无 epsilon、无跨型提升、零近似
bool valuesEqual(const Value& a, const Value& b) noexcept;

} // namespace ltir
