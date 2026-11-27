// ltir006/src/value.hpp —— 0.0.6 数值域（纯标准库，复用 core 的 checked 有理数）
//
// 纪律：
//  * 全程精确，零浮点；溢出 / 除零 / 未定形一律 UND（交上层判 UNDECIDED），绝不硬猜
//  * core/ 十一件内核一字不动，只 #include "ltir/rational.hpp" 并链接 rational.cpp
//  * ^0 常量折叠发生在「含 π/e 即弃权」之前（patch_005 教训，已并入 0.0.6 立法）
#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <limits>
#include "ltir/rational.hpp"

namespace v6 {

using ltir::Rational;
using ltir::makeRational;
using ltir::ratAdd;
using ltir::ratSub;
using ltir::ratMul;
using ltir::ratDiv;
using ltir::ratNeg;

/* ── 数值 ──────────────────────────────────────────────────────
 * RAT  a                      精确有理
 * RAD  a + b√r                代数数（r 平方自由，r=1 归约为 RAT）
 * CPX  a + im·i               复数（虚部纯有理；与 √ 混合即 UND，诚实弃权）
 * INF  ∞ 哨兵                 1/0；∞−∞ / ∞×0 / ∞/∞ → UND（E-INF）
 * BOOL 比较结果               True / False
 * SYM  coef·sym + off         π / e 符号量，仅供三角精确角与弃权判据
 * UND  弃权                   上层输出 UNDECIDED
 */
struct Num {
  enum K { RAT, RAD, CPX, INF, UND, BOOL, SYM, RAD2 } k = UND;
  Rational a{0,1}, b{0,1};
  std::int64_t r = 1;
  Rational im{0,1};
  Rational c2{0,1}; std::int64_t r2 = 1;   // 0.0.9：a + b√r + c2√r2（双根式精确值）
  bool bv = false;
  Rational coef{1,1}, off{0,1};
  std::string sym;
  bool indet = false;   /* 0.1.1：0/0 不定式原因标记；只答「哪条规则炸的」，不答「值是多少」 */
};

inline Num und() { return Num(); }
/* 0.1.1：不定式原因标记。NaN 不是真值，标记只用于把 UND 归到正确码，
 * 绝不改变 UND 的弃权语义；所有算术/比较短路到 UND 时原样传播。 */
inline Num undIndet() { Num n=und(); n.indet=true; return n; }
inline Num undProp(const Num& x,const Num& y) { Num n=und(); n.indet=x.indet||y.indet; return n; }
inline Num undProp(const Num& x) { Num n=und(); n.indet=x.indet; return n; }
inline Num rat(std::int64_t p, std::int64_t q = 1) {
  Num n; Rational x;
  if (makeRational(p, q, x)) { n.k = Num::RAT; n.a = x; }
  return n;
}
inline Num fromR(const Rational& x) {
  Num n; n.k = Num::RAT; n.a = x; return n;
}
inline Num boolean(bool v) { Num n; n.k = Num::BOOL; n.bv = v; return n; }
inline Num infinity() { Num n; n.k = Num::INF; return n; }
inline Num sym(const std::string& s) {
  Num n; n.k = Num::SYM; n.sym = s; n.coef = rat(1,1).a; n.off = rat(0,1).a;
  return n;
}
/* 0.0.9：双根式精确值，如 (√6 − √2)/4 */
inline Num rad2(std::int64_t p1, std::int64_t q1, std::int64_t r1,
                std::int64_t p2, std::int64_t q2, std::int64_t r2v) {
  Num n; n.k = Num::RAD2; n.a = rat(0,1).a;
  n.b = rat(p1,q1).a; n.r = r1;
  n.c2 = rat(p2,q2).a; n.r2 = r2v;
  return n;
}
inline bool isRad2(const Num& n) { return n.k == Num::RAD2; }
inline bool isZero(const Num& n) {
  if (n.k == Num::RAT) return n.a.p == 0;
  if (n.k == Num::RAD) return n.a.p == 0 && n.b.p == 0;
  if (n.k == Num::CPX) return n.a.p == 0 && n.im.p == 0;
  if (n.k == Num::RAD2) return n.a.p == 0 && n.b.p == 0 && n.c2.p == 0;
  return false;
}
inline void norm(Num& n) {
  if (n.k == Num::RAD) {
    if (n.b.p == 0) { n.k = Num::RAT; n.r = 1; }
    else if (n.r == 1) {
      Rational s;
      if (ratAdd(n.a, n.b, s)) { n.a = s; n.b = rat(0,1).a; n.k = Num::RAT; n.r = 1; }
    }
  }
  if (n.k == Num::CPX && n.im.p == 0) n.k = Num::RAT;
}
inline bool rEq(const Rational& x, std::int64_t p) { return x.q == 1 && x.p == p; }
inline int signR(const Rational& x) { return x.p > 0 ? 1 : (x.p < 0 ? -1 : 0); }
/* 0.0.9：双根式不参与四则运算，一律诚实弃权（不伪造成有理数） */
inline bool rad2Abstain(const Num& x, const Num& y) { return isRad2(x) || isRad2(y); }

inline Num neg(const Num& x) {
  switch (x.k) {
    case Num::RAT: { Rational o; return ratNeg(x.a,o) ? fromR(o) : und(); }
    case Num::RAD: { Num n=x; Rational o1,o2;
      if(!ratNeg(x.a,o1)||!ratNeg(x.b,o2)){return und();} n.a=o1;n.b=o2; return n; }
    case Num::CPX: { Num n=x; Rational o1,o2;
      if(!ratNeg(x.a,o1)||!ratNeg(x.im,o2)){return und();} n.a=o1;n.im=o2; return n; }
    case Num::SYM: { Num n=x; Rational c,o;
      if(!ratNeg(x.coef,c)||!ratNeg(x.off,o)){return und();} n.coef=c;n.off=o; return n; }
    case Num::BOOL: return boolean(!x.bv);
    case Num::RAD2: { Num n=x; Rational a,b,c;
      if(!ratNeg(x.a,a)||!ratNeg(x.b,b)||!ratNeg(x.c2,c)) return und();
      n.a=a; n.b=b; n.c2=c; return n; }
    default: return undProp(x);
  }
}

inline Num add(const Num& x, const Num& y) {
  if (rad2Abstain(x,y)) return und();
  if (x.k==Num::UND || y.k==Num::UND) return undProp(x,y);
  if (x.k==Num::INF || y.k==Num::INF) {
    if (x.k==Num::INF && y.k==Num::INF) return infinity();
    return infinity();
  }
  if (x.k==Num::SYM || y.k==Num::SYM) {
    if (x.k==Num::SYM && y.k==Num::SYM && x.sym==y.sym) {
      Num n=x; Rational c,o;
      if(!ratAdd(x.coef,y.coef,c)||!ratAdd(x.off,y.off,o)) return und();
      n.coef=c;n.off=o; return n;
    }
    if (x.k==Num::SYM && y.k==Num::RAT) { Num n=x; Rational o;
      if(!ratAdd(x.off,y.a,o)){return und();} n.off=o; return n; }
    if (y.k==Num::SYM && x.k==Num::RAT) { Num n=y; Rational o;
      if(!ratAdd(y.off,x.a,o)){return und();} n.off=o; return n; }
    return und();
  }
  if (x.k==Num::CPX || y.k==Num::CPX) {
    if (x.k==Num::RAD || y.k==Num::RAD) return und();
    Num n; n.k=Num::CPX;
    Rational re,im;
    if(!ratAdd(x.k==Num::CPX?x.a:x.a, y.k==Num::CPX?y.a:y.a, re)) return und();
    const Rational xi = (x.k==Num::CPX)?x.im:rat(0,1).a;
    const Rational yi = (y.k==Num::CPX)?y.im:rat(0,1).a;
    if(!ratAdd(xi,yi,im)) return und();
    n.a=re;n.im=im;norm(n);return n;
  }
  // RAT / RAD
  const std::int64_t r = (x.k==Num::RAD)?x.r:(y.k==Num::RAD?y.r:1);
  if (x.k==Num::RAD && y.k==Num::RAD && x.r!=y.r) return und();
  Num n; n.k=Num::RAD; n.r=r;
  const Rational xb = (x.k==Num::RAD)?x.b:rat(0,1).a;
  const Rational yb = (y.k==Num::RAD)?y.b:rat(0,1).a;
  if(!ratAdd(x.a,y.a,n.a)||!ratAdd(xb,yb,n.b)) return und();
  norm(n); return n;
}
inline Num sub(const Num& x, const Num& y) { return add(x, neg(y)); }

inline Num mul(const Num& x, const Num& y) {
  if (rad2Abstain(x,y)) return und();
  if (x.k==Num::UND||y.k==Num::UND) return undProp(x,y);
  if (x.k==Num::INF||y.k==Num::INF) {
    if (isZero(x)||isZero(y)) return und();                  // ∞×0 未定形
    Num n=infinity();
    const int sx = (x.k==Num::INF)?1:(signR(x.a)<0?-1:1);
    const int sy = (y.k==Num::INF)?1:(signR(y.a)<0?-1:1);
    if (sx*sy<0) { /* 符号由上层只做展示，这里保留 INF */ }
    return n;
  }
  if (x.k==Num::SYM||y.k==Num::SYM) {
    if (x.k==Num::SYM && y.k==Num::RAT) { Num n=x; Rational c,o;
      if(!ratMul(x.coef,y.a,c)||!ratMul(x.off,y.a,o)){return und();} n.coef=c;n.off=o;return n; }
    if (y.k==Num::SYM && x.k==Num::RAT) { Num n=y; Rational c,o;
      if(!ratMul(y.coef,x.a,c)||!ratMul(y.off,x.a,o)){return und();} n.coef=c;n.off=o;return n; }
    return und();                                            // π·π 等超越乘积：弃权
  }
  if (x.k==Num::CPX||y.k==Num::CPX) {
    if (x.k==Num::RAD||y.k==Num::RAD) return und();
    const Rational xr=x.a, yr=y.a;
    const Rational xi=(x.k==Num::CPX)?x.im:rat(0,1).a;
    const Rational yi=(y.k==Num::CPX)?y.im:rat(0,1).a;
    Rational t1,t2,t3,t4,re,im;
    if(!ratMul(xr,yr,t1)||!ratMul(xi,yi,t2)||!ratSub(t1,t2,re)) return und();
    if(!ratMul(xr,yi,t3)||!ratMul(xi,yr,t4)||!ratAdd(t3,t4,im)) return und();
    Num n;n.k=Num::CPX;n.a=re;n.im=im;norm(n);return n;
  }
  const std::int64_t r=(x.k==Num::RAD)?x.r:(y.k==Num::RAD?y.r:1);
  if (x.k==Num::RAD&&y.k==Num::RAD&&x.r!=y.r) return und();
  const Rational xb=(x.k==Num::RAD)?x.b:rat(0,1).a;
  const Rational yb=(y.k==Num::RAD)?y.b:rat(0,1).a;
  Rational ac,bd,adr,bcr,rr,real,imag;
  const Rational R=rat(r,1).a;
  if(!ratMul(x.a,y.a,ac)||!ratMul(xb,yb,bd)||!ratMul(bd,R,rr)||!ratAdd(ac,rr,real)) return und();
  if(!ratMul(x.a,yb,adr)||!ratMul(xb,y.a,bcr)||!ratAdd(adr,bcr,imag)) return und();
  Num n;n.k=Num::RAD;n.r=r;n.a=real;n.b=imag;norm(n);return n;
}

inline Num divv(const Num& x, const Num& y) {
  if (rad2Abstain(x,y)) return und();
  if (x.k==Num::UND||y.k==Num::UND) return undProp(x,y);
  if (y.k==Num::INF) return isZero(x)?und():rat(0,1);
  if (x.k==Num::INF) {
    if (y.k==Num::INF) return und();                       // ∞/∞ 未定形
    if (isZero(y)) return und();
    return infinity();
  }
  if (isZero(y)) return isZero(x)?undIndet():infinity();   // 0/0 → E-INDETERMINATE；1/0 → ∞
  /* 0.1.2 B9：RAT/RAT 直接精确相除，不再绕「分母有理化」先算 y.a*y.a。
   * 旧路径 1/1e10 会先算 1e20 撑爆 checkedMul，把能算的式子洗成 UNDECIDED。 */
  if (x.k==Num::RAT&&y.k==Num::RAT) {
    Rational q; if(!ratDiv(x.a,y.a,q)) return und();
    return fromR(q);
  }
  if (x.k==Num::SYM||y.k==Num::SYM) {
    if (x.k==Num::SYM&&y.k==Num::RAT) { Num n=x; Rational c,o;
      if(!ratDiv(x.coef,y.a,c)||!ratDiv(x.off,y.a,o)){return und();} n.coef=c;n.off=o;return n;}
    return und();
  }
  if (y.k==Num::CPX) {
    // (a)/(c+di) = a(c-di)/(c²+d²)
    if (x.k==Num::RAD) return und();
    Rational c2,d2,den;
    if(!ratMul(y.a,y.a,c2)||!ratMul(y.im,y.im,d2)||!ratAdd(c2,d2,den)) return und();
    if (den.p==0) return und();
    Num conj=y; Rational ni; if(!ratNeg(y.im,ni))return und(); conj.im=ni;
    Num p=mul(x,conj);
    if (p.k==Num::UND) return p;
    Num q=fromR(den);
    return divv(p,q);
  }
  if (x.k==Num::CPX) {
    // (a+bi)/c = a/c + (b/c)i
    Rational re,im;
    if(!ratDiv(x.a,y.a,re)||!ratDiv(x.im,y.a,im)) return und();
    Num n;n.k=Num::CPX;n.a=re;n.im=im;norm(n);return n;
  }
  const std::int64_t r=(x.k==Num::RAD)?x.r:(y.k==Num::RAD?y.r:1);
  if (x.k==Num::RAD&&y.k==Num::RAD&&x.r!=y.r) return und();
  const Rational xb=(x.k==Num::RAD)?x.b:rat(0,1).a;
  const Rational yb=(y.k==Num::RAD)?y.b:rat(0,1).a;
  const Rational R=rat(r,1).a;
  Rational yc2,yd2r,den;
  if(!ratMul(y.a,y.a,yc2)||!ratMul(yb,yb,yd2r)||!ratMul(yd2r,R,yd2r)||!ratSub(yc2,yd2r,den)) return und();
  if (den.p==0) return und();
  // (a+b√r)(c-d√r) / den
  Rational t1,t2,t3,t4,real,imag;
  if(!ratMul(x.a,y.a,t1)) return und();
  if(!ratMul(xb,yb,t2)||!ratMul(t2,R,t2)) return und();
  Rational nyd; if(!ratNeg(y.b,nyd)) return und();
  if(!ratMul(x.b,y.a,t3)||!ratMul(x.a,nyd,t4)||!ratAdd(t3,t4,imag)) return und();
  if(!ratSub(t1,t2,real)) return und();
  Num n;n.k=Num::RAD;n.r=r;
  if(!ratDiv(real,den,n.a)||!ratDiv(imag,den,n.b)) return und();
  norm(n);return n;
}

inline Num power(const Num& b, std::int64_t e) {
  if (isRad2(b)) return und();
  if (e==0) return rat(1,1);          // ★ 常量折叠先于 Richardson 判据（π^0 → 1）
  if (b.k==Num::UND||b.k==Num::BOOL) return undProp(b);
  if (b.k==Num::SYM) return und();    // 真超越等式诚实弃权
  if (e<0) {
    if (e==std::numeric_limits<std::int64_t>::min()) return und();
    Num inv=divv(rat(1,1),b);
    return power(inv,-e);
  }
  if (e>64) return und();             // 2^64 / 溢出弃权
  Num res=rat(1,1), base=b;
  while (e>0) {
    if (e&1) { res=mul(res,base); if(res.k==Num::UND) return res; }
    e>>=1;
    if (e) { base=mul(base,base); if(base.k==Num::UND) return base; }
  }
  return res;
}

inline std::int64_t isqrtFloor(std::int64_t n) {
  if (n<0) return -1;
  std::int64_t x=0;
  while ((x+1)<=n/(x+1>0?x+1:1) && (x+1)*(x+1)<=n) ++x;
  return x;
}
inline Num sqrtR(const Rational& x) {
  if (x.p<0) return und();                                // 高中版负数根走 i，另见 cplx 路径
  if (x.p==0) return rat(0,1);
  // √(p/q) = √(p·q)/q
  if (x.p > 100000000LL || x.q > 100000000LL) return und();
  std::int64_t m=x.p*x.q;
  std::int64_t s=isqrtFloor(m);
  while (s>0 && m%(s*s)!=0) --s;
  if (s<=0) s=1;
  const std::int64_t rr=m/(s*s);
  Num n; n.k=Num::RAD; n.r=rr;
  Rational b; if(!makeRational(s,x.q,b)) return und();
  n.a=rat(0,1).a; n.b=b; norm(n); return n;
}

/* 精确比较：返回 -1/0/1；不能判 → 2（上层 UNDECIDED） */
inline int cmp(const Num& x, const Num& y) {
  if (rad2Abstain(x,y)) return 2;
  if (x.k==Num::UND||y.k==Num::UND||x.k==Num::INF||y.k==Num::INF||
      x.k==Num::SYM||y.k==Num::SYM||x.k==Num::CPX||y.k==Num::CPX) return 2;
  if (x.k==Num::BOOL||y.k==Num::BOOL) return 2;
  const std::int64_t r=(x.k==Num::RAD)?x.r:(y.k==Num::RAD?y.r:1);
  if (x.k==Num::RAD&&y.k==Num::RAD&&x.r!=y.r) return 2;
  const Rational xb=(x.k==Num::RAD)?x.b:rat(0,1).a;
  const Rational yb=(y.k==Num::RAD)?y.b:rat(0,1).a;
  Rational da,db;
  if(!ratSub(x.a,y.a,da)||!ratSub(xb,yb,db)) return 2;
  const int sa=signR(da), sb=signR(db);
  if (sb==0) return sa;
  if (sa==0) {
    if (sb>0) return 1; else return -1;
  }
  if (sa>0 && sb>0) return 1;
  if (sa<0 && sb<0) return -1;
  // 异号：比较 da² 与 db²·r
  Rational d2,b2,t;
  if(!ratMul(da,da,d2)||!ratMul(db,db,b2)||!ratMul(b2,rat(r,1).a,t)) return 2;
  const int c=(d2.p>t.p)?1:(d2.p<t.p?-1:0);
  if (c>0) return sa; else return sb;
}

/* 三角精确角：arg = coef·π（分母 ∈ {1,2,3,4,6}） */
inline Num trigValue(const std::string& fn, const Num& arg) {
  Rational c=rat(0,1).a;
  if (arg.k==Num::RAT) { if (arg.a.p!=0) return und(); c=rat(0,1).a; }
  else if (arg.k==Num::SYM && arg.sym=="pi" && arg.off.p==0) c=arg.coef;
  else return und();
  // 化到 [0,2)
  std::int64_t num=c.p, den=c.q;
  if (den<=0||den>1000||num<-10000||num>10000) return und();
  std::int64_t k=num;
  while (k<0) k+=2*den;
  while (k>=2*den) k-=2*den;
  auto key=[&](std::int64_t d){ return den==d ? k : -1; };
  auto V=[&](std::int64_t p,std::int64_t q){ return rat(p,q); };
  auto Rad=[&](std::int64_t p,std::int64_t q,std::int64_t rr){ Num n;n.k=Num::RAD;n.r=rr;n.a=rat(0,1).a;n.b=V(p,q).a;return n; };
  // 以「k/den·π」查 12 个特殊角（全部精确值，无浮点）
  std::int64_t d=den, kk=k;
  auto is=[&](std::int64_t a,std::int64_t b){ return kk*b==a*d; };
  if (is(0,1)) return fn=="sin"?V(0,1):fn=="cos"?V(1,1):V(0,1);
  if (is(1,6)) return fn=="sin"?V(1,2):fn=="cos"?Rad(1,2,3):Rad(1,3,3);
  if (is(1,4)) return fn=="sin"?Rad(1,2,2):fn=="cos"?Rad(1,2,2):V(1,1);
  if (is(1,3)) return fn=="sin"?Rad(1,2,3):fn=="cos"?V(1,2):Rad(1,1,3);
  if (is(1,2)) return fn=="sin"?V(1,1):fn=="cos"?V(0,1):und();
  if (is(2,3)) return fn=="sin"?Rad(1,2,3):fn=="cos"?V(-1,2):neg(Rad(1,1,3));
  if (is(3,4)) return fn=="sin"?Rad(1,2,2):fn=="cos"?neg(Rad(1,2,2)):V(-1,1);
  if (is(5,6)) return fn=="sin"?V(1,2):fn=="cos"?neg(Rad(1,2,3)):neg(Rad(1,3,3));
  if (is(1,1)) return fn=="sin"?V(0,1):fn=="cos"?V(-1,1):V(0,1);
  if (is(7,6)) return fn=="sin"?V(-1,2):fn=="cos"?neg(Rad(1,2,3)):Rad(1,3,3);
  if (is(5,4)) return fn=="sin"?neg(Rad(1,2,2)):fn=="cos"?neg(Rad(1,2,2)):V(1,1);
  if (is(4,3)) return fn=="sin"?neg(Rad(1,2,3)):fn=="cos"?V(-1,2):Rad(1,1,3);
  if (is(3,2)) return fn=="sin"?V(-1,1):fn=="cos"?V(0,1):und();
  /* 0.0.9：π/12 家族（双根式精确值，零浮点） */
  auto R2=[&](std::int64_t p1,std::int64_t q1,std::int64_t rr1,
              std::int64_t p2,std::int64_t q2,std::int64_t rr2){
    return rad2(p1,q1,rr1,p2,q2,rr2); };
  auto Tan=[&](std::int64_t a,std::int64_t b){       // a + b√3
    Num n; n.k=Num::RAD; n.a=rat(a,1).a; n.b=rat(b,1).a; n.r=3; return n; };
  auto S1=[&](){ return R2(1,4,6,-1,4,2); };        // (√6 − √2)/4
  auto S2=[&](){ return R2(1,4,6, 1,4,2); };        // (√6 + √2)/4
  if (is(1,12))  return fn=="sin"?S1():fn=="cos"?S2():Tan(2,-1);
  if (is(5,12))  return fn=="sin"?S2():fn=="cos"?S1():Tan(2, 1);
  if (is(7,12))  return fn=="sin"?S2():fn=="cos"?neg(S1()):neg(Tan(2, 1));
  if (is(11,12)) return fn=="sin"?S1():fn=="cos"?neg(S2()):neg(Tan(2,-1));
  if (is(13,12)) return fn=="sin"?neg(S1()):fn=="cos"?neg(S2()):Tan(2,-1);
  if (is(17,12)) return fn=="sin"?neg(S2()):fn=="cos"?neg(S1()):Tan(2, 1);
  if (is(19,12)) return fn=="sin"?neg(S2()):fn=="cos"?S1():neg(Tan(2, 1));
  if (is(23,12)) return fn=="sin"?neg(S1()):fn=="cos"?S2():neg(Tan(2,-1));
  (void)key;
  return und();                                            // 非特殊角 → Richardson 弃权
}

inline std::string ratStr(const Rational& x) {
  if (x.q==1) return std::to_string(x.p);
  return std::to_string(x.p)+"/"+std::to_string(x.q);
}
inline std::string numStr(const Num& n) {
  switch(n.k){
    case Num::RAT: return ratStr(n.a);
    case Num::BOOL: return n.bv?"True":"False";
    case Num::INF: return "\xE2\x88\x9E";
    case Num::UND: return "UNDECIDED";
    case Num::SYM: {
      std::string s;
      if (n.coef.p!=1||n.coef.q!=1) s+=ratStr(n.coef)+"*";
      s+=n.sym;
      if (n.off.p!=0) s+=" + "+ratStr(n.off);
      return s;
    }
    case Num::CPX: {
      if (n.a.p==0) return (rEq(n.im,1)?"i":(rEq(n.im,-1)?"-i":ratStr(n.im)+"i"));
      std::string s=ratStr(n.a);
      if (n.im.p>0) s+=" + "+(rEq(n.im,1)?"i":ratStr(n.im)+"i");
      else s+=" - "+(rEq(n.im,-1)?"i":ratStr(Rational{-n.im.p,n.im.q})+"i");
      return s;
    }
    case Num::RAD2: {
      /* 同分母时紧凑成 "(√6 - √2)/4"；否则逐项列出 */
      if(n.a.p==0 && n.b.q==n.c2.q){
        auto coef=[](std::int64_t p)->std::string{
          if(p==1) return std::string("");
          if(p==-1) return std::string("-");
          return std::to_string(p);
        };
        std::int64_t p1=n.b.p, p2=n.c2.p;
        const std::int64_t q=n.b.q;
        bool minus=false;
        if(p1<0 || (p1==0 && p2<0)){ minus=true; p1=-p1; p2=-p2; }
        std::string body=coef(p1)+"\xE2\x88\x9A"+std::to_string(n.r);
        body += (p2<0?" - ":" + ");
        body += coef(p2<0?-p2:p2)+"\xE2\x88\x9A"+std::to_string(n.r2);
        std::string val=(q==1)?body:("("+body+")/"+std::to_string(q));
        return minus?("-"+val):val;
      }
      return ratStr(n.a)+" + "+ratStr(n.b)+"\xE2\x88\x9A"+std::to_string(n.r)
            +" + "+ratStr(n.c2)+"\xE2\x88\x9A"+std::to_string(n.r2);
    }
    case Num::RAD: {
      std::string tail;
      if (rEq(n.b,1)) tail="\xE2\x88\x9A"+std::to_string(n.r);
      else if (rEq(n.b,-1)) tail="-\xE2\x88\x9A"+std::to_string(n.r);
      else tail="("+ratStr(n.b)+")\xE2\x88\x9A"+std::to_string(n.r);
      if (n.a.p==0) return tail;
      if (n.b.p>0) return ratStr(n.a)+" + "+tail;
      return ratStr(n.a)+" - "+tail.substr(1);
    }
  }
  return "UNDECIDED";
}

/* ── 符号多项式（id: / derive: / solve: 用） ─────────────────── */
using Monom = std::map<std::string,int>;
struct Poly {
  std::map<Monom,Rational> t;
  bool ok=true;
  static Poly c(std::int64_t p,std::int64_t q=1){ Poly z; Rational r;
    if(!makeRational(p,q,r)){z.ok=false;return z;} z.t[Monom{}]=r; return z; }
  static Poly v(const std::string& s){ Poly z; Monom m; m[s]=1; z.t[m]=rat(1,1).a; return z; }
};
inline Poly pAdd(const Poly&a,const Poly&b){ Poly z=a; if(!a.ok||!b.ok){z.ok=false;return z;}
  for(auto&kv:b.t){ Rational o; auto it=z.t.find(kv.first);
    if(it==z.t.end()){z.t[kv.first]=kv.second;continue;}
    if(!ratAdd(it->second,kv.second,o)){z.ok=false;return z;} it->second=o; }
  for(auto it=z.t.begin();it!=z.t.end();){ if(it->second.p==0) it=z.t.erase(it); else ++it; }
  return z; }
inline Poly pNeg(const Poly&a){ Poly z=a; if(!a.ok){return z;}
  for(auto&kv:z.t){Rational o; if(!ratNeg(kv.second,o)){z.ok=false;return z;}kv.second=o;} return z; }
inline Poly pSub(const Poly&a,const Poly&b){ return pAdd(a,pNeg(b)); }
inline Poly pMul(const Poly&a,const Poly&b){ Poly z; if(!a.ok||!b.ok){z.ok=false;return z;}
  for(auto&x:a.t) for(auto&y:b.t){ Monom m=x.first; for(auto&p:y.first)m[p.first]+=p.second;
    Rational o; if(!ratMul(x.second,y.second,o)){z.ok=false;return z;}
    auto it=z.t.find(m); if(it==z.t.end())z.t[m]=o; else { Rational s;
      if(!ratAdd(it->second,o,s)){z.ok=false;return z;} it->second=s; } }
  for(auto it=z.t.begin();it!=z.t.end();){ if(it->second.p==0)it=z.t.erase(it); else ++it; }
  return z; }
inline Poly pPow(const Poly&a,std::int64_t e){ if(e<0||e>16) {Poly z;z.ok=false;return z;}
  Poly z=Poly::c(1),b=a; while(e>0){ if(e&1)z=pMul(z,b); e>>=1; if(e)b=pMul(b,b);} return z; }
inline std::string monomStr(const Monom&m){ std::string s;
  for(auto&p:m){ if(!s.empty())s+="*"; s+=p.first; if(p.second>1)s+="^"+std::to_string(p.second);} return s; }
inline std::string polyStr(const Poly&p){ if(!p.ok||p.t.empty())return p.ok?"0":"UNDECIDED";
  std::string s; bool first=true;
  for(auto it=p.t.rbegin();it!=p.t.rend();++it){ const Rational&c=it->second; const std::string m=monomStr(it->first);
    std::string term;
    if(m.empty()) term=ratStr(c);
    else if(rEq(c,1)) term=m;
    else if(rEq(c,-1)) term="-"+m;
    else term=ratStr(c)+"*"+m;
    if(first){ s=term; first=false; }
    else { if(!term.empty()&&term[0]=='-') s+=" - "+term.substr(1); else s+=" + "+term; } }
  return s; }
inline Poly pDerive(const Poly&a,const std::string&var){ Poly z; if(!a.ok){z.ok=false;return z;}
  for(auto&kv:a.t){ Monom m=kv.first; auto it=m.find(var); if(it==m.end()||it->second==0)continue;
    Rational c; if(!ratMul(kv.second,rat(it->second,1).a,c)){z.ok=false;return z;}
    if(it->second==1) m.erase(var); else it->second-=1;
    auto jt=z.t.find(m); if(jt==z.t.end())z.t[m]=c; else {Rational o;
      if(!ratAdd(jt->second,c,o)){z.ok=false;return z;}jt->second=o;} }
  return z; }
inline Rational pEvalAt(const Poly&p,const std::map<std::string,Num>&env,bool&ok){ ok=true; Rational acc{0,1};
  for(auto&kv:p.t){ Rational term=kv.second;
    for(auto&m:kv.first){ auto it=env.find(m.first); if(it==env.end()||it->second.k!=Num::RAT){ok=false;return acc;}
      Num pw=power(it->second,m.second); if(pw.k!=Num::RAT){ok=false;return acc;}
      if(!ratMul(term,pw.a,term)){ok=false;return acc;} }
    if(!ratAdd(acc,term,acc)){ok=false;return acc;} }
  return acc; }

} // namespace v6
