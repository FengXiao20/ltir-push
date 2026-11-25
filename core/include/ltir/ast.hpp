// ltir/ast.hpp —— 阶段二·第 2 步：AST 定义（已过审）
#pragma once
#include <cstdint>
#include <memory>
#include <variant>
#include "lexer.hpp"          // SourcePos
#include "rational.hpp"       // 宪法 10 v3：值域切 ℚ（阶段四批准的连带 diff）

namespace ltir {

// 宪法 10 v3：ℚ 域典范表示；字面量保留原始类别由 Rational 承载
using Value = Rational;

enum class Domain { Math, Physics, Chemistry };

struct Expr;   // 前置声明：递归边只需不完整类型

struct NumberExpr {
  Value     value;
  SourcePos pos;          // 数字起始列
};

struct UnaryExpr {        // v0.0.1 封闭为一元负，不存 Op 枚举
  std::unique_ptr<Expr> operand;
  SourcePos op_pos;
};

struct BinaryExpr {
  enum class Op { Add, Sub, Mul, Div } op;
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;
  SourcePos op_pos;       // 运算符位置，供除零等错误定位
};

struct Expr {
  std::variant<NumberExpr, UnaryExpr, BinaryExpr> node;
};

// 值语义补全：Expr 因 unique_ptr 为 move-only；显式深拷贝，不引入 shared_ptr
// v0.0.1 不调用（evaluator 只读 const Expr&），仅保证「值」可完整复制
Expr cloneExpr(const Expr& e);

struct LtirFile {
  Domain    domain;
  SourcePos domain_pos;   // <subject> 的 '<' 位置，白名单报错用
  Expr      subject;
  Expr      answer;
  SourcePos answer_pos;   // <answer> 位置；缺失时指向文件尾 End
  // 保留字段名（宪法 7）：assumptions、difficulty
  // v0.0.1 不产生语法、不占字段；v0.0.2 启用
};

} // namespace ltir
