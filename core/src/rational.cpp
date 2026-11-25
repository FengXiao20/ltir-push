#include "ltir/rational.hpp"
#include <cstdint>
#include <limits>
#include <string_view>

namespace ltir {
namespace {

constexpr std::int64_t kMax = std::numeric_limits<std::int64_t>::max();
constexpr std::int64_t kMin = std::numeric_limits<std::int64_t>::min();

// ===== 实现注释 1 / 2（阶段三定稿，宪法 10 v3）=====
// INT64_MIN 不可直书、经算可达：
// 「-9223372036854775808」的 Number token 是正数 9223372036854775808 > INT64_MAX，
// 在 parseNumber 即 LIT_RANGE；但 「-9223372036854775807-1」 经 checkedSub 可达
// p == INT64_MIN。故 checkedNeg / formatRational / 约分取绝对值处均不得假设
// 「p 可取负」，必须用 checkedNeg 或按无符号幅值处理，禁止 -p 直写。
// ------------------------------------------------------------------
// formatRational 的 p/q 档恰好是合法 LTIR 表达式（/ 为真除），可回灌自测；
// 此为表示巧合，非契约：换输出档、改小数位数上限或扩语法均可破坏。
// 可用于黄金用例，不承诺、不写入用户文档、不据此设计任何依赖。
// ==================================================================

// 无符号幅值：INT64_MIN 亦安全（先 +1 再取负，避免 -kMin 直接溢出）
std::uint64_t absU64(std::int64_t v) noexcept {
  return v < 0 ? static_cast<std::uint64_t>(-(v + 1)) + 1u
               : static_cast<std::uint64_t>(v);
}

std::uint64_t gcdU(std::uint64_t a, std::uint64_t b) noexcept {
  while (b != 0) { const auto t = a % b; a = b; b = t; }
  return a;
}

} // namespace

bool checkedAdd(std::int64_t a, std::int64_t b, std::int64_t& out) noexcept {
  if (b > 0 && a > kMax - b) return false;
  if (b < 0 && a < kMin - b) return false;
  out = a + b;
  return true;
}

bool checkedSub(std::int64_t a, std::int64_t b, std::int64_t& out) noexcept {
  // b == kMin 时不能用 -b；直接用 kMax + b / kMin + b（b 的符号已知，安全）
  if (b < 0 && a > kMax + b) return false;
  if (b > 0 && a < kMin + b) return false;
  out = a - b;
  return true;
}

bool checkedMul(std::int64_t a, std::int64_t b, std::int64_t& out) noexcept {
  if (a == 0 || b == 0) { out = 0; return true; }
  if (a > 0) {
    if (b > 0) { if (a > kMax / b) return false; }
    else       { if (b < kMin / a) return false; }
  } else {
    if (b > 0) { if (a < kMin / b) return false; }
    else       { if (a < kMax / b) return false; }
  }
  out = a * b;
  return true;
}

bool checkedNeg(std::int64_t a, std::int64_t& out) noexcept {
  if (a == kMin) return false;
  out = -a;
  return true;
}

bool makeRational(std::int64_t num, std::int64_t den, Rational& out) noexcept {
  if (den == 0) return false;
  if (den < 0) {
    std::int64_t n = 0, d = 0;
    if (!checkedNeg(num, n) || !checkedNeg(den, d)) return false;
    num = n; den = d;
  }
  if (num == 0) { out = Rational{0, 1}; return true; }
  // g <= den <= kMax，窄化安全
  const std::int64_t g = static_cast<std::int64_t>(gcdU(absU64(num), absU64(den)));
  if (g == 0) return false;
  out = Rational{num / g, den / g};
  return true;
}

bool ratAdd(const Rational& a, const Rational& b, Rational& out) noexcept {
  const std::int64_t g = static_cast<std::int64_t>(gcdU(absU64(a.q), absU64(b.q)));
  const std::int64_t q1 = a.q / g, q2 = b.q / g;
  std::int64_t t1 = 0, t2 = 0, num = 0, den = 0;
  if (!checkedMul(a.p, q2, t1)) return false;
  if (!checkedMul(b.p, q1, t2)) return false;
  if (!checkedAdd(t1, t2, num)) return false;
  if (!checkedMul(q1, b.q, den)) return false;
  return makeRational(num, den, out);
}

bool ratSub(const Rational& a, const Rational& b, Rational& out) noexcept {
  const std::int64_t g = static_cast<std::int64_t>(gcdU(absU64(a.q), absU64(b.q)));
  const std::int64_t q1 = a.q / g, q2 = b.q / g;
  std::int64_t t1 = 0, t2 = 0, num = 0, den = 0;
  if (!checkedMul(a.p, q2, t1)) return false;
  if (!checkedMul(b.p, q1, t2)) return false;
  if (!checkedSub(t1, t2, num)) return false;
  if (!checkedMul(q1, b.q, den)) return false;
  return makeRational(num, den, out);
}

bool ratMul(const Rational& a, const Rational& b, Rational& out) noexcept {
  // 交叉约分再乘：避免本可表示的结果被中间量误报溢出（第 11 条）
  const std::int64_t g1 = static_cast<std::int64_t>(gcdU(absU64(a.p), absU64(b.q)));
  const std::int64_t g2 = static_cast<std::int64_t>(gcdU(absU64(b.p), absU64(a.q)));
  if (g1 == 0 || g2 == 0) return false;
  std::int64_t p = 0, q = 0;
  if (!checkedMul(a.p / g1, b.p / g2, p)) return false;
  if (!checkedMul(a.q / g2, b.q / g1, q)) return false;
  return makeRational(p, q, out);
}

bool ratDiv(const Rational& a, const Rational& b, Rational& out) noexcept {
  if (b.p == 0) return false;                 // → E0020 DIV_ZERO
  return ratMul(a, Rational{b.q, b.p}, out); // den 为负时 makeRational 归一化
}

bool ratNeg(const Rational& a, Rational& out) noexcept {
  std::int64_t p = 0;
  if (!checkedNeg(a.p, p)) return false;      // → E0019 NEG_OVERFLOW
  out = Rational{p, a.q};
  return true;
}

bool parseDecimalToRational(std::string_view text, Rational& out) noexcept {
  if (text.empty()) return false;
  std::size_t dot = std::string_view::npos;
  for (std::size_t i = 0; i < text.size(); ++i) {
    const char c = text[i];
    if (c == '.') {
      if (dot != std::string_view::npos) return false;   // 第二个小数点
      dot = i;
    } else if (c < '0' || c > '9') {
      return false;
    }
  }
  std::string_view int_part = (dot == std::string_view::npos)
      ? text : text.substr(0, dot);
  std::string_view frac = (dot == std::string_view::npos)
      ? std::string_view{} : text.substr(dot + 1);
  if (dot != std::string_view::npos && (int_part.empty() || frac.empty()))
    return false;                                        // "1." / ".5"

  // ===== 剥尾零顺序（宪法 10 v3 补注）=====
  // 必须在任何 int64 溢出判定之前剥除小数尾零：
  // "0.1000000000000000000" 不剥时分母 10^19 > INT64_MAX 会被误判 LIT_RANGE，
  // 剥后只剩 "0.1" → 1/10。剥后分子/分母仍超界才返回 false → E0017。
  while (!frac.empty() && frac.back() == '0') frac.remove_suffix(1);

  std::int64_t num = 0;
  for (char c : int_part) {
    if (!checkedMul(num, 10, num)) return false;
    if (!checkedAdd(num, c - '0', num)) return false;
  }
  for (char c : frac) {
    if (!checkedMul(num, 10, num)) return false;
    if (!checkedAdd(num, c - '0', num)) return false;
  }
  std::int64_t den = 1;
  for (std::size_t i = 0; i < frac.size(); ++i) {
    if (!checkedMul(den, 10, den)) return false;
  }
  return makeRational(num, den, out);
}

} // namespace ltir
