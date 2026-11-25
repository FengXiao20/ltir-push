// ltir/rational.hpp —— 宪法 10 v3：精确有理数 ℚ，标准库之外零依赖
#pragma once
#include <cstdint>
#include <string_view>

namespace ltir {

// canonical：q > 0；gcd(|p|,q)==1；零 = 0/1
struct Rational {
  std::int64_t p = 0;   // 分子（可负）
  std::int64_t q = 1;   // 分母（恒正）
};

// 纯 int64 checked 原语：成功写 out 返回 true，溢出返回 false
// 不用 __int128 / 编译器内建 → 本地与 Emscripten 行为逐字一致
bool checkedAdd(std::int64_t a, std::int64_t b, std::int64_t& out) noexcept;
bool checkedSub(std::int64_t a, std::int64_t b, std::int64_t& out) noexcept;
bool checkedMul(std::int64_t a, std::int64_t b, std::int64_t& out) noexcept;
bool checkedNeg(std::int64_t a,              std::int64_t& out) noexcept;

// 构造 canonical：分母为 0 / 约分或取负溢出 → false
bool makeRational(std::int64_t num, std::int64_t den,
                  Rational& out) noexcept;

// 四则 checked：先交叉约分再乘；任一 checked 失败或除零 → false
// 绝不产出近似值（宪法 10 v3 / 11）
bool ratAdd(const Rational& a, const Rational& b, Rational& out) noexcept;
bool ratSub(const Rational& a, const Rational& b, Rational& out) noexcept;
bool ratMul(const Rational& a, const Rational& b, Rational& out) noexcept;
bool ratDiv(const Rational& a, const Rational& b, Rational& out) noexcept;
bool ratNeg(const Rational& a,                   Rational& out) noexcept;

// 十进制字面量 → canonical Rational；"123" / "0.1" / "1.25"
// 不接受 "1." ".5" 指数记法（阶段二冻结文法）
// 宪法 10 v3 补注：构造前先剥小数尾零；剥后超 int64 → false（→ ParseError）
bool parseDecimalToRational(std::string_view text, Rational& out) noexcept;

} // namespace ltir
