// ltir006/src/ltir006.cpp —— 0.1.1 语言层（Linux + Termux / C++17 / 纯标准库）
// 0.0.7：集合论实验区开放（语法资产 L0/L1 + 有限集精确判定 + 诚实弃权）
//        防「沙」省略号折叠 + 傲娇彩蛋；公理模块 preset:/load:；⊢ 地图坐标。
//        Windows 10+ 分支本轮不重打，共用同一份源码、同一套资源表。
//
// 架构：core/ 十一件内核一字不动（SHA 门禁常驻），本文件是「一源」的自包含语言层。
// 语法关键词见 SYNTAX.md；用法见 速览.md；错误解释全部由 lab_codes.tsv 驱动。
// 三态锁死：True / False / UNDECIDED，UNDECIDED 绝不并入 False。
#include "value.hpp"

/* 0.1.1：版本号单一来源——所有横幅/JSON/help 引用本宏，杜绝重复字面量 */
#define LTIR_VER "0.2.1"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <cstdint>
#include <iomanip>
#include <cctype>
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00   // Win10+：RegDeleteTreeA / RegCreateKeyExA 等
#endif
#include <windows.h>
#include <conio.h>
#endif

using namespace v6;

/* ══════════════ 0.0.7 防「沙」：多打省略号折叠 + 傲娇彩蛋 ══════════════ */
static int g_sandFix = 0;                      // 本次 gate 折叠掉的多余省略号组数
static const char* SAND_ZH = "你多打了省略号，已经补全了。才不是帮你呢。";
static const char* SAND_EN = "You typed extra ellipses; normalized to one. "
                             "N-not like I did it for you or anything!";
static const char* EGG_ZH  = "才不是帮你呢。";
static const char* EGG_EN  = "N-not like I did it for you or anything!";
/* 把任意「一组 ≥3 个点」及其后空白+更多省略号组折叠成单个 ...
 * 保护 1..10（两点）与 0.5（一点）；单个 ... 不触发吐槽。 */
static int collapseEllipsis(std::string& s) {
  int fixes=0; std::string out; std::size_t i=0;
  while(i<s.size()){
    if(s[i]=='.'){
      std::size_t j=i; while(j<s.size()&&s[j]=='.')++j;
      const std::size_t cnt=j-i;
      if(cnt>=3){
        std::size_t k=j; int groups=1;
        for(;;){
          std::size_t m=k;
          while(m<s.size()&&(s[m]==' '||s[m]=='\t'||s[m]==','||s[m]==';'))++m;
          if(m<s.size()&&s[m]=='.'){
            std::size_t q=m; while(q<s.size()&&s[q]=='.')++q;
            if(q-m>=3){ ++groups; k=q; continue; }
          }
          break;
        }
        if(cnt>3||groups>1) ++fixes;
        out += "..."; i=k; continue;
      }
      out.append(s,i,j-i); i=j; continue;
    }
    out.push_back(s[i]); ++i;
  }
  s=out; return fixes;
}
/* LaTeX → 内部 ASCII 别名（按长度降序，避免 \notin 被 \in 抢先） */
static void latexNorm(std::string& s) {
  static const std::pair<const char*,const char*> T[] = {
    {"2^{\\aleph_0}"," 2^aleph0 "},
    {"\\mathfrak{c}"," cfrak "},{"\\varnothing"," emptyset "},
    {"\\subseteq"," subseteq "},{"\\supseteq"," supseteq "},
    {"\\subseteqq"," subseteq "},
    {"\\setminus"," diff "},{"\\mathbb{N}"," N "},
    {"\\mathbb{R}"," R "},{"\\mathbb{Z}"," Z "},{"\\mathbb{Q}"," Q "},
    {"\\aleph_0"," aleph0 "},{"\\aleph_1"," aleph1 "},
    {"\\infty"," oo "},{"\\notin"," notin "},
    {"\\cdots"," ... "},{"\\ldots"," ... "},{"\\dots"," ... "},
    {"\\Box"," box "},{"\\Diamond"," diamond "},
    {"\\vdash"," vdash "},{"\\models"," models "},
    {"\\subset"," subseteq "},{"\\supset"," supseteq "},
    {"\\neg"," not "},{"\\cup"," union "},{"\\cap"," inter "},
    {"\\emptyset"," emptyset "},{"\\Omega"," Omega "},
    {"\\omega"," omega "},{"\\forall"," forall "},
    {"\\exists"," exists "},{"\\in"," in "},
    {nullptr,nullptr}
  };
  for(int k=0;T[k].first;++k){
    const std::string a=T[k].first,b=T[k].second;
    for(;;){ const std::size_t q=s.find(a); if(q==std::string::npos)break;
            s.replace(q,a.size(),b); }
  }
}

/* ══════════════ 入口防护（CRLF / BOM / 全角 / 数学 Unicode） ══════════════ */
static std::string gate(std::string s) {
  if (s.size() >= 3 && (unsigned char)s[0]==0xEF && (unsigned char)s[1]==0xBB &&
      (unsigned char)s[2]==0xBF) s.erase(0,3);
  std::string t;
  for (std::size_t i=0;i<s.size();++i) {
    if (s[i]=='\r') { if (i+1<s.size()&&s[i+1]=='\n') continue; t.push_back('\n'); continue; }
    t.push_back(s[i]);
  }
  // UTF-8 逐码点归一：全角 ASCII → 半角；数学符号 → ASCII 别名
  std::string out;
  std::size_t i=0;
  while (i<t.size()) {
    const std::size_t begin=i;
    const std::uint32_t cp=[&](){
      const unsigned char c=(unsigned char)t[i];
      if (c<0x80u){++i;return (std::uint32_t)c;}
      int n=0; std::uint32_t v=0;
      if((c&0xE0u)==0xC0u){n=2;v=c&0x1Fu;} else if((c&0xF0u)==0xE0u){n=3;v=c&0x0Fu;}
      else if((c&0xF8u)==0xF0u){n=4;v=c&0x07u;} else {++i;return (std::uint32_t)c;}
      if(i+n>t.size()){++i;return (std::uint32_t)c;}
      for(int k=1;k<n;++k){const unsigned char d=(unsigned char)t[i+k];
        if((d&0xC0u)!=0x80u){++i;return (std::uint32_t)c;} v=(v<<6)|(d&0x3Fu);}
      i+=n; return v;
    }();
    if (cp>=0xFF01u && cp<=0xFF5Eu) { out.push_back((char)(cp-0xFEE0u)); continue; }
    if (cp==0x3000u) { out.push_back(' '); continue; }
    const char* alias=nullptr;
    switch(cp){
      case 0x03C0u: alias="pi"; break;
      case 0x221Au: alias="sqrt_"; break;       // 下一步补括号
      case 0x221Eu: alias="oo"; break;
      case 0x00D7u: alias="*"; break;
      case 0x00F7u: alias="/"; break;
      case 0x2212u: alias="-"; break;
      case 0x2192u: case 0x21D2u: case 0x27F6u: alias="->"; break;
      case 0x00B2u: alias="^2"; break;
      case 0x00B3u: alias="^3"; break;
      case 0x2264u: alias="<="; break;
      case 0x2265u: alias=">="; break;
      case 0x2260u: alias="!="; break;
      /* 0.0.7 集合论实验区：域外记号升格为输入算子（002 铁律 4 输入侧撤销） */
      case 0x2026u: alias="..."; break;                 // … → ...
      case 0x22A2u: alias=" vdash "; break;             // ⊢
      case 0x22A8u: alias=" models "; break;            // ⊨
      case 0x00ACu: alias=" not "; break;               // ¬
      case 0x2208u: alias=" in "; break;                // ∈
      case 0x2209u: alias=" notin "; break;             // ∉
      case 0x222Au: alias=" union "; break;             // ∪
      case 0x2229u: alias=" inter "; break;             // ∩
      case 0x2216u: alias=" diff "; break;              // ∖
      case 0x2286u: alias=" subseteq "; break;          // ⊆
      case 0x2287u: alias=" supseteq "; break;          // ⊇
      case 0x2282u: alias=" subset "; break;            // ⊂（真子集，不与 ⊆ 合并）
      case 0x2283u: alias=" supset "; break;            // ⊃
      case 0x2205u: alias=" emptyset "; break;          // ∅
      case 0x03A9u: alias="Omega"; break;               // Ω（绝对无限）
      case 0x03C9u: alias="omega"; break;               // ω
      case 0x2200u: alias="forall "; break;             // ∀
      case 0x2203u: alias="exists "; break;             // ∃
      case 0x2234u: alias=" [UNREGSYM] "; break;        // ∴ 未登记符号
      case 0x22A3u: alias=" [UNREGSYM] "; break;        // ⊣ 未登记符号
      case 0x25A1u: alias=" box "; break;               // □ 所有 forcing 扩张
      case 0x25C7u: alias=" diamond "; break;           // ◇ 某个 forcing 扩张
      default: break;
    }
    if (alias) { out+=alias; continue; }
    // 其余（中文 / 域外记号 ∪∩⊆ / 标点）原样保留，不解释
    out.append(t, begin, i-begin);
  }
  // √ → sqrt( ... )：后面的括号已够；裸数字/标识符补一对括号
  for (;;) {
    const std::size_t q=out.find("sqrt_");
    if (q==std::string::npos) break;
    out.replace(q,5,"sqrt");
    std::size_t p=q+4;
    if (p<out.size() && out[p]=='(') continue;
    std::size_t e=p;
    while (e<out.size() && (isalnum((unsigned char)out[e])||out[e]=='_'||out[e]=='.')) ++e;
    out.insert(p,"("); out.insert(e+1,")");
  }
  // LaTeX \frac{A}{B} → ((A)/(B))
  for (;;) {
    const std::size_t q=out.find("\\frac{");
    if (q==std::string::npos) break;
    std::size_t a=q+6; int d=1; std::size_t e=a;
    while (e<out.size()&&d>0){ if(out[e]=='{')++d; else if(out[e]=='}')--d; ++e; }
    if (d!=0) break;
    const std::string A=out.substr(a,e-a-1);
    if (e>=out.size()||out[e]!='{') break;
    std::size_t b=e+1; d=1; std::size_t f=b;
    while (f<out.size()&&d>0){ if(out[f]=='{')++d; else if(out[f]=='}')--d; ++f; }
    if (d!=0) break;
    const std::string B=out.substr(b,f-b-1);
    const std::string rep="(("+A+")/("+B+"))";
    out.replace(q,f-q,rep);
  }
  latexNorm(out);
  g_sandFix += collapseEllipsis(out);   // 防「沙」：n 个省略号 → 单个
  return out;
}

static bool hasCjk(const std::string& s) {
  for (std::size_t i=0;i<s.size();++i) if ((unsigned char)s[i]>=0x80u) return true;
  return false;
}
static std::string trim(const std::string& s) {
  std::size_t b=0,e=s.size();
  while(b<e&&(s[b]==' '||s[b]=='\t'||s[b]=='\r'||s[b]=='\n'))++b;
  while(e>b&&(s[e-1]==' '||s[e-1]=='\t'||s[e-1]=='\r'||s[e-1]=='\n'))--e;
  return s.substr(b,e-b);
}
/* 显示宽度（East Asian Wide 码点范围，不用 \p{…}） */
static int dispWidth(const std::string& s) {
  int w=0; std::size_t i=0;
  while(i<s.size()){
    const unsigned char c=(unsigned char)s[i];
    std::uint32_t cp=c; std::size_t n=1;
    if(c>=0xE0u&&c<0xF0u&&i+2<s.size()){n=3;cp=((c&0x0Fu)<<12)|(((unsigned char)s[i+1]&0x3Fu)<<6)|((unsigned char)s[i+2]&0x3Fu);}
    else if(c>=0xC0u&&c<0xE0u&&i+1<s.size()){n=2;cp=((c&0x1Fu)<<6)|((unsigned char)s[i+1]&0x3Fu);}
    w += (cp>=0x1100u && (cp<=0x115Fu||cp==0x2329u||cp==0x232Au||
          (cp>=0x2E80u&&cp<=0xA4CFu)||(cp>=0xAC00u&&cp<=0xD7A3u)||
          (cp>=0xF900u&&cp<=0xFAFFu)||(cp>=0xFE10u&&cp<=0xFE6Fu)||
          (cp>=0xFF00u&&cp<=0xFF60u)||(cp>=0xFFE0u&&cp<=0xFFE6u))) ? 2 : 1;
    i+=n;
  }
  return w;
}

/* ══════════════ 关键词表（英文长名 + 带附号短写，禁裸单字母） ══════════════ */
static const std::map<std::string,std::string> SHORT_MAP = {
  {"A:","assert:"},{"V:","value:"},{"N:","numth:"},{"at:","at:"},
  {"id:","ident:"},{"val:","value:"},{"nt:","numth:"},{"<s>","subject:"}
};
static bool isKeyword(const std::string& k) {
  static const std::set<std::string> K = {
    "assert:","value:","at:","ident:","numth:","given:","define:","solve:",
    "derive:","sum:","seq:","limit:","vector:","prob:","stat:","set:",
    "domain:","case:","check:","const:","note:","subject:","zf:","zfc:",
    /* 0.0.7 集合论全图：新增只给长名，禁裸单字母，短写表不开 */
    "system:","choice:","axiom:","ref:","module:","preset:","load:",
    "universe:","theory:","inner:","paradox:","absolute:","problem:",
    "status:","forall:","under:","claim:","cert:","witness:","card:",
    "univ:","map:","independent:","model:","cite:",
    /* 补丁二：量词 / 反例 / 映射性质 / 有限映射定义（全英文长名，短写表仍冻结） */
    "exists:","cex:","inj:","surj:","bij:","mapping:",
    /* 0.0.8：形式化定义 / 集合论多宇宙观（全英文长名，短写表仍冻结） */
    "sig:","type:","where:","rule:","grammar:","expand:",
    "multiverse:","world:","force:","ground:","mantle:","reachable:",
    /* 0.1.0 进制眼镜 / 赋值环境 / 试卷库 / 一致性强度·势·真类 / 范畴论（全英文长名，短写表仍冻结） */
    "radix:","base:","submit:","hint:","difficulty:","attribution:","exam:",
    "con:","equiconsistent:","strength_le:","interprets:","translation:","theory_sig:",
    "class:","proper_class:","card_le:","card_eq:",
    "cat:","object:","arrow:","compose:","identity:","commute:",
    "functor:","nat:","preserves:","colimit:","product:",   /* limit: 已在 0.0.6 分析组登记，此处不重复 */
    "equalizer:","pullback:","adj:","unit:","counit:","yoneda:",
    "topos:","subobject:","monad:","algebra:","carrier:","structure:",
    /* 0.1.0 A 增量：理论语言签名（全英文长名，短写表仍冻结） */
    "lang:",
    /* 强度草案试验：指纹口径开关 + 展示型值标签（只登记，不产值；短写表仍冻结） */
    "sig_mode:","ordinal:","benchmark:","metatheory:","notation:","con_base:",
    /* 0.1.1：顶层证书行 by: 只登记，不再落 E-UNK-LINE（短写表仍冻结） */
    "by:",
    /* 0.2.0 元编程：只登记、0 三态、不自举（全英文长名，短写表仍冻结） */
    "gap:","reformulate:","quote:","reflect:","depth:","trace:",
    /* 0.2.1 重制：四项元能力各占一个工位词 + 试验门禁（全英文长名，短写表仍冻结） */
    "shape:","bind:","rewrite:","split:","mode:"
  };
  return K.count(k)>0;
}
static std::string normalizeKey(const std::string& k) {
  auto it=SHORT_MAP.find(k); return it==SHORT_MAP.end()?k:it->second;
}

/* 0.0.6 强化修复版：解题文本层装饰简写（机器跳过、原样保留、渲染加粗）
 * 与字段全拼小写分界：effect: 是字段，E: 是简写行；裸 C: 历史封死不放开。
 * V: 在 0.0.6 里仍是 value: 求值短写（回归零破坏），仅额外登记装饰身份。 */
static bool isDecoration(const std::string& k) {
  static const std::set<std::string> D = {"S:","E:","P:","Q:"};
  return D.count(k)>0;
}
static const std::set<std::string>& containerTags() {
  static const std::set<std::string> T = {
    "a","calc","C_and_E","answer","s","subject","proc","过程"
  };
  return T;
}
static std::string closeTag(const std::string& open) {
  if(open=="a"){ return "</a>"; }
  if(open=="calc"){ return "</calc>"; }
  if(open=="C_and_E"){ return "</C_and_E>"; }
  if(open=="answer"){ return "</answer>"; }
  if(open=="s"){ return "</s>"; }
  if(open=="subject"){ return "</subject>"; }
  if(open=="proc"){ return "</proc>"; }
  if(open=="过程"){ return "</过程>"; }
  return "</"+open+">";
}

/* ══════════════ 码表（只读，解释由表驱动） ══════════════ */
struct Code { int state=1; std::string en, zh; };
static std::map<std::string,Code> CODES;
static void loadCodes(const std::string& path) {
  std::ifstream f(path);
  std::string ln;
  while (std::getline(f,ln)) {
    if (ln.empty()||ln[0]=='#') continue;
    std::vector<std::string> c; std::stringstream ss(ln); std::string x;
    while (std::getline(ss,x,'\t')) c.push_back(x);
    if (c.size()<4) continue;
    CODES[c[0]] = Code{std::atoi(c[1].c_str()), c[2], c[3]};
  }
}

/* ══════════════ 0.1.1：E-INDETERMINATE ∵∴ 因果链（表驱动码 + 形式化展示） ══════════════
 * 默认只给一行 ∴；--explain 摊开完整链。全链走 stdout 的人话档，
 * --tri 只吐三态词、--json 只走结构字段，均不被形式化展示污染。 */
static std::string indetLine(bool zh){
  return zh ? "∴ 0/0 不定式：逆元不存在 ∧ 商不唯一 ∧ 路径依赖 → 诚实弃权（人话兜底词 NaN，非真值）"
            : "∴ 0/0 indeterminate: inverse undefined ∧ quotient non-unique ∧ path-dependent → honest abstain (NaN is a word, not a value)";
}
static std::string indetChain(bool zh){
  if(zh) return
    "∵ N   norm(1-1, 1-1) = (0, 0)          归一化后再判，不靠字面匹配\n"
    "∵ R1  a/b ≜ a·b⁻¹            (b ≠ 0)    除法定义\n"
    "∵ R2  b ≠ 0 ⇒ ∃! b⁻¹, b·b⁻¹ = 1        域逆元公理\n"
    "∵ R3  ∀c∈ℚ: 0·c = 0                    零吸收律\n"
    "∵ R4  lim x/x = 1, lim x²/x = 0, lim x/x² = ∞   路径依赖\n"
    "─────────────────────────────────────────────────────────\n"
    "∴ R2 ∧ b=0 ⇒ 0⁻¹ 不存在，R1 前提失效\n"
    "∴ 外推 R1 至 b=0 ⇒ 0 = 0·c 对 ∀c 成立（R3）⇒ c 不唯一，商无定义\n"
    "∴ R4 ⇒ 不可归为单一 ∞（1/0 才是无界发散登记）\n"
    "∴ 与 ∞−∞ / 0·∞ 同档：不赋 True / 不并 False / 不并 ∞\n"
    "∴ UNDECIDED  E-INDETERMINATE   不定式：诚实弃权";
  return
    "∵ N   norm(1-1, 1-1) = (0, 0)          normalize first, no string matching\n"
    "∵ R1  a/b ≜ a·b⁻¹            (b ≠ 0)    division definition\n"
    "∵ R2  b ≠ 0 ⇒ ∃! b⁻¹, b·b⁻¹ = 1        field inverse axiom\n"
    "∵ R3  ∀c∈ℚ: 0·c = 0                    zero absorption\n"
    "∵ R4  lim x/x = 1, lim x²/x = 0, lim x/x² = ∞   path-dependent 0/0\n"
    "─────────────────────────────────────────────────────────\n"
    "∴ R2 ∧ b=0 ⇒ 0⁻¹ undefined; R1 precondition fails\n"
    "∴ extending R1 to b=0 ⇒ 0 = 0·c for every c (R3) ⇒ c not unique\n"
    "∴ R4 ⇒ not a single ∞; 1/0 registers unbounded divergence\n"
    "∴ same tier as ∞−∞, 0·∞: no True, not folded into False, not ∞\n"
    "∴ UNDECIDED  E-INDETERMINATE   indeterminate form: honest abstain";
}

static std::string explain(const std::string& code, bool chinese,
                           const std::map<std::string,std::string>& fill) {
  auto it=CODES.find(code);
  if (it==CODES.end()) return code;
  std::string s = chinese ? it->second.zh : it->second.en;
  for (auto&kv:fill) {
    std::string k="{"+kv.first+"}";
    for(;;){ const std::size_t q=s.find(k); if(q==std::string::npos)break; s.replace(q,k.size(),kv.second); }
  }
  return s;
}

/* ══════════════ 词法 / 语法 ══════════════ */
struct Tok { enum T{NUM,ID,OP,LP,RP,LB,RB,COMMA,END} t=END; std::string s; Rational v; };
static std::vector<Tok> lex(const std::string& src) {
  std::vector<Tok> r; std::size_t i=0;
  auto push=[&](Tok::T t,const std::string& s){ Tok z; z.t=t; z.s=s; r.push_back(z); };
  while (i<src.size()) {
    const char c=src[i];
    if (c==' '||c=='\t'||c=='\n'||c=='\r') { ++i; continue; }
    if ((unsigned char)c>=0x80u) { ++i; continue; }       // 域外/中文：跳过，不解释
    if (isdigit((unsigned char)c)||(c=='.'&&i+1<src.size()&&isdigit((unsigned char)src[i+1]))) {
      std::size_t b=i; while(i<src.size()&&(isdigit((unsigned char)src[i])||src[i]=='.'))++i;
      const std::string s=src.substr(b,i-b);
      /* 0.1.2 B10：大写 E 科学计数法整体吃进 NUM。
       * 小写 e 仍是欧拉数 SYM，绝不拆成 1.1 * e * 17；零浮点，仍是精确分数。 */
      if(i<src.size()&&src[i]=='E'){
        std::size_t j=i+1; bool eneg=false;
        if(j<src.size()&&(src[j]=='+'||src[j]=='-')){ eneg=(src[j]=='-'); ++j; }
        const std::size_t eb=j;
        while(j<src.size()&&isdigit((unsigned char)src[j]))++j;
        if(j>eb&&j-eb<=2){
          int e=0; for(std::size_t t=eb;t<j;++t) e=e*10+(src[t]-'0');
          if(eneg) e=-e;
          if(e>=-18&&e<=18){
            std::string m=s; int frac=0;
            const std::size_t dot=m.find('.');
            if(dot!=std::string::npos){ frac=(int)(m.size()-dot-1); m.erase(dot,1); }
            const std::size_t nz=m.find_first_not_of('0');
            const std::string digs=(nz==std::string::npos?"0":m.substr(nz));
            const int shift=e-frac;
            std::string out;
            if(shift>=0) out=digs+std::string(shift,'0');
            else { const int k=-shift;
              if(k>=(int)digs.size()) out="0."+std::string(k-(int)digs.size(),'0')+digs;
              else out=digs.substr(0,digs.size()-k)+"."+digs.substr(digs.size()-k); }
            Rational sv;
            if(ltir::parseDecimalToRational(out,sv)){
              i=j; Tok z; z.t=Tok::NUM; z.s=src.substr(b,j-b); z.v=sv; r.push_back(z); continue;
            }
          }
        }
      }
      Rational v;
      if (!ltir::parseDecimalToRational(s,v)) { push(Tok::ID,s); continue; }
      Tok z; z.t=Tok::NUM; z.s=s; z.v=v; r.push_back(z); continue;
    }
    if (isalpha((unsigned char)c)||c=='_'||c=='\'') {
      std::size_t b=i; while(i<src.size()&&(isalnum((unsigned char)src[i])||src[i]=='_'||src[i]=='\''))++i;
      push(Tok::ID,src.substr(b,i-b)); continue;
    }
    if (c=='('){push(Tok::LP,"(");++i;continue;} if(c==')'){push(Tok::RP,")");++i;continue;}
    if (c=='['){push(Tok::LB,"[");++i;continue;} if(c==']'){push(Tok::RB,"]");++i;continue;}
    if (c==','){push(Tok::COMMA,",");++i;continue;}
    static const char* ops3[]={"...",nullptr};
    (void)ops3;
    static const char* ops2[]={"==","!=","<=",">=","->","..","**",nullptr};
    bool hit=false;
    for(int k=0;ops2[k];++k){ const std::string o=ops2[k];
      if(src.compare(i,o.size(),o)==0){ push(Tok::OP,o); i+=o.size(); hit=true; break; } }
    if(hit)continue;
    if (std::string("+-*/^!=<>:").find(c)!=std::string::npos) { push(Tok::OP,std::string(1,c)); ++i; continue; }
    ++i;                                                     // 其余字符忽略
  }
  push(Tok::END,""); return r;
}

static const std::set<std::string> FUNCS = {
  "sin","cos","tan","sqrt","abs","isprime","nextprime","factors","gcd","lcm",
  "phi","modpow","C","P","mean","var","cross","dot","L"
};

struct Parser {
  std::vector<Tok> t; std::size_t p=0;
  std::map<std::string,Num> env;
  std::map<std::string,Poly> funcs;
  std::map<std::string,std::pair<std::string,std::string>> seqs; // name -> (var, expr)
  std::map<std::string,std::string> defParams;  // 0.1.1：define: f(t) 的真实参数名
  bool ok=true;
  /* 0.1.1 bugfix B4：递归深度护栏——深嵌套诚实弃权，绝不栈溢出崩溃。
   * expr()/postfix() 共用计数；1024 层远高于正常算式，远低于实测崩溃区（3000+）。 */
  int depth=0;
  struct DepthGuard{ int& d; bool over; explicit DepthGuard(int& dd):d(dd){over=(++d>1024);} ~DepthGuard(){--d;} };
  explicit Parser(const std::string& s):t(lex(s)){}
  /* 0.1.1：三处写死 "x" 的消费端统一走这里；旧卷缺省回退 x，零回归 */
  std::string paramOf(const std::string& fn) const {
    auto it=defParams.find(fn); return it==defParams.end()?std::string("x"):it->second;
  }
  const Tok& cur() const { return t[p<t.size()?p:t.size()-1]; }
  bool eat(const std::string& s){ if(cur().s==s){++p;return true;} return false; }
  bool isOp(const std::string& s) const { return cur().t==Tok::OP&&cur().s==s; }

  Num primary() {
    if (cur().t==Tok::NUM) { const Rational v=cur().v; ++p; return fromR(v); }
    if (eat("(")) { Num n=expr(); if(!eat(")")) ok=false; return n; }
    if (cur().t==Tok::ID) {
      const std::string id=cur().s; ++p;
      std::string fname=id; bool deriv=false;
      if (id.size()>1 && id.back()=='\'') { fname=id.substr(0,id.size()-1); deriv=true; }
      if (cur().t==Tok::LP && (FUNCS.count(id)||funcs.count(id)||(deriv&&funcs.count(fname)))) {
        ++p; std::vector<Num> args;
        if (!eat(")")) { for(;;){ args.push_back(expr()); if(eat(","))continue; if(eat(")"))break;
            ok=false; break; } }
        if (deriv && funcs.count(fname) && args.size()==1 && args[0].k==Num::RAT) {
          const std::string par=paramOf(fname);
          Poly d=pDerive(funcs[fname],par); bool good=true;
          std::map<std::string,Num> e; e[par]=args[0];
          Rational v=pEvalAt(d,e,good); return good?fromR(v):und();
        }
        return call(id,args);
      }
      if (id=="pi"||id=="Pi") return sym("pi");
      if (id=="e"||id=="E") return sym("e");
      if (id=="i"||id=="I") { Num n;n.k=Num::CPX;n.a=rat(0,1).a;n.im=rat(1,1).a;return n; }
      if (id=="oo"||id=="inf"||id=="Inf") return infinity();
      auto it=env.find(id); if(it!=env.end()) return it->second;
      auto sq=seqs.find(id); if(sq!=seqs.end()) { /* a_n 由 sum 上下文代入 */ }
      return PolyVarAsNum(id);
    }
    ok=false; ++p; return und();
  }
  Num PolyVarAsNum(const std::string& id) {
    // 未绑定变量：保留为符号多项式值（id: 走另一条路）；数值路径弃权
    if (id=="x"||id=="y"||id=="z"||id=="n"||id=="k"||id=="m"||id=="a"||id=="b"||id=="c"||id=="t") {
      // 若环境无绑定，返回 UND（诚实）；sum/derive 走专用路径
      return und();
    }
    return und();
  }
  Num call(const std::string& id,const std::vector<Num>& a) {
    const Num& u=rat(0,1);
    (void)u;
    auto need=[&](std::size_t n){ return a.size()==n&&!a.empty(); };
    if (id=="sqrt") { if(!need(1)){ok=false;return und();} 
      if(a[0].k==Num::RAT){
        /* 0.1.1 S1：负完全平方 → 纯虚复数（sqrt(-1)=i）；
         * 负非完全平方是 根式×i 混合形态，超出 CPX（虚部纯有理）能力域，诚实弃权。 */
        if(a[0].a.p<0){
          Rational nx; if(!ratNeg(a[0].a,nx)) return und();
          Num s=sqrtR(nx);
          if(s.k==Num::RAT){ Num n; n.k=Num::CPX; n.a=rat(0,1).a; n.im=s.a; return n; }
          return und();
        }
        return sqrtR(a[0].a);
      } return und(); }
    if (id=="abs") { if(!need(1)){ok=false;return und();}
      if(a[0].k==Num::RAT){ return a[0].a.p<0?neg(a[0]):a[0]; } return und(); }
    if (id=="sin"||id=="cos"||id=="tan") { if(!need(1)){ok=false;return und();} return trigValue(id,a[0]); }
    if (id=="gcd"||id=="lcm"||id=="modpow"||id=="C"||id=="P"||id=="phi"||id=="nextprime"||id=="factors"||id=="isprime") {
      return numth(id,a);
    }
    if (id=="mean"||id=="var"||id=="cross"||id=="dot"||id=="L") return und(); // 由行层专门解析
    auto f=funcs.find(id);
    if (f!=funcs.end()&&a.size()==1&&a[0].k==Num::RAT) {
      bool good=true; std::map<std::string,Num> e; e[paramOf(id)]=a[0];
      Rational v=pEvalAt(f->second,e,good); return good?fromR(v):und();
    }
    ok=false; return und();
  }
  static std::uint64_t mulmod(std::uint64_t a,std::uint64_t b,std::uint64_t m){
    std::uint64_t r=0; a%=m;
    while(b){ if(b&1) r=(r+a)%m; a=(a*2)%m; b>>=1; } return r; }
  static std::uint64_t powmod(std::uint64_t a,std::uint64_t e,std::uint64_t m){
    std::uint64_t r=1%m; a%=m;
    while(e){ if(e&1) r=mulmod(r,a,m); a=mulmod(a,a,m); e>>=1; } return r; }
  static bool isPrime64(std::uint64_t n){
    static const std::uint64_t bases[]={2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL,19ULL,23ULL,29ULL,31ULL,37ULL,0ULL};
    if(n<2)return false;
    for(int i=0;bases[i];++i){ if(n%bases[i]==0) return n==bases[i]; }
    std::uint64_t d=n-1,s=0; while((d&1)==0){d>>=1;++s;}
    for(int i=0;bases[i];++i){ const std::uint64_t a=bases[i]; if(a>=n)continue;
      std::uint64_t x=powmod(a,d,n); if(x==1||x==n-1)continue;
      bool witness=false;
      for(std::uint64_t r=1;r<s;++r){ x=mulmod(x,x,n); if(x==n-1){witness=true;break;} }
      if(!witness){return false;} }
    return true; }
  Num numth(const std::string& id,const std::vector<Num>& a) {
    if (a.empty()||a[0].k!=Num::RAT||a[0].a.q!=1) {ok=false;return und();}
    const std::int64_t x=a[0].a.p;
    auto iarg=[&](std::size_t k)->std::int64_t{ return (a.size()>k&&a[k].k==Num::RAT&&a[k].a.q==1)?a[k].a.p:0; };
    if(id=="isprime"){ return v6::boolean(isPrime64((std::uint64_t)x)); }
    if(id=="nextprime"){ std::int64_t n=x+1;
      for(;n<100000000LL;++n){ if(isPrime64((std::uint64_t)n)) return rat(n); }
      return und(); }
    if(id=="phi"){ std::int64_t n=x,r=n; for(std::int64_t d=2;d*d<=n;++d)if(n%d==0){while(n%d==0)n/=d;r-=r/d;}if(n>1)r-=r/n;return rat(r); }
    if(id=="gcd"||id=="lcm"){ if(a.size()<2){return und();} std::int64_t u=x,v=iarg(1);
      while(v){std::int64_t w=u%v;u=v;v=w;} if(u<0)u=-u;
      if(id=="gcd"){return rat(u);} if(u==0)return und();
      std::int64_t q1=x/u,q2=iarg(1); if(q2!=0&&q1>INT64_MAX/q2)return und(); return rat(q1*q2); }
    if(id=="modpow"){ std::int64_t b=x,e=iarg(1),m=iarg(2),r=1%m;
      if(m<=0||e<0){return und();} b%=m; while(e){if(e&1)r=(r*b)%m;b=(b*b)%m;e>>=1;}return rat(r); }
    if(id=="C"||id=="P"){ std::int64_t n=x,k=iarg(1); if(n<0||k<0||n>60)return und();
      if(k>n) return rat(0);   /* 0.1.1 S2：组合学惯例 C(n,k)=P(n,k)=0（k>n），不再弃权 */
      std::int64_t r=1; for(std::int64_t j=1;j<=k;++j){ r=r*(n-j+1)/j; }
      if(id=="P"){ std::int64_t q=1; for(std::int64_t j=0;j<k;++j){ if(q>INT64_MAX/(n-j))return und(); q*=(n-j);} return rat(q); }
      return rat(r); }
    if(id=="factors"){ // 仅把结果作为展示串由行层处理，数值路径返回 UND 由专用行处理
      return und(); }
    ok=false; return und();
  }
  Num postfix() {
    Num n=primary();
    for(;;) {
      if (eat("!")) { if(n.k==Num::RAT&&n.a.q==1&&n.a.p>=0&&n.a.p<=20){ std::int64_t f=1;
          for(std::int64_t j=2;j<=n.a.p;++j){ f*=j; } n=rat(f);} else n=und(); continue; }
      if (eat("^")) { Num e=unary(); if(e.k!=Num::RAT||e.a.q!=1){n=und();continue;}   /* 0.1.1 S3：指数允许一元负号（2^-1），右结合不变 */
        n=power(n,e.a.p); continue; }
      if (isOp("'")) { ++p; continue; }                    // f'(x) 的撇号，derive 行走专用路径
      break;
    }
    return n;
  }
  Num unary() { if(eat("-"))return neg(postfix()); if(eat("+"))return postfix(); return postfix(); }
  Num mul() {
    Num n=unary();
    for(;;){
      if(eat("*")){ n=mul2(n,unary()); continue; }
      if(eat("/")){ Num d=unary(); n=divv(n,d); continue; }
      // 隐式乘：2x / x(x+1) / 2(x+1) / 2sin(pi/6) / 2sqrt(2)
      // 0.1.1 bugfix：遇函数名必须交给 primary() 正常吃括号并 call()；
      // 早先直接 break 会把剩余 token 留在流里 → 静默截断成前导因子（假 True / 假 False）。
      if(cur().t==Tok::NUM||cur().t==Tok::ID||cur().t==Tok::LP||cur().t==Tok::LB){
        n=mul2(n,unary()); continue; }
      break;
    }
    return n;
  }
  Num mul2(const Num&a,const Num&b){ return v6::mul(a,b); }
  Num add() { Num n=mul();
    for(;;){ if(eat("+")){n=add2(n,mul());continue;} if(eat("-")){n=sub(n,mul());continue;} break; } return n; }
  Num add2(const Num&a,const Num&b){ return v6::add(a,b); }
  Num expr() {
    DepthGuard g(depth); if(g.over){ ok=false; return und(); }
    Num n=add();
    static const char* cops[]={"==","=","!=","<=",">=","<",">",nullptr};
    for(int k=0;cops[k];++k){ if(eat(cops[k])){ Num r=add(); const int c=cmp(n,r);
        if(c==2) return undProp(n,r);   /* 0.1.1：比较不可判时保留不定式原因 */
        const std::string o=cops[k];
        if(o=="=="||o=="=") return v6::boolean(c==0);
        if(o=="!=") return v6::boolean(c!=0);
        if(o=="<") return v6::boolean(c<0);
        if(o==">") return v6::boolean(c>0);
        if(o=="<=") return v6::boolean(c<=0);
        return v6::boolean(c>=0); } }
    return n;
  }
  Num run(){ Num n=expr(); if(!ok) return und(); return n; }
};

/* 符号多项式解析（id: / derive: / solve:） */
struct PolyParser {
  std::vector<Tok> t; std::size_t p=0; bool ok=true;
  /* 0.1.1 bugfix B4：符号多项式解析同样加深度护栏 */
  int depth=0;
  struct DepthGuard{ int& d; bool over; explicit DepthGuard(int& dd):d(dd){over=(++d>1024);} ~DepthGuard(){--d;} };
  bool lastSplit=false; std::vector<std::string> splitVars;  // 0.0.9b：^ 绑定拆分末变量
  explicit PolyParser(const std::string& s):t(lex(s)){}
  const Tok& cur() const { return t[p<t.size()?p:t.size()-1]; }
  bool eat(const std::string& s){ if(cur().s==s){++p;return true;}return false; }
  Poly prim(){ if(cur().t==Tok::NUM){auto v=cur().v;++p;lastSplit=false;Poly z;z.t[Monom{}]=v;return z;}
    if(eat("(")){lastSplit=false;Poly z=add();if(!eat(")"))ok=false;return z;}
    if(cur().t==Tok::ID){
      std::string id=cur().s; ++p;
      /* 0.1.0：多项式里的连写变量 2xy → 2*x*y（保留函数名 / 常量 / 已知单名） */
      static const std::set<std::string> KEEP={
        "pi","e","i","oo","N","R","Z","Q","omega","aleph0","aleph1",
        "emptyset","cfrak","True","False","sqrt","abs","C","P","phi",
        "sin","cos","tan","isprime","nextprime","factors","gcd","lcm","modpow"};
      bool allAlpha=!id.empty();
      for(char ch:id) if(!isalpha((unsigned char)ch)){ allAlpha=false; break; }
      if(allAlpha && id.size()>1 && !KEEP.count(id) && !FUNCS.count(id)){
        lastSplit=true; splitVars.clear();
        Poly z=Poly::c(1);
        for(char ch:id){ z=pMul(z,Poly::v(std::string(1,ch))); splitVars.push_back(std::string(1,ch)); }
        return z;
      }
      lastSplit=false;
      return Poly::v(id);
    }
    ok=false;++p;lastSplit=false;return Poly::c(0); }
  Poly post(){ Poly n=prim();
    for(;;){ if(eat("^")){ if(cur().t!=Tok::NUM){ok=false;return n;} std::int64_t e=(std::int64_t)cur().v.p;++p;
        /* 0.0.9b：^ 只绑拆分组的最后一个变量——xy^2 = x*y^2，不是 (xy)^2 */
        if(lastSplit && splitVars.size()>=2){
          Poly z=Poly::c(1);
          for(std::size_t j=0;j+1<splitVars.size();++j) z=pMul(z,Poly::v(splitVars[j]));
          z=pMul(z,pPow(Poly::v(splitVars.back()),e));
          n=z;
        } else n=pPow(n,e);
        lastSplit=false; continue;} break; } return n; }
  Poly un(){ if(eat("-"))return pNeg(post()); if(eat("+"))return post(); return post(); }
  Poly mm(){ Poly n=un();
    for(;;){ if(eat("*")){n=pMul(n,un());continue;} if(eat("/")){ok=false;return n;}
      if(cur().t==Tok::NUM||cur().t==Tok::ID||cur().t==Tok::LP){n=pMul(n,un());continue;} break; } return n; }
  Poly add(){ Poly n=mm();
    for(;;){ if(eat("+")){n=pAdd(n,mm());continue;} if(eat("-")){n=pSub(n,mm());continue;} break; } return n; }
  Poly run(){ Poly z=add(); if(!ok||cur().t!=Tok::END) z.ok=false; return z; }
};

/* ══════════════ 行层上下文 ══════════════ */
/* ══════════════ 0.1.0 进制眼镜：只换「数字字面量怎么读」，不换世界 ══════════════
 * 换的是词法层眼镜：base=2 时 1010 → 10，base=3 时 0.1 → (1/3)，
 * 全部折成精确有理数表达式再交给原 Parser；core/ 十一件一字不动。
 * 符号常量 / 函数名 / 标识符（2xy、pi/12、0x..）不参与重解析。 */
static int digitVal(char ch){
  if(ch>='0'&&ch<='9') return ch-'0';
  if(ch>='a'&&ch<='f') return ch-'a'+10;
  if(ch>='A'&&ch<='F') return ch-'A'+10;
  return -1;
}
static int radixBase(const std::string& name){
  const std::size_t p=name.find("_nary");
  if(p!=std::string::npos&&p>0){
    const std::string d=name.substr(0,p);
    bool ok=!d.empty(); for(char ch:d) if(!isdigit((unsigned char)ch)) ok=false;
    if(ok){ const int b=std::atoi(d.c_str()); if(b>=2&&b<=16) return b; }
  }
  if(name=="二进制") return 2;
  if(name=="三进制") return 3;
  if(name=="八进制") return 8;
  if(name=="十六进制") return 16;
  if(name.rfind("base=",0)==0){
    const std::string d=name.substr(5);
    bool ok=!d.empty(); for(char ch:d) if(!isdigit((unsigned char)ch)) ok=false;
    if(ok){ const int b=std::atoi(d.c_str()); if(b>=2&&b<=16) return b; }
  }
  return 0;
}
static bool radixNorm(const std::string& s,int base,std::string& out,std::string& err){
  out.clear(); err.clear();
  std::size_t i=0;
  while(i<s.size()){
    const char ch=s[i];
    const bool start = isdigit((unsigned char)ch) ||
                       (ch=='.' && i+1<s.size() && isdigit((unsigned char)s[i+1]));
    const bool prevOk = (i==0) || !(isalnum((unsigned char)s[i-1])||s[i-1]=='_'||s[i-1]=='.');
    if(!(start&&prevOk)){ out.push_back(ch); ++i; continue; }
    std::size_t j=i;
    while(j<s.size() && (digitVal(s[j])>=0||s[j]=='.')) ++j;
    const bool nextOk = (j>=s.size()) || !(isalpha((unsigned char)s[j])||s[j]=='_');
    if(!nextOk){ out.append(s,i,j-i); i=j; continue; }   // 2xy / 0x.. 属标识符，不换眼镜
    const std::string tok=s.substr(i,j-i);
    if(std::count(tok.begin(),tok.end(),'.')>1){ out.append(tok); i=j; continue; } // 1..10 不误伤
    std::string ip,fp; bool dot=false;
    for(char c:tok){ if(c=='.'){dot=true;continue;} (dot?fp:ip).push_back(c); }
    if(ip.empty()&&fp.empty()){ out.append(tok); i=j; continue; }
    for(char c:ip) if(digitVal(c)<0||digitVal(c)>=base){ err="E-LITERAL-BASE"; return false; }
    for(char c:fp) if(digitVal(c)<0||digitVal(c)>=base){ err="E-LITERAL-BASE"; return false; }
    __int128 v=0;
    for(char c:ip){ v=v*base+digitVal(c); if(v>((__int128)1<<62)){ err="E-OVF"; return false; } }
    if(!dot){ out += std::to_string((long long)v); }
    else {
      __int128 den=1,num=0;
      for(char c:fp){ num=num*base+digitVal(c); den*=base;
                      if(den>((__int128)1<<62)){ err="E-OVF"; return false; } }
      const __int128 total=v*den+num;
      out += "(" + std::to_string((long long)total) + "/" +
             std::to_string((long long)den) + ")";
    }
    i=j;
  }
  return true;
}

struct Ctx {
  /* 0.2.0 元编程：登记容器与有限步开关（不产三态、不执行自产代码） */
  std::vector<std::string> gaps, reformulations, quotes;
  std::map<std::string,std::string> gapSeen;
  int expandDepth=8;
  bool traceOn=false;
  /* 0.2.0 B：元编程锚护栏（gap 需 sig+by 锚，reformulate 需对应 gap） */
  bool hasBy=false;
  std::vector<std::pair<int,std::string>> gapTrace, reformTrace;
  std::string reflGaps;
  std::map<std::string,Num> env;
  std::map<std::string,Poly> funcs;
  std::map<std::string,std::pair<std::string,std::string>> seqs;
  std::map<std::string,std::vector<Num>> sets;
  std::vector<std::string> givens,cases,checks;
  int registered=0;
  bool chinese=false;
  std::string codesPath="lab_codes.tsv";

  // ── 0.0.6 强化修复版：标签体系与领域（乙裁决并回，非替换） ──
  std::vector<std::string> containers;            // 具名容器栈：<a>/<calc>/<C_and_E>/<answer>/<s>/<subject>
  std::string domain="math";                     // 当前领域（由 <subject domain="..."> 或 domain: 设定）
  std::map<std::string,std::string> domainNames; // domains.tsv 注册表
  std::set<std::string> ledgerDomains;           // 甲：标 [台账] 的域（只有名字，无判定核）
  std::vector<std::string> domainsSeen;          // 卷内出现过的领域（含未知领域，原样登记）
  std::vector<std::string> decorations;          // S:/E:/V:/P:/Q: 装饰简写，机器跳过、原样保留
  std::vector<std::string> preserved;            // 容器内中文装饰词与原文，渲染层可查
  std::vector<std::map<std::string,std::string>> inferences; // <C_and_E> 的 cause/effect/by 三字段
  std::vector<std::string> answers;              // A: 归一为 <answer> 后的答卷块
  std::vector<std::map<std::string,Num>> envStack;  // 容器闭合时恢复，避免「设 x=5」跨容器泄漏
  // ── 0.0.7 集合论实验区：地图坐标 / 公理表 / 引用 / 模块 / 集合定义 ──
  std::string system="ZF", choice="unset";
  std::vector<std::string> axiomsSeen, refsSeen, byRefs, refUnknown;
  std::vector<std::string> loadedPresets, loadDiags;
  std::set<std::string> loadedPaths;
  std::map<std::string,std::string> setDefs;   // 构造式（只登记，不猜通项）
  /* 0.0.7 补丁二：悖论集 / 上一行断言 / 有限映射表（四私货共用） */
  std::set<std::string> paradoxSets;
  std::string lastExpr;
  std::map<std::string,std::vector<std::pair<std::string,std::string>>> mappings;
  std::map<std::string,std::vector<std::string>> codomains;
  std::vector<std::string> problems, theories, maps, certificates, witnesses;
  bool isModule=false;
  bool systemDeclared=false;
  /* 0.0.8：形式化定义（签名/类型/前置条件/规则/BNF/有限步 δ 展开）与多宇宙图册 */
  std::map<std::string,std::string> defSigs, defTypes, defBodies, defParams;
  std::vector<std::string> defWhere, rules, grammars, worldEdges;
  std::set<std::string> worlds;
  bool multiverseOn=false;
  /* 0.1.0：进制眼镜栈 / 真类名册 / 台账登记（全部只登记，不构造判定核） */
  int base=10;
  std::vector<int> baseStack;
  std::set<std::string> properClasses;
  std::vector<std::string> ledger;
  int sandFix=0;
  /* 0.1.0 A 增量：公理本体化（只登记，不构造判定核） */
  std::vector<std::pair<std::string,std::string>> axiomBodies; // name=formula，保留全部声明
  std::string langSig;                 // lang: 语言签名，按声明顺序以 " | " 拼接
  std::string theorySigHex;            // 最近一次内容指纹（FNV-1a 64，16 位 hex）
  bool theorySigAsked=false;           // 卷内是否出现过 theory_sig:
  int sigMismatch=0;                   // 期望指纹不符次数：只登记，绝不产三态
  /* 强度草案试验：v1 含公理名（默认，零回归）；v2 只认公理公式本体。
   * strengthLabels 是展示型值标签，只登记、不进指纹、不产三态。 */
  std::string sigMode="named";
  std::vector<std::string> strengthLabels;
  /* ── 0.2.1 重制：四工位各有一个容器，零复用旧词语义 ──
   * shape: 认形状 / bind: 补槽位 / rewrite: 摊开回投 / split: 拆账本；mode: 试验门禁。
   * 这些容器只装「写法与登记」，不装真值；True/False 永远只由原判定核写。 */
  struct ShapeDef { std::string id, head, target; std::vector<std::string> slots; };
  std::map<std::string,ShapeDef> shapes;
  std::map<std::string,std::vector<std::string>> binds;
  std::set<std::string> bindSeen;
  std::map<std::string,std::string> reformNames;   // fn -> 规则原文；撞名即弃权，绝不 last-wins
  std::vector<std::string> splits;
  std::string reflExtra;                  /* 0.2.1 r2 / bug B：reflect 新目标的人话档出口 */
  std::string metaMode="strict";
  int rewriteOk=0, rewriteRejected=0, illegalHits=0;
  std::vector<std::string> rewriteTraceAll;
  /* 回投队列：(产物行, 触发行号)。主循环直接 handle，不插行、不漂移 file_line。 */
  std::vector<std::pair<std::string,int>> replayQueue;
  bool inContainer() const { return !containers.empty(); }
  std::string curContainer() const { return containers.empty()?"":containers.back(); }
};
struct Result { int state=0; std::string line, out, code, detail, tag; int fileLine=0; int derivedFrom=0; }; // 1=True 0=False 2=UNDECIDED 3=ERROR -1=登记

/* ══════════ 0.1.0 A 增量：公理本体化（只登记，不构造判定核） ══════════
 * 立法点：一致性强度由 lang + 公理公式 + 推导规则这套本体给出，不是名字给出。
 * 旧写法 axiom: AC, Separation 逐字保留；新写法 axiom: Sep = <公式> 把公式本体入册。
 * 公式内请避免半角逗号（用中文逗号或分号），逗号仍是列表分隔符。
 */
static void absorbAxiomList(Ctx& c,const std::string& b){
  std::stringstream ss(b); std::string x;
  while(std::getline(ss,x,',')){
    x=trim(x); if(x.empty()) continue;
    const std::size_t eq=x.find('=');
    if(eq==std::string::npos){ c.axiomsSeen.push_back(x); c.axiomBodies.push_back({x,""}); }
    else {
      const std::string nm=trim(x.substr(0,eq)), fm=trim(x.substr(eq+1));
      if(nm.empty()) continue;
      c.axiomsSeen.push_back(nm); c.axiomBodies.push_back({nm,fm});
    }
  }
}
/* FNV-1a 64：纯整数位运算，零浮点、跨平台一致；同一份卷重跑逐字一样。
 * 输入按字典序归一，声明/加载顺序不影响指纹 —— 认的是内容，不是书写次序。 */
static std::string computeTheorySig(const Ctx& c){
  /* 试验开关：v2 只喂公式本体，公理名这个符号不再参与 hash；默认 v1 逐字保留 0.1.1 输出。 */
  const bool formulaOnly=(c.sigMode=="formula");
  std::vector<std::string> ax,rl;
  /* 0.1.1 ST 补漏：v2 下空本体（纯名字）公理回退用名字参与 hash——
   * 否则不同理论的纯名字公理列表全塌成空串，指纹退化碰撞、sig_mismatch 失灵。 */
  for(const auto&p:c.axiomBodies) ax.push_back(formulaOnly?(p.second.empty()?(p.first+"="):p.second):(p.first+"="+p.second));
  std::sort(ax.begin(),ax.end());
  rl=c.rules; std::sort(rl.begin(),rl.end());
  std::string s=std::string(formulaOnly?"LTIR-THEORY-SIG-v2\n":"LTIR-THEORY-SIG-v1\n")+"lang:"+c.langSig+"\n";
  for(const auto&a:ax) s+="axiom:"+a+"\n";
  for(const auto&r:rl) s+="rule:"+r+"\n";
  std::uint64_t h=1469598103934665603ULL;
  for(unsigned char ch:s){ h^=ch; h*=1099511628211ULL; }
  std::ostringstream os; os<<std::hex<<std::setw(16)<<std::setfill('0')<<h;
  return os.str();
}
static std::string jesc(const std::string& s){
  std::string o; for(char ch:s){ if(ch=='"'||ch=='\\') o+='\\'; o+=ch; } return o;
}

/* 领域注册表：语法层资产，非错误码表；本令授权扩增，不改 lab_codes.tsv */
static void loadDomains(Ctx& c,const std::string& path) {
  std::ifstream f(path);
  std::string ln;
  while (std::getline(f,ln)) {
    if (ln.empty()||ln[0]=='#') continue;
    std::vector<std::string> v; std::stringstream ss(ln); std::string x;
    while (std::getline(ss,x,'\t')) v.push_back(x);
    if (v.size()>=2 && v[0]!="name") {
      c.domainNames[v[0]]=v[1];
      if (v.size()>=3 && v[2].find("[台账]")!=std::string::npos)
        c.ledgerDomains.insert(v[0]);
    }
  }
}
static void setDomain(Ctx& c,const std::string& d) {
  c.domain=d;
  c.domainsSeen.push_back(d);            // 含未知领域：原样登记、不拦截不报错不解释
}

static std::vector<std::string> splitTop(const std::string& s,char sep) {
  std::vector<std::string> r; int d=0; std::string c;
  for(char ch:s){ if(ch=='('||ch=='['||ch=='{'){++d;c.push_back(ch);}
    else if(ch==')'||ch==']'||ch=='}'){--d;c.push_back(ch);}
    else if(ch==sep&&d==0){r.push_back(trim(c));c.clear();} else c.push_back(ch); }
  if(!trim(c).empty()){ r.push_back(trim(c)); }
  return r;
}
static std::vector<Num> parseList(Ctx& c,const std::string& s) {
  std::vector<Num> r;
  const std::size_t b=s.find('['), e=s.rfind(']');
  const std::string inner = (b!=std::string::npos&&e!=std::string::npos&&e>b)?s.substr(b+1,e-b-1):s;
  for(auto&part:splitTop(inner,',')){ Parser q(part);q.env=c.env;q.funcs=c.funcs;q.seqs=c.seqs;q.defParams=c.defParams;r.push_back(q.run()); }
  return r;
}
static Num evalExpr(Ctx& c,const std::string& s) {
  Parser q(s); q.env=c.env; q.funcs=c.funcs; q.seqs=c.seqs; q.defParams=c.defParams; return q.run();
}
static bool findVerdict(const std::string& body,std::string& lhs,bool& claim) {
  const std::size_t q=body.rfind("->");
  if(q==std::string::npos) return false;
  std::string tail=trim(body.substr(q+2));
  std::size_t cut=tail.find_first_of(" \t\r\n.,;:!?");
  for(const char* pu:{"。","，","．","：","；"}) { const std::size_t z=tail.find(pu);
    if(z!=std::string::npos&&(cut==std::string::npos||z<cut))cut=z; }
  const std::string w=trim(cut==std::string::npos?tail:tail.substr(0,cut));
  if(w=="True"||w=="true"||w=="成立"||w=="真") claim=true;
  else if(w=="False"||w=="false"||w=="不成立"||w=="假") claim=false;
  else return false;
  lhs=trim(body.substr(0,q)); return true;
}
/* 0.0.6 强化修复版：容器内「一段」→「多段」状态机。
 * 中文 / 中文标点当分隔符，收集全部 ASCII 算式段，不再只挑最长的一段 —— 这正是
 * L06「设 x=5 ... A: x^2 = 25」被丢掉代入值的根因。 */
static std::vector<std::string> splitSegments(const std::string& s) {
  std::vector<std::string> segs; std::string cur;
  for(std::size_t i=0;i<s.size();){
    const unsigned char ch=(unsigned char)s[i];
    if(ch>=0x80u){
      if(!trim(cur).empty()) segs.push_back(trim(cur));
      cur.clear();
      i += (ch>=0xF0u?4:(ch>=0xE0u?3:(ch>=0xC0u?2:1)));
      continue;
    }
    cur.push_back(s[i]); ++i;
  }
  if(!trim(cur).empty()) segs.push_back(trim(cur));
  // 丢弃纯标点段（中文标点归一后残留的 ':' ',' '.'），保住含算式/关键字的段
  std::vector<std::string> out;
  for(auto&x:segs){
    bool has=false; for(char ch:x) if(isalnum((unsigned char)ch)){has=true;break;}
    if(has) out.push_back(x);
  }
  return out;
}
static bool isIdent(const std::string& s) {
  if(s.empty()||!(isalpha((unsigned char)s[0])||s[0]=='_')) return false;
  for(char ch:s) if(!(isalnum((unsigned char)ch)||ch=='_')) return false;
  return true;
}
/* 「设 x=5」/「令 x=5」/「取 x=5」：中文已被切成分隔符，落到这里是纯 `x=5` 段。
 * 只认「单标识符 = 可求值数值」，不认 x^2=25 / 2+2=4 / 1/3=1/2，避免误绑。 */
static bool tryBindEnv(Ctx& c,const std::string& seg) {
  if(seg.find("->")!=std::string::npos) return false;
  const std::size_t q=seg.find('=');
  if(q==std::string::npos) return false;
  if(q+1<seg.size()&&seg[q+1]=='=') return false;
  if(q>0&&(seg[q-1]=='<'||seg[q-1]=='>'||seg[q-1]=='!')) return false;
  const std::string lhs=trim(seg.substr(0,q)), rhs=trim(seg.substr(q+1));
  if(!isIdent(lhs)) return false;
  if(lhs=="pi"||lhs=="e"||lhs=="i"||lhs=="oo"||lhs=="inf") return false; // 常量身份不可覆写
  Num v=evalExpr(c,rhs);
  if(v.k==Num::UND||v.k==Num::SYM||v.k==Num::INF) return false;
  c.env[lhs]=v;
  return true;
}


/* ══════════════ 0.0.7 有限集精确运算（无限/构造/嵌套一律诚实弃权） ══════════════ */
struct SetR { bool ok=false, infinite=false, nested=false, unknown=false;
              bool boolean=false; std::vector<std::string> elems; bool bv=false;
              std::string scalar; };
static std::string setKey(const Num& v){ return numStr(v); }
static SetR setAtom(Ctx& c, std::string e) {
  SetR r; e=trim(e);
  if(e.empty()){ r.unknown=true; return r; }
  if(e[0]=='{'){
    const std::size_t rb=e.rfind('}');
    if(rb==std::string::npos||rb==0){ r.unknown=true; return r; }
    const std::string inner=e.substr(1,rb-1);
    if(inner.find('{')!=std::string::npos){ r.nested=true; r.ok=true; return r; }
    if(inner.find(':')!=std::string::npos||inner.find("...")!=std::string::npos||
       inner.find(" in ")!=std::string::npos||inner.find("forall")!=std::string::npos){
      r.infinite=true; r.ok=true; return r; }
    std::vector<std::string> parts; std::stringstream ss(inner); std::string x;
    while(std::getline(ss,x,',')) parts.push_back(trim(x));
    for(auto&p:parts){ if(p.empty())continue;
      Num v=evalExpr(c,p); if(v.k==Num::UND){r.unknown=true;return r;}
      r.elems.push_back(setKey(v)); }
    std::sort(r.elems.begin(),r.elems.end());
    r.elems.erase(std::unique(r.elems.begin(),r.elems.end()),r.elems.end());
    r.ok=true; return r;
  }
  // card(A) / |A|
  if(e.rfind("card(",0)==0 && e.size()>6){
    std::string inner=trim(e.substr(5,e.size()-6));
    /* 甲补丁：字面集合 / 空集直接按元素数取势，不再误报 E-REF-UNKNOWN */
    if(inner.rfind("{",0)==0 || inner=="emptyset" || inner=="∅"){
      SetR base=setAtom(c,inner=="∅"?"emptyset":inner);
      if(base.nested){ r.nested=true; r.ok=true; return r; }
      if(base.infinite){ r.infinite=true; r.ok=true; return r; }
      if(!base.ok||base.unknown){ r.unknown=true; return r; }
      r.ok=true; r.scalar=std::to_string(base.elems.size()); return r;
    }
    auto it=c.sets.find(inner);
    if(it!=c.sets.end()){
      std::vector<std::string> v; for(auto&x:it->second)v.push_back(setKey(x));
      std::sort(v.begin(),v.end()); v.erase(std::unique(v.begin(),v.end()),v.end());
      r.ok=true; r.scalar=std::to_string(v.size()); return r; }
    if(c.setDefs.count(inner)){ r.infinite=true; r.ok=true; return r; }
    r.unknown=true; return r;
  }
  if(e.size()>2 && e[0]=='|' && e.back()=='|'){
    const std::string inner=trim(e.substr(1,e.size()-2));
    return setAtom(c,"card("+inner+")");
  }
  auto it=c.sets.find(e);
  if(it!=c.sets.end()){
    for(auto&x:it->second)r.elems.push_back(setKey(x));
    std::sort(r.elems.begin(),r.elems.end());
    r.elems.erase(std::unique(r.elems.begin(),r.elems.end()),r.elems.end());
    r.ok=true; return r; }
  if(c.setDefs.count(e)){ r.infinite=true; r.ok=true; return r; }
  if(e=="emptyset"||e=="∅"){ r.ok=true; return r; }
  if(e=="V"||e=="Omega"||e=="aleph0"||e=="cfrak"||e=="omega"||
     e=="N"||e=="R"||e=="Z"||e=="Q"){ r.infinite=true; r.ok=true; return r; }
  // 单个数值当元素（x in A）
  Num v=evalExpr(c,e);
  if(v.k!=Num::UND){ r.ok=true; r.scalar=setKey(v); return r; }
  r.unknown=true; return r;
}
static SetR setUnion(const SetR&a,const SetR&b,char mode){
  SetR r; r.ok=true;
  if(a.infinite||b.infinite){r.infinite=true;return r;}
  if(a.nested||b.nested){r.nested=true;return r;}
  if(mode=='u'){ r.elems=a.elems; for(auto&x:b.elems)r.elems.push_back(x); }
  else if(mode=='i'){ for(auto&x:a.elems) if(std::find(b.elems.begin(),b.elems.end(),x)!=b.elems.end()) r.elems.push_back(x); }
  else { for(auto&x:a.elems) if(std::find(b.elems.begin(),b.elems.end(),x)==b.elems.end()) r.elems.push_back(x); }
  std::sort(r.elems.begin(),r.elems.end());
  r.elems.erase(std::unique(r.elems.begin(),r.elems.end()),r.elems.end());
  return r;
}
static std::string setStr(const SetR&r){
  std::string s="{"; for(std::size_t i=0;i<r.elems.size();++i){if(i)s+=", ";s+=r.elems[i];} s+="}"; return s;
}
/* setSide：先规约二元集合运算，再落到原子（支持 A union B == {...} 链式） */
static SetR setSide(Ctx& c, std::string e) {
  e=trim(e);
  struct Op { const char* s; char m; };
  static const Op BIN[]={{" union ",'u'},{" inter ",'i'},{" diff ",'d'},{nullptr,0}};
  for(int k=0;BIN[k].s;++k){
    const std::string o=BIN[k].s;
    const std::size_t q=e.rfind(o);
    if(q==std::string::npos) continue;
    SetR a=setSide(c,e.substr(0,q)), b=setSide(c,e.substr(q+o.size()));
    if(!a.ok||!b.ok||a.unknown||b.unknown){ SetR z; z.unknown=true; return z; }
    return setUnion(a,b,BIN[k].m);
  }
  return setAtom(c,e);
}
/* 返回 ok=true 表示「本行该走集合语义」；结果写进 R */
template<typename CodeFn>
static bool evalSetAssert(Ctx& c, const std::string& expr, Result& R,
                          const CodeFn& codeRes) {
  static const char* TRIG[] = {" union "," inter "," diff "," subseteq "," subset ",
                               " supseteq "," supset "," in "," notin ","card(","|",
                               "{", nullptr};
  bool hit=false; for(int k=0;TRIG[k];++k) if(expr.find(TRIG[k])!=std::string::npos){hit=true;break;}
  if(!hit) return false;
  std::string e=expr;
  const std::size_t vd=e.find(" vdash ");
  if(vd!=std::string::npos) e=trim(e.substr(vd+7));
  auto sz=[&](const SetR& x)->long long{
    if(!x.scalar.empty()) return std::atoll(x.scalar.c_str());
    return (long long)x.elems.size(); };
  auto bail=[&](const SetR& a,const SetR& b)->bool{
    if(!a.ok||!b.ok||a.unknown||b.unknown) return false;
    if(a.infinite||b.infinite){ R=codeRes("E-SCOPE-INFINITE",2); return true; }
    if(a.nested||b.nested){ R=codeRes("E-SCOPE-NESTED",2); return true; }
    return false; };
  /* 比较 / 势比较 */
  static const char* OPS[]={"==","<=",">=","<",">",nullptr};
  for(int k=0;OPS[k];++k){
    const std::string o=OPS[k]; const std::size_t q=e.find(o);
    if(q==std::string::npos) continue;
    SetR a=setSide(c,trim(e.substr(0,q))), b=setSide(c,trim(e.substr(q+o.size())));
    if(!a.ok||!b.ok||a.unknown||b.unknown) return false;
    if(a.infinite||b.infinite){ R=codeRes("E-SCOPE-INFINITE",2); return true; }
    if(a.nested||b.nested){ R=codeRes("E-SCOPE-NESTED",2); return true; }
    bool bv=false;
    if(o=="=="){
      if(!a.scalar.empty()&&!b.scalar.empty()) bv=(a.scalar==b.scalar);
      else bv=(a.elems==b.elems);
    } else if(o=="<=") bv=(sz(a)<=sz(b));
    else if(o==">=") bv=(sz(a)>=sz(b));
    else if(o=="<")  bv=(sz(a)<sz(b));
    else             bv=(sz(a)>sz(b));
    R.state=bv?1:0; R.out=bv?"True":"False";
    if(!bv)R.code="E-VAL-MISMATCH";
    return true;
  }
  /* 包含 / 属于 */
  struct Rel { const char* s; int m; };
  static const Rel RS[]={{" notin ",1},{" in ",0},{" subseteq ",2},{" subset ",3},
                         {" supseteq ",4},{nullptr,0}};
  for(int k=0;RS[k].s;++k){
    const std::string o=RS[k].s; const std::size_t q=e.find(o);
    if(q==std::string::npos) continue;
    SetR a=setSide(c,trim(e.substr(0,q))), b=setSide(c,trim(e.substr(q+o.size())));
    if(!a.ok||!b.ok||a.unknown||b.unknown) return false;
    if(bail(a,b)) return true;
    bool bv=false;
    if(RS[k].m==0) bv=std::find(b.elems.begin(),b.elems.end(),a.scalar)!=b.elems.end();
    if(RS[k].m==1) bv=std::find(b.elems.begin(),b.elems.end(),a.scalar)==b.elems.end();
    if(RS[k].m==2||RS[k].m==4){
      bool all=true;
      for(auto&x:a.elems) if(std::find(b.elems.begin(),b.elems.end(),x)==b.elems.end()){all=false;break;}
      bv=RS[k].m==2?all:(all&&a.elems.size()<=b.elems.size());
    }
    if(RS[k].m==3){
      bool all=true;
      for(auto&x:a.elems) if(std::find(b.elems.begin(),b.elems.end(),x)==b.elems.end()){all=false;break;}
      bv=all&&a.elems.size()<b.elems.size();
    }
    R.state=bv?1:0; R.out=bv?"True":"False";
    if(!bv)R.code="E-VAL-MISMATCH";
    return true;
  }
  /* 单独集合表达式：输出规约后的集合 / 基数 */
  SetR a=setSide(c,e);
  if(a.infinite){ R=codeRes("E-SCOPE-INFINITE",2); return true; }
  if(a.nested){ R=codeRes("E-SCOPE-NESTED",2); return true; }
  if(!a.ok||a.unknown) return false;
  if(!a.scalar.empty()){ R.state=1; R.out=a.scalar; return true; }
  R.state=1; R.out=setStr(a); return true;
}

/* ══════════ 0.2.1 重制：受控改写层的静态护栏（L2 词表 / L3 兑换权） ══════════
 * 纪律：元操作行只准描述写法，不准夹带判定词、手写结论、自证或关闭目标。
 * 宽档（quote 内层 / witness 体 / 回投产物）允许 V: <表达式>，仍禁手写结论。
 * 旧词零改义：expand: 仍只管 δ 展开，grammar: 仍只收 BNF。 */
static bool metaWordHit(const std::string& s,const char* w){
  const std::size_t n=std::string(w).size();
  std::size_t q=s.find(w);
  while(q!=std::string::npos){
    if(q==0||!isalnum((unsigned char)s[q-1])) return true;
    q=s.find(w,q+n);
  }
  return false;
}
/* 0.2.1 r2 / bug A：禁「一切手写结论」，第三态 UNDECIDED 不再漏。
 * 切词口径与 findVerdict 一致（含中文标点），扫所有 -> 而非只扫最后一个。 */
static bool isVerdictWord(const std::string& w){
  std::string low=w;
  for(char& ch:low) ch=(char)std::tolower((unsigned char)ch);
  return low=="true"||low=="false"||low=="undecided"||
         w=="真"||w=="假"||w=="成立"||w=="不成立"||w=="不知道"||w=="弃权";
}
static std::string arrowTailWord(const std::string& s,std::size_t q){
  std::string tail=trim(s.substr(q+2));
  std::size_t cut=tail.find_first_of(" \t\r\n.,;:!?");
  for(const char* pu:{"。","，","．","：","；"}){
    const std::size_t z=tail.find(pu);
    if(z!=std::string::npos&&(cut==std::string::npos||z<cut)) cut=z;
  }
  return trim(cut==std::string::npos?tail:tail.substr(0,cut));
}
static bool hasVerdictTail(const std::string& s){
  std::size_t q=s.find("->");
  while(q!=std::string::npos){
    if(isVerdictWord(arrowTailWord(s,q))) return true;
    q=s.find("->",q+2);
  }
  return false;
}
static bool metaRedeem(const std::string& s){
  /* 手写 True / False / UNDECIDED 一律先验拒收；不再只查两种。 */
  if(hasVerdictTail(s)) return true;
  static const char* B[]={"A:","assert:","con:","submit:","close:","exact:","goal:","eval:",nullptr};
  for(int i=0;B[i];++i) if(metaWordHit(s,B[i])) return true;
  return false;
}
static bool metaStrictIllegal(const std::string& s){
  if(metaRedeem(s)) return true;
  static const char* B[]={"V:","value:","val:","solve:","expand:","derive:","claim:","forall:",nullptr};
  for(int i=0;B[i];++i) if(metaWordHit(s,B[i])) return true;
  return false;
}
static bool metaProductAllowed(const std::string& L){ return !metaRedeem(L); }
/* 槽位字面量：只收整数 / 单个分数 / 负数。1/2/3 连除一律拒收（bug 7）。 */
static bool slotLiteralOk(const std::string& v){
  if(v.empty()) return false;
  std::size_t i=0; if(v[0]=='-') i=1;
  if(i>=v.size()) return false;
  bool slash=false, digit=false;
  for(;i<v.size();++i){
    const char ch=v[i];
    if(ch>='0'&&ch<='9'){ digit=true; continue; }
    if(ch=='/'){ if(slash||!digit||i+1>=v.size()) return false; slash=true; digit=false; continue; }
    return false;
  }
  return digit;
}
static std::vector<std::string> plainSplit(const std::string& s){
  std::vector<std::string> out; std::stringstream ss(s); std::string x;
  while(std::getline(ss,x,',')){ const std::string v=trim(x); if(!v.empty()) out.push_back(v); }
  return out;
}
/* shape: <id> ::= "<head>" slots: a,b,c,d target: <fn>
 * 与 load: 模块路径共用同一解析器，避免两处隐式约定分叉。 */
static bool parseShapeLine(const std::string& b,Ctx::ShapeDef& d,std::string& why){
  const std::size_t dc=b.find("::=");
  if(dc==std::string::npos){ why="E-REWRITE-ARITY"; return false; }
  d.id=trim(b.substr(0,dc));
  if(!isIdent(d.id)){ why="E-REWRITE-ARITY"; return false; }
  const std::size_t q1=b.find('"',dc+3), q2=(q1==std::string::npos)?std::string::npos:b.find('"',q1+1);
  if(q1==std::string::npos||q2==std::string::npos||q2<=q1){ why="E-REWRITE-ARITY"; return false; }
  d.head=b.substr(q1+1,q2-q1-1);
  if(d.head.empty()){ why="E-REWRITE-ARITY"; return false; }
  const std::size_t sp=b.find("slots:",q2), tp=b.find("target:",q2);
  if(sp==std::string::npos||tp==std::string::npos||tp<sp){ why="E-REWRITE-ARITY"; return false; }
  d.slots=plainSplit(b.substr(sp+6,tp-sp-6));
  d.target=trim(b.substr(tp+7));
  if(d.slots.empty()||!isIdent(d.target)){ why="E-REWRITE-ARITY"; return false; }
  std::set<std::string> seen;
  static const std::set<std::string> RESERVED={"i","e","pi","inf","oo","con","base","rule","sig","x"};
  for(const auto& p:d.slots){
    if(!isIdent(p)||RESERVED.count(p)||seen.count(p)){ why="E-REWRITE-ARITY"; return false; }
    seen.insert(p);
  }
  return true;
}
/* bind: <shape-id> <- 1,2,3,4  或  <- [[1,2],[3,4]] */
static std::vector<std::string> parseBindVals(const std::string& rhs){
  std::string v=trim(rhs);
  if(v.size()>=2&&v[0]=='['){
    std::string flat;
    for(char ch:v) if(ch!='['&&ch!=']') flat.push_back(ch);
    v=flat;
  }
  return plainSplit(v);
}

/* ══════════════ 0.0.7 公理模块：preset: 内置 / load: 外部 .ltir ══════════════ */
struct Preset { const char* name; const char* system; const char* choice; const char* axioms; const char* refs; };
static const Preset PRESETS[] = {
  {"ZFC","ZFC","on","Extensionality, Separation, Powerset, Union, Infinity, Replacement, Foundation, AC","Zermelo1904, Fraenkel1922, Godel1940, Cohen1963"},
  {"ZF","ZF","off","Extensionality, Separation, Powerset, Union, Infinity, Replacement, Foundation","Zermelo1904, Fraenkel1922"},
  {"NBG","NBG","on","Extensionality, ClassComprehension, Foundation, AC","vonNeumann1925, Bernays1937, Godel1940"},
  {"MK","MK","on","ClassComprehension, Foundation, AC","MorseKelley"},
  {"KP","KP","off","Extensionality, Separation, Delta0Collection, Foundation","KripkePlatek"},
  {"Naive","Naive","unset","Comprehension(unrestricted)","Russell1901, Frege"},
  {"ZFC_V=L","ZFC","on","ZFC, V=L","Godel1940"},
  {"ZFC_UltL","ZFC","on","ZFC, Ultimate-L","Woodin"},
  {"ZFC_Measurable","ZFC","on","ZFC, Measurable","LevySolovay1967"},
  {"I0","ZFC","on","ZFC, I0","Kunen1971"},
  {"ZF_Reinhardt","ZF","off","ZF, j:V->V (Reinhardt)","Kunen1971"},
  {"ZF_Berkeley","ZF","off","ZF, Berkeley","Kunen1971"},
  {"ZF_AD","ZF","off","ZF, AD","MycielskiSteinhaus1962, Kechris1984"},
  {"ZFC_CH","ZFC","on","ZFC, CH","Godel1940"},
  {"ZFC_notCH","ZFC","on","ZFC, notCH","Cohen1963"},
  {nullptr,nullptr,nullptr,nullptr,nullptr}
};
static void absorbAxioms(Ctx& c,const std::string& sys,const std::string& ch,
                         const std::string& ax,const std::string& rf){
  c.system=sys; c.choice=ch;
  absorbAxiomList(c,ax);
  std::stringstream sr(rf); std::string y;
  while(std::getline(sr,y,',')){ y=trim(y); if(!y.empty()) c.refsSeen.push_back(y); }
}
static bool hasAxiom(const Ctx& c,const std::string& needle){
  for(auto&a:c.axiomsSeen) if(a.find(needle)!=std::string::npos) return true;
  return false;
}
static void conflictCheck(Ctx& c){
  const bool ac=(c.choice=="on");
  const bool ad=hasAxiom(c,"AD"), rein=hasAxiom(c,"Reinhardt"), berk=hasAxiom(c,"Berkeley");
  if(ac&&ad) c.loadDiags.push_back("E-CHOICE-CONFLICT: AD 与 AC 不能同图");
  if(ac&&(rein||berk)) c.loadDiags.push_back("E-AXIOM-CONFLICT: Kunen 1971 不相容（ZFC + j:V->V）");
}
static void loadModuleFile(Ctx& c,const std::string& path,int depth){
  if(depth>8){ c.loadDiags.push_back("E-LOAD-CYCLE: "+path); return; }
  std::string p=path;
  if(c.loadedPaths.count(p)){ c.loadDiags.push_back("E-LOAD-CYCLE: "+p); return; }
  std::ifstream f(p);
  if(!f){ f.close(); std::ifstream g("axioms/"+p);
    if(!g){ c.loadDiags.push_back("E-LOAD-MISSING: "+p); return; }
    p="axioms/"+p; f=std::move(g); }
  c.loadedPaths.insert(p);
  std::string ln;
  while(std::getline(f,ln)){
    const std::string t=trim(ln);
    if(t.empty()||t[0]=='#') continue;
    const std::size_t col=t.find(':');
    if(col==std::string::npos) continue;
    const std::string k=normalizeKey(t.substr(0,col+1)), b=trim(t.substr(col+1));
    if(k=="module:"){ continue; }   // 外部模块首行：只认，不改顶层卷身份
    if(k=="system:"){ c.system=b; c.systemDeclared=true; continue; }
    if(k=="choice:"){ c.choice=b; continue; }
    if(k=="axiom:"){ absorbAxiomList(c,b); continue; }
    if(k=="lang:"){ if(!b.empty()){ if(!c.langSig.empty()) c.langSig+=" | "; c.langSig+=b; } continue; }
    if(k=="ref:"){ c.refsSeen.push_back(b); continue; }
    if(k=="load:"){ loadModuleFile(c,b,depth+1); continue; }
    if(k=="preset:"){ continue; }
    /* 0.2.1 重制 bug 5：模块路径不再静默丢 meta 行 —— 登记 + 计数 + 诊断。
     * 模块内不重新解析形状/不回投，只把登记事实带回顶卷账本。 */
    if(k=="gap:"||k=="reformulate:"||k=="quote:"||k=="reflect:"||k=="depth:"||k=="trace:"||
       k=="shape:"||k=="bind:"||k=="rewrite:"||k=="split:"||k=="mode:"||k=="by:"||k=="witness:"){
      if(k=="gap:"){ const std::string id=trim(b.substr(0,b.find_first_of(" \t")));
        if(!id.empty()&&!c.gapSeen.count(id)){ c.gaps.push_back(b); c.gapSeen[id]=b; } }
      else if(k=="reformulate:") c.reformulations.push_back(b);
      else if(k=="quote:") c.quotes.push_back(b);
      else if(k=="shape:"){ Ctx::ShapeDef d; std::string why;
        if(parseShapeLine(b,d,why)&&!c.shapes.count(d.id)) c.shapes[d.id]=d; }
      else if(k=="split:") c.splits.push_back(b);
      else if(k=="by:") c.hasBy=true;
      else if(k=="witness:") c.witnesses.push_back(b);
      else if(k=="mode:"&&(b=="trial"||b=="strict")) c.metaMode=b;
      c.loadDiags.push_back("E-LOAD-META: "+k+" registered (no re-eval)");
      c.registered++;
      continue;
    }
    if(k=="assert:"||k=="value:"||k=="numth:"||k=="ident:"||k=="solve:"||(t[0]=='A'&&t.find("->")!=std::string::npos)){
      c.loadDiags.push_back("E-LOAD-ASSERT: 模块内不应含断言行"); continue; }
    c.registered++;
  }
  conflictCheck(c);
}

/* 甲补丁：行尾 by: 与容器内 by: 共用一本引用账 */
static void registerByRef(Ctx& c,const std::string& raw){
  const std::string nm=trim(raw);
  if(nm.empty()) return;
  c.byRefs.push_back(nm); bool known=false;
  for(auto&a:c.axiomsSeen) if(a.find(nm)!=std::string::npos||nm.find(a)!=std::string::npos) known=true;
  for(auto&a:c.refsSeen)  if(a.find(nm)!=std::string::npos||nm.find(a)!=std::string::npos) known=true;
  if(nm=="calc"||nm=="Kunen1971"||nm=="Godel1940"||nm=="Cohen1963") known=true;
  for(auto&a:c.rules) if(a.find(nm)!=std::string::npos||nm.find(a)!=std::string::npos) known=true;
  if(!known) c.refUnknown.push_back(nm);
}
/* 甲补丁：有限集语义块抽出，assert: 与 value:/numth: 共用 */
template<typename CodeFn>
static bool trySetAssert(Ctx& c,const std::string& lhs,Result& R,
                         const CodeFn& codeRes,bool has,bool claim){
  Result SR;
  if(!evalSetAssert(c,lhs,SR,codeRes)) return false;
  R=SR;
  /* UNDECIDED / ERROR 不是布尔真值，绝不拿去和声明词比对（严禁并入 False） */
  if(has && (R.state==1||R.state==0)){
    const bool bv=(R.state==1), agree=(bv==claim);
    R.state=agree?1:0; R.out=agree?"True":"False";
    /* 甲补丁：声明词与实算一致时清掉集合侧残留码，避免 True 却挂 E-VAL-MISMATCH */
    if(agree){ R.code.clear(); R.detail.clear(); }
    if(!agree){ R.code="E-VAL-MISMATCH";
      R.detail=explain("E-VAL-MISMATCH",c.chinese,
        {{"lhs",bv?"True":"False"},{"rhs",claim?"True":"False"}}); }
  }
  return true;
}


/* ══════════ 0.0.7 补丁二：有限集域解析 / 量词统一判定 / 受限概括 ══════════
 * 设计口径：字面量 {...} 与具名有限集同权；连续域/构造式/嵌套一律诚实弃权；
 * 判 False 是合法求值结果，绝不挂 E-VAL-MISMATCH（那是断言声明词冲突专用码）。 */
static int resolveFiniteSet(Ctx& c, const std::string& raw, std::vector<Num>& out) {
  std::string s=trim(raw);
  { std::string l; bool cl; if(findVerdict(s,l,cl)) s=l; }   // 剥可能的声明词
  if(!s.empty() && s[0]=='{'){
    const std::size_t rb=s.rfind('}');
    if(rb==std::string::npos||rb==0) return 2;
    const std::string inner=s.substr(1,rb-1);
    if(inner.find('{')!=std::string::npos) return 1;          // 嵌套：不压平
    if(inner.find(':')!=std::string::npos||inner.find("...")!=std::string::npos||
       inner.find(" in ")!=std::string::npos||inner.find('|')!=std::string::npos||
       inner.find("forall")!=std::string::npos) return 1;     // 构造式 / 无限
    for(auto&p:splitTop(inner,',')){
      const std::string q=trim(p); if(q.empty()) continue;
      Num v=evalExpr(c,q); if(v.k==Num::UND) return 2;
      out.push_back(v); }
    return 0;
  }
  auto it=c.sets.find(s);
  if(it!=c.sets.end()){ out=it->second; return 0; }
  if(c.setDefs.count(s)) return 1;
  if(s=="emptyset"||s=="R"||s=="N"||s=="Z"||s=="Q"||s=="V"||
     s=="Omega"||s=="aleph0"||s=="cfrak"||s=="omega"||s=="(0, 1)") return 1;
  return 2;                                                  // 未登记：同样弃权
}

/* 受限概括 {x ∈ {1,2,3} | x > 1}：左边绑定域，右边当条件，逐元素精确算。
 * 返回 false = 不是合法受限概括（调用方按无受限域 → 罗素悖论处理）。 */
/* 0.1.0：三种写法都认
 *   {x in A | P(x)}            受限域在竖线左
 *   {x | x in A and P(x)}      受限域在竖线右（用户卷写法）
 *   {x | x in A}               无附加条件 → 取全域
 * 返回 0=精确算出 / 1=无受限域（罗素塌图）/ 2=有受限域但不可判定 */
static int restrictedComprehension(Ctx& c, const std::string& inner,
                                   std::vector<Num>& out) {
  const std::size_t bar=inner.find('|');
  if(bar==std::string::npos) return 1;
  const std::string left=trim(inner.substr(0,bar));
  const std::string cond0=trim(inner.substr(bar+1));
  std::string var, dom, cond=cond0;
  const std::size_t inL=left.find(" in ");
  if(inL!=std::string::npos){
    var=trim(left.substr(0,inL)); dom=trim(left.substr(inL+4));
  } else {
    /* 竖线右侧：x in A [and P(x)] */
    var=trim(left);
    const std::size_t inR=cond0.find(" in ");
    if(inR==std::string::npos) return 1;                 // 真·无受限域 → 罗素
    if(trim(cond0.substr(0,inR))!=var) return 1;
    std::size_t sep=cond0.find(" and ",inR+4);
    if(sep==std::string::npos) sep=cond0.find(" && ",inR+4);
    const std::size_t sepLen=(cond0.find(" && ",inR+4)!=std::string::npos &&
                              (sep==cond0.find(" && ",inR+4)))?4:5;
    if(sep==std::string::npos){ dom=trim(cond0.substr(inR+4)); cond=""; }
    else { dom=trim(cond0.substr(inR+4,sep-(inR+4))); cond=trim(cond0.substr(sep+sepLen)); }
  }
  if(!isIdent(var)||dom.empty()) return 1;
  std::vector<Num> elems;
  const int dr=resolveFiniteSet(c,dom,elems);
  if(dr==1) return 2;                                    // 无限 / 构造域：有码弃权
  if(dr!=0) return 1;
  for(auto&v:elems){
    Ctx d=c; d.env[var]=v;
    if(!cond.empty()){
      Num r=evalExpr(d,cond);
      if(r.k==Num::UND||r.k!=Num::BOOL) return 2;
      if(!r.bv) continue;
    }
    out.push_back(v);
  }
  return 0;
}

/* ∀ / ∃ 统一判定：forall: / exists: 关键字与 A: ∀ … / A: ∃ … 符号两路共用。
 * gate 已把 ∀→"forall "、∃→"exists "、∈→" in "，此处只见 ASCII。 */
template<typename CodeFn>
static Result quantEval(Ctx& c, const std::string& body, bool isExists,
                        const CodeFn& codeRes) {
  Result R;
  std::string lhs=body; bool claim=false;
  const bool has=findVerdict(body,lhs,claim);
  const std::size_t in=lhs.find(" in ");
  if(in==std::string::npos) return codeRes("E-SCOPE-QUANTIFIER",2);
  const std::string var=trim(lhs.substr(0,in));
  if(!isIdent(var)) return codeRes("E-SCOPE-QUANTIFIER",2);
  /* 域解析：字面集合按花括号配对，避开 {1,2,3} 内部逗号被误当分隔符；
   * 具名域按第一个 ',' / ':' 切。两种写法共用同一套逐元素判定。 */
  const std::string rest=trim(lhs.substr(in+4));
  std::string dom, cond;
  if(!rest.empty() && rest[0]=='{'){
    int d=0; std::size_t e=0;
    for(;e<rest.size();++e){
      if(rest[e]=='{') ++d;
      else if(rest[e]=='}'){ --d; if(d==0){ ++e; break; } }
    }
    if(d!=0) return codeRes("E-SCOPE-QUANTIFIER",2);
    dom=trim(rest.substr(0,e));
    std::string tail=trim(rest.substr(e));
    /* 防误伤：字面集合域后必须显式跟 ',' 或 ':'，不接受紧贴的裸表达式 */
    if(tail.empty()||(tail[0]!=','&&tail[0]!=':'))
      return codeRes("E-SCOPE-QUANTIFIER",2);
    cond=trim(tail.substr(1));
  } else {
    const std::size_t sep=rest.find_first_of(",:");
    if(sep==std::string::npos) return codeRes("E-SCOPE-QUANTIFIER",2);
    dom=trim(rest.substr(0,sep));
    cond=trim(rest.substr(sep+1));
  }
  if(cond.empty()) return codeRes("E-SCOPE-QUANTIFIER",2);
  std::vector<Num> elems;
  if(resolveFiniteSet(c,dom,elems)!=0) return codeRes("E-SCOPE-INFINITE",2);
  bool any=false, all=true;
  for(auto&v:elems){
    Ctx d=c; d.env[var]=v;
    Num r=evalExpr(d,cond);
    if(r.k==Num::UND||r.k!=Num::BOOL) return codeRes("E-SCOPE-QUANTIFIER",2);
    if(r.bv) any=true; else all=false;
  }
  const bool bv=isExists?any:all;
  if(has){
    const bool agree=(bv==claim);
    R.state=agree?1:0; R.out=agree?"True":"False";
    if(!agree){ R.code="E-VAL-MISMATCH";
      R.detail=explain("E-VAL-MISMATCH",c.chinese,
        {{"lhs",bv?"True":"False"},{"rhs",claim?"True":"False"}}); }
  } else { R.state=bv?1:0; R.out=bv?"True":"False"; }   // 判假无码
  return R;
}

/* 0.0.8：按词边界替换标识符（δ 代入用） */
static std::string substIdent(const std::string& s,const std::string& from,const std::string& to){
  if(from.empty()) return s;
  std::string out; std::size_t i=0;
  while(i<s.size()){
    if(isalpha((unsigned char)s[i])||s[i]=='_'){
      std::size_t j=i+1;
      while(j<s.size()&&(isalnum((unsigned char)s[j])||s[j]=='_'))++j;
      const std::string w=s.substr(i,j-i);
      out += (w==from)?to:w;
      i=j; continue;
    }
    out.push_back(s[i]); ++i;
  }
  return out;
}
/* 0.0.8：一步 δ 展开 —— 代入第一个已登记函数应用 */
static std::string deltaStep(const Ctx& c,const std::string& e,bool& changed){
  changed=false;
  for(const auto&kv:c.defBodies){
    const std::string& fn=kv.first;
    if(fn.empty()) continue;
    std::size_t q=e.find(fn+"(");
    while(q!=std::string::npos){
      const bool lb=(q==0||!(isalnum((unsigned char)e[q-1])||e[q-1]=='_'));
      if(lb){
        std::size_t rp=std::string::npos; int depth=1;
        for(std::size_t k=q+fn.size()+1;k<e.size();++k){
          if(e[k]=='(')++depth;
          else if(e[k]==')'){ --depth; if(depth==0){ rp=k; break; } }
        }
        if(rp!=std::string::npos){
          const std::string arg=trim(e.substr(q+fn.size()+1,rp-q-fn.size()-1));
          auto pit=c.defParams.find(fn);
          const std::string par=(pit==c.defParams.end())?"x":pit->second;
          const std::string inst="("+substIdent(kv.second,par,"("+arg+")")+")";
          changed=true;
          return e.substr(0,q)+inst+e.substr(rp+1);
        }
      }
      q=e.find(fn+"(",q+1);
    }
  }
  return e;
}

/* 0.1.0：纯数字阶乘超 20! 是整数溢出，不是超越域 —— 归还 E-OVF 家族 */
static std::string overflowHint(const std::string& body){
  const std::string t=trim(body);
  auto allDig=[](const std::string& x)->bool{
    if(x.empty()) return false;
    for(char ch:x) if(!isdigit((unsigned char)ch)) return false;
    return true;
  };
  /* 纯数字阶乘 > 20!：整数溢出 */
  if(t.size()>1 && t.back()=='!' && allDig(trim(t.substr(0,t.size()-1)))){
    const long long v=std::strtoll(trim(t.substr(0,t.size()-1)).c_str(),nullptr,10);
    if(v>20 || v<0) return "E-OVF";
  }
  /* 纯数字幂 a^b 在 power() 已判 UND → 同属整数溢出家族，不归超越域 */
  {
    const std::size_t q=t.find('^');
    if(q!=std::string::npos && allDig(trim(t.substr(0,q))) &&
       allDig(trim(t.substr(q+1)))) return "E-OVF";
  }
  /* 双根式精确值暂不参与四则/比较：代数数能力边界，不是超越域 */
  {
    const bool trig=(t.find("sin(")!=std::string::npos||t.find("cos(")!=std::string::npos||
                     t.find("tan(")!=std::string::npos);
    bool topOp=false; int d=0;
    for(char ch:t){
      if(ch=='(')++d;
      else if(ch==')')--d;
      else if(d==0 && std::string("+-*=<>!").find(ch)!=std::string::npos) topOp=true;
    }
    if(trig&&topOp) return "E-SCOPE-ALGEBRAIC";
  }
  return "";
}
/* 0.1.1 S6：质因数分解展示串——N: 行与 eval 共用同一口径 */
static std::string factorStr(std::int64_t m){
  std::string s; bool first=true;
  for(std::int64_t d=2;d*d<=m;++d){ int c=0; while(m%d==0){m/=d;++c;}
    if(c){ if(!first)s+=" \xC3\x97 "; first=false; s+=std::to_string(d);
           if(c>1)s+="^"+std::to_string(c); } }
  if(m>1){ if(!first)s+=" \xC3\x97 "; s+=std::to_string(m); }
  return s.empty()?"1":s;
}
static Result handle(Ctx& c,const std::string& key,const std::string& raw,int lineno) {
  Result R; R.line=raw; R.fileLine=lineno;   /* 0.1.1 D4：文件行号入 Result，JSON 输出 file_line */
  const bool zh=c.chinese;
  auto codeRes=[&](const std::string& code,int state,const std::map<std::string,std::string>& fill={}){
    R.code=code; R.state=state; R.detail=explain(code,zh,fill); return R; };
  const std::string body=trim(raw);
  /* 补丁二：罗素集（无受限域的竖线构造式）一旦登记，其上任何属于/势判断
   * 都按「这张地图自相不一致」弃权，严禁判 True/False。按词边界匹配，防短名误伤。 */
  auto hitParadox=[&](const std::string& s)->bool{
    for(const auto&p:c.paradoxSets){
      if(p.empty()) continue;
      std::size_t q=s.find(p);
      while(q!=std::string::npos){
        const bool lb=(q==0||!isalnum((unsigned char)s[q-1]));
        const bool rb=(q+p.size()>=s.size()||!isalnum((unsigned char)s[q+p.size()]));
        if(lb&&rb) return true;
        q=s.find(p,q+1);
      }
    }
    return false; };

  /* 甲：仅台账域不复用数学域求值器 —— 有码弃权，不是硬错、不是 False、不跨域串台 */
  {
    static const std::set<std::string> JUDGE = {
      "assert:","value:","numth:","claim:","forall:","card:","prob:","stat:",
      "solve:","limit:","vector:","sum:","seq:","derive:","ident:"
    };
    /* 0.1.1 bugfix：「台账域无判定核」只管域专有记号，纯有理算术与域无关。
     * 早先把整段 A:/V: 判失能（domain: topology 下 1+1==2 也弃权）是作用域外溢。
     * 剥离声明词后若表达式不含任何字母（纯数字四则/比较），照常精确判。*/
    auto pureArith=[&](const std::string& s)->bool{
      std::string e=s;
      const std::size_t a=e.rfind("->");
      if(a!=std::string::npos) e=e.substr(0,a);
      bool digit=false;
      for(unsigned char ch:e){ if(isalpha(ch)) return false; if(isdigit(ch)) digit=true; }
      return digit;
    };
    if(c.ledgerDomains.count(c.domain) && JUDGE.count(key) &&
       !((key=="assert:"||key=="value:") && pureArith(body)))
      return codeRes("E-SCOPE-LEDGER-ONLY",2);
  }

  /* ══════════════ 0.1.0 L0 台账层：一致性强度 / 势 / 真类 / 范畴论 / 试卷 ══════════════
   * 纪律：只登记关系与弃权理由，绝不输出 Con(T)=True；手写 -> True 只触发校验，
   * 不提供真值。没找到证书 ≠ 不存在解释 → 一律 UNDECIDED，绝不并入 False。 */
  if (key=="con:") {
    const std::size_t a=body.find("->");            // Gödel II：结构护栏，不是外挂 if
    if(a!=std::string::npos){
      const std::string l=trim(body.substr(0,a)), r=trim(body.substr(a+2));
      /* 授权收尾：Gödel II 独立立码，不再蹭通用「无解释证书」 */
      if(!l.empty() && l==r) return codeRes("E-GODEL-II",2);
    }
    c.registered++; c.ledger.push_back("con: "+body); R.state=-1; return R;
  }
  {
    static const std::set<std::string> REG = {
      "equiconsistent:","translation:","class:","proper_class:",
      "cat:","object:","arrow:","compose:","identity:","functor:","nat:","preserves:",
      "limit:","colimit:","product:","equalizer:","pullback:","adj:","unit:","counit:",
      "yoneda:","topos:","subobject:","monad:","algebra:","carrier:","structure:",
      "hint:","difficulty:","attribution:","exam:","submit:",
      "ordinal:","benchmark:","metatheory:","notation:","con_base:"
    };
    if(REG.count(key) &&
       /* 0.1.1 bugfix B1：分析极限 limit: var->target of expr 不被范畴论登记拦截 */
       !(key=="limit:" && body.find(" of ")!=std::string::npos && body.find("->")!=std::string::npos)){
      c.registered++; c.ledger.push_back(key+" "+body);
      /* 展示型值标签：只进 ledger，不进 computeTheorySig（值不反过来定义理论身份） */
      if(key=="ordinal:"||key=="benchmark:"||key=="metatheory:"||key=="notation:"||key=="con_base:")
        c.strengthLabels.push_back(key+" "+trim(body));
      if(key=="class:"||key=="proper_class:"){
        std::string nm=body;
        const std::size_t sp=nm.find_first_of(" \t=");
        if(sp!=std::string::npos) nm=nm.substr(0,sp);
        if(!nm.empty()) c.properClasses.insert(nm);
      }
      R.state=-1; return R;
    }
  }
  if (key=="radix:"||key=="base:") {
    const int b=std::atoi(body.c_str());
    if(b<2||b>16) return codeRes("E-LITERAL-BASE",3);
    c.base=b; c.registered++; R.state=-1; return R;
  }
  if (key=="strength_le:"||key=="interprets:") {
    /* 授权收尾：强度比较专用码。手写 -> True 不提供真值，UNDECIDED 不并 False。 */
    return codeRes("E-STRENGTH-NOCERT",2);
  }
  if (key=="card_le:"||key=="card_eq:") {
    for(const auto& p:c.properClasses){
      if(!p.empty() && body.find(p)!=std::string::npos)
        return codeRes("E-SCOPE-PROPER-CLASS",2);  // 真类谈势：直接拦下
    }
    return codeRes("E-SCOPE-CARDINAL",2);          // 无限集 / 无双射证书
  }
  if (key=="commute:") {
    if(body.find("!=")!=std::string::npos||body.find("/=")!=std::string::npos||
       body.find(" no")!=std::string::npos)
      return codeRes("E-CAT-NONCOMMUTE",0);        // 有限交换图：能判就别弃权
    return codeRes("E-SCOPE-UNIVERSAL",2);
  }

  if (key=="note:"||key=="subject:") { R.state=-1; return R; }
  /* 0.1.1：顶层证书行只登记（keys 卷裸 verify 不再 ERROR，也不冒充三态） */
  /* 强度草案试验：指纹口径开关，只登记、不产三态 */
  if (key=="sig_mode:") {
    const std::string v=trim(body);
    if(v!="formula"&&v!="named") return codeRes("E-SYN",3);   /* 0.1.1 ST：非法开关值硬错误，不静默忽略 */
    c.sigMode=v;
    c.registered++; c.ledger.push_back("sig_mode: "+c.sigMode); R.state=-1; return R;
  }
  if (key=="by:") { c.hasBy=true; c.registered++; R.state=-1; return R; }

  /* ══════════ 0.2.0 元编程：quotation 可做，eval 永久不做 ══════════
   * 六词全部只登记：0 三态、不进指纹、不搬真值、不执行自产 .ltir。
   * 0.2.0 B：参数越界立 E-META-RANGE（TA 令），缺锚弃权 E-META-ANCHOR；撞名仍 E-DEF-SIG。 */
  /* ══════════ 0.2.1 重制：命名权显式、兑换权不给 ══════════
   * 五词全只登记；仅 rewrite: 在 mode: trial 下产出「旧词行」回投原判定核。
   * 缺省 mode: strict —— 旧卷零回归，试验态必须写进卷面，不留「跑的是哪套」疑案。 */
  if (key=="mode:") {
    const std::string v=trim(body);
    if(v!="trial"&&v!="strict") return codeRes("E-META-RANGE",3);
    c.metaMode=v; c.registered++; c.ledger.push_back("mode: "+v); R.state=-1; return R;
  }
  if (key=="shape:") {
    const std::string b=trim(body);
    if(metaStrictIllegal(b)){ c.illegalHits++; c.rewriteRejected++; return codeRes("E-REWRITE-REDEEM",3); }
    Ctx::ShapeDef d; std::string why;
    if(!parseShapeLine(b,d,why)){ c.rewriteRejected++; return codeRes(why,2); }
    if(c.shapes.count(d.id)){ c.rewriteRejected++; return codeRes("E-REWRITE-CONFLICT",2); }
    c.shapes[d.id]=d; c.registered++; c.ledger.push_back("shape: "+d.id);
    R.state=-1; return R;
  }
  if (key=="bind:") {
    const std::string b=trim(body);
    if(metaStrictIllegal(b)){ c.illegalHits++; c.rewriteRejected++; return codeRes("E-REWRITE-REDEEM",3); }
    const std::size_t ar=b.find("<-");
    if(ar==std::string::npos){ c.rewriteRejected++; return codeRes("E-REWRITE-ARITY",2); }
    const std::string id=trim(b.substr(0,ar));
    auto si=c.shapes.find(id);
    if(si==c.shapes.end()){ c.rewriteRejected++; return codeRes("E-SHAPE-UNKNOWN",2); }
    if(c.bindSeen.count(id)){ c.rewriteRejected++; return codeRes("E-REWRITE-CONFLICT",2); }
    const std::vector<std::string> vals=parseBindVals(b.substr(ar+2));
    if(vals.size()!=si->second.slots.size()){ c.rewriteRejected++; return codeRes("E-REWRITE-ARITY",2); }
    for(const auto& v:vals) if(!slotLiteralOk(v)){ c.rewriteRejected++; return codeRes("E-REWRITE-ARITY",2); }
    c.binds[id]=vals; c.bindSeen.insert(id);
    c.registered++; c.ledger.push_back("bind: "+id); R.state=-1; return R;
  }
  if (key=="split:") {
    const std::string b=trim(body);
    if(metaRedeem(b)){ c.illegalHits++; c.rewriteRejected++;
      if(metaWordHit(b,"con:")) return codeRes("E-GODEL-II",2);
      return codeRes("E-REWRITE-REDEEM",3); }
    if(b.find("by:")==std::string::npos) return codeRes("E-META-ANCHOR",2);
    const std::string id=trim(b.substr(0,b.find_first_of(" \t")));
    if(id.empty()||!isIdent(id)) return codeRes("E-REWRITE-ARITY",2);
    c.splits.push_back(b); c.registered++; c.ledger.push_back("split: "+id);
    R.state=-1; return R;   /* 只拆账本，没有 close、没有 exact */
  }
  if (key=="rewrite:") {
    const std::string id=trim(body);
    if(metaStrictIllegal(id)){ c.illegalHits++; c.rewriteRejected++; return codeRes("E-REWRITE-REDEEM",3); }
    if(id.empty()||!isIdent(id)){ c.rewriteRejected++; return codeRes("E-SHAPE-UNKNOWN",2); }
    /* L1 门禁：strict 下只登记，不展开（旧卷零回归 + 试验态显式） */
    if(c.metaMode!="trial"){
      c.ledger.push_back("rewrite: "+id+" (strict: registered only)");
      c.registered++; R.state=-1; return R;
    }
    /* L1 锚定 / L6 有限步 */
    if(!c.hasBy||c.defSigs.empty()) return codeRes("E-META-ANCHOR",2);
    if(c.rewriteOk>=c.expandDepth) return codeRes("E-DEF-DEPTH",2);
    auto si=c.shapes.find(id);
    auto bi=c.binds.find(id);
    if(si==c.shapes.end()||bi==c.binds.end()){ c.rewriteRejected++; return codeRes("E-SHAPE-UNKNOWN",2); }
    auto ri=c.reformNames.find(si->second.target);
    if(ri==c.reformNames.end()){ c.rewriteRejected++; return codeRes("E-SHAPE-UNKNOWN",2); }
    /* 规则本体：fn(p1,..,pn) = <旧表达式>；参数名必须与 shape 槽位逐字一致 */
    const std::string rule=ri->second;
    const std::size_t lp=rule.find('('), rp=rule.find(')'), eq=rule.find('=');
    if(lp==std::string::npos||rp==std::string::npos||eq==std::string::npos||rp<lp||eq<rp){
      c.rewriteRejected++; return codeRes("E-REWRITE-ARITY",2); }
    const std::vector<std::string> ps=plainSplit(rule.substr(lp+1,rp-lp-1));
    const std::string expr=trim(rule.substr(eq+1));
    if(ps.size()!=si->second.slots.size()||expr.empty()||expr.find(':')!=std::string::npos){
      c.rewriteRejected++; return codeRes("E-REWRITE-ARITY",2); }
    for(std::size_t k=0;k<ps.size();++k)
      if(!isIdent(ps[k])||ps[k]!=si->second.slots[k]){
        c.rewriteRejected++; return codeRes("E-REWRITE-ARITY",2); }
    if(!metaProductAllowed(expr)){ c.illegalHits++; c.rewriteRejected++; return codeRes("E-REWRITE-REDEEM",3); }
    /* 产物只准是已立法词条：at: 绑定 + V: 表达式，交回原判定核算三态 */
    std::string atLine="at: ";
    for(std::size_t k=0;k<ps.size();++k)
      atLine+=(k?", ":"")+ps[k]+"="+bi->second[k];
    std::string vLine="V: "+expr;
    if(!metaProductAllowed(atLine)||!metaProductAllowed(vLine)){
      c.illegalHits++; c.rewriteRejected++; return codeRes("E-REWRITE-REDEEM",3); }
    c.replayQueue.push_back({atLine,R.fileLine});
    c.replayQueue.push_back({vLine,R.fileLine});
    c.rewriteTraceAll.push_back(atLine); c.rewriteTraceAll.push_back(vLine);
    c.rewriteOk++; c.registered++; c.ledger.push_back("rewrite: "+id+" -> 2 derived lines");
    if(c.traceOn){ std::cerr<<"[rewrite] "<<atLine<<"\n"; std::cerr<<"[rewrite] "<<vLine<<"\n"; }
    R.state=-1; return R;   /* 触发行自身 0 三态；真值由回投产物给 */
  }

  if (key=="depth:") {
    const std::string dv=trim(body); int d=0; bool ok=!dv.empty();
    for(char ch:dv){ if(ch<'0'||ch>'9'){ok=false;break;} d=d*10+(ch-'0'); if(d>1000){ok=false;break;} }
    if(!ok||d<1||d>64) return codeRes("E-META-RANGE",3);
    c.expandDepth=d; c.registered++; c.ledger.push_back("depth: "+dv); R.state=-1; return R;
  }
  if (key=="trace:") {
    const std::string v=trim(body);
    if(v!="on"&&v!="off") return codeRes("E-META-RANGE",3);
    c.traceOn=(v=="on"); c.registered++; R.state=-1; return R;
  }
  if (key=="gap:") {
    const std::string b=trim(body);
    const std::string id=trim(b.substr(0,b.find_first_of(" \t")));
    if(id.empty()) return codeRes("E-DEF-SIG",3);
    if(metaStrictIllegal(b)){ c.illegalHits++; c.rewriteRejected++; return codeRes("E-REWRITE-REDEEM",3); }
    if(c.gapSeen.count(id)) return codeRes("E-DEF-SIG",2);  /* 同 id 两条：弃权，不静默择一 */
    c.gapSeen[id]=b; c.gaps.push_back(b); c.gapTrace.push_back({R.fileLine,R.line}); c.registered++; c.ledger.push_back("gap: "+id); R.state=-1; return R;
  }
  if (key=="reformulate:") {
    const std::string b=trim(body);
    if(metaStrictIllegal(b)){ c.illegalHits++; c.rewriteRejected++; return codeRes("E-REWRITE-REDEEM",3); }
    const std::size_t lp=b.find('(');
    const std::string fn=trim(b.substr(0,lp==std::string::npos?b.find_first_of(" \t"):lp));
    if(fn.empty()||!isIdent(fn)){ c.rewriteRejected++; return codeRes("E-REWRITE-ARITY",2); }
    /* bug 1：同 fn 两条不再 last-wins，撞名即弃权 */
    if(c.reformNames.count(fn)){ c.rewriteRejected++; return codeRes("E-REWRITE-CONFLICT",2); }
    c.reformNames[fn]=b;
    c.reformulations.push_back(b); c.reformTrace.push_back({R.fileLine,R.line}); c.registered++; R.state=-1; return R;
  }
  if (key=="quote:") {
    const std::string b=trim(body);
    const std::size_t op=b.find("<ltir>"), cl=b.find("</ltir>");
    if(op==std::string::npos||cl==std::string::npos||cl<op) return codeRes("E-SYN",3);  /* 未闭合残词：有意报错，不静默吞行 */
    const std::string inner=b.substr(op+6,cl-op-6);
    if(metaRedeem(inner)){ c.illegalHits++; c.rewriteRejected++; return codeRes("E-REWRITE-REDEEM",3); }
    /* 重制：quote 只作展示数据，不参与求值；不存在「取第一个」的选择规则 */
    c.quotes.push_back(b); c.registered++; R.state=-1; return R;
  }
  if (key=="reflect:") {
    const std::string w=trim(body);
    if(w=="gaps"){ for(const auto& g:c.gaps) c.ledger.push_back("reflect/gap: "+g);
      std::string ids; for(const auto& g:c.gaps){ const std::size_t sp=g.find_first_of(" \t");
        ids+=(ids.empty()?"":", ")+g.substr(0,sp); }
      c.reflGaps=ids; }  /* 0.2.0 B：人话档列已登记 id，--tri/--json 不受污染 */
    else if(w=="rules") for(const auto& g:c.reformulations) c.ledger.push_back("reflect/rule: "+g);
    else if(w=="quotes") for(const auto& g:c.quotes) c.ledger.push_back("reflect/quote: "+g);
    else if(w=="rewrites"){
      c.reflExtra+="reflect: rewrites ->\n";
      for(const auto& r:c.rewriteTraceAll){
        c.ledger.push_back("reflect/rewrite: "+r);
        c.reflExtra+="  "+r+"\n";       /* L5：逐字列出回投产物 */
      }
      if(c.rewriteTraceAll.empty()) c.reflExtra+="  (none)\n";
    }
    else if(w=="shapes"){
      c.reflExtra+="reflect: shapes ->\n";
      for(const auto& p:c.shapes){
        c.ledger.push_back("reflect/shape: "+p.first);
        c.reflExtra+="  "+p.first+" ::= \""+p.second.head+"\" slots: ";
        for(std::size_t k=0;k<p.second.slots.size();++k)
          c.reflExtra+=(k?",":"")+p.second.slots[k];
        c.reflExtra+=" target: "+p.second.target+"\n";
      }
      if(c.shapes.empty()) c.reflExtra+="  (none)\n";
    }
    else if(w=="binds"){
      c.reflExtra+="reflect: binds ->\n";
      for(const auto& p:c.binds){
        c.ledger.push_back("reflect/bind: "+p.first);
        c.reflExtra+="  "+p.first+" <- ";
        for(std::size_t k=0;k<p.second.size();++k)
          c.reflExtra+=(k?",":"")+p.second[k];
        c.reflExtra+="\n";
      }
      if(c.binds.empty()) c.reflExtra+="  (none)\n";
    }
    else if(w=="splits"){
      c.reflExtra+="reflect: splits ->\n";
      for(const auto& r:c.splits){ c.ledger.push_back("reflect/split: "+r); c.reflExtra+="  "+r+"\n"; }
      if(c.splits.empty()) c.reflExtra+="  (none)\n";
    }
    else if(w=="mode"){
      c.ledger.push_back("reflect/mode: "+c.metaMode);
      c.reflExtra+="reflect: mode -> "+c.metaMode+"\n";
    }
    else if(w=="codes") c.ledger.push_back("reflect/codes: read-only");  /* 码表只读镜，不列内容 */
    else return codeRes("E-DEF-SIG",3);
    c.registered++; R.state=-1; return R;
  }

  if (key=="zf:"||key=="zfc:"||key=="given:"||key=="case:") {
    c.registered++; R.state=-1;                       // 只登记不判（保留现有语义）
    if(key=="given:"){ c.givens.push_back(body); }
    if(key=="case:"){ c.cases.push_back(body); }
    return R;
  }
  if (key=="check:") { c.checks.push_back(body); c.registered++; R.state=-1; return R; }

  if (key=="const:") {
    const std::size_t q=body.find('='); if(q==std::string::npos) return codeRes("E-EXT",2);
    const std::string name=trim(body.substr(0,q)), val=trim(body.substr(q+1));
    Num v=evalExpr(c,val);
    if(v.k==Num::UND) return codeRes("E-SCOPE-TRANSCENDENTAL",2);
    c.env[name]=v; c.registered++; R.state=-1; return R;
  }
  if (key=="at:") {
    c.env.clear();
    if(body.empty()){R.state=-1;return R;}
    for(auto&kv:splitTop(body,',')){ const std::size_t q=kv.find('='); if(q==std::string::npos)continue;
      const std::string n=trim(kv.substr(0,q)),e=trim(kv.substr(q+1));
      Num v=evalExpr(c,e); if(v.k!=Num::UND)c.env[n]=v; }
    R.state=-1; return R;
  }
  if (key=="define:") {
    /* 0.0.8：可选签名 define: f(x : Num) : Num = x*x（旧写法逐字保留） */
    std::string b2=body; std::string retType;
    const std::size_t lp0=b2.find('(');
    const std::size_t rp0=(lp0==std::string::npos)?std::string::npos:b2.find(')',lp0);
    if(lp0!=std::string::npos&&rp0!=std::string::npos){
      std::string inside=b2.substr(lp0+1,rp0-lp0-1);
      const std::size_t cc=inside.find(':');
      if(cc!=std::string::npos){ inside=trim(inside.substr(0,cc));
        b2=b2.substr(0,lp0+1)+inside+b2.substr(rp0); }
    }
    const std::size_t eq0=b2.find('=');
    if(eq0!=std::string::npos){
      std::string lhs2=b2.substr(0,eq0); const std::size_t cc=lhs2.rfind(':');
      if(cc!=std::string::npos){ retType=trim(lhs2.substr(cc+1));
        b2=trim(lhs2.substr(0,cc))+"="+b2.substr(eq0+1); }
    }
    const std::size_t q=b2.find('='); if(q==std::string::npos) return codeRes("E-SYN",3);
    const std::string lhs=trim(b2.substr(0,q)), rhs=trim(b2.substr(q+1));
    const std::size_t lp=lhs.find('('); if(lp==std::string::npos) return codeRes("E-SYN",3);
    const std::string fn=trim(lhs.substr(0,lp)), var=trim(lhs.substr(lp+1,lhs.size()-lp-2));
    /* 终止性：递归自引用不收（有限步 δ 展开必须有界） */
    if(rhs.find(fn+"(")!=std::string::npos) return codeRes("E-DEF-NONTERM",2);
    /* 良构性：右侧自由标识符须在参数 / 白名单 / where: 内 */
    {
      static const std::set<std::string> ALLOW={"x","y","z","pi","i","e","oo",
        "sqrt","sin","cos","tan","abs","N","R","Z","Q",
        "aleph0","omega","emptyset","cfrak"};
      std::string w;
      for(std::size_t k=0;k<=rhs.size();++k){
        const char ch=(k<rhs.size())?rhs[k]:' ';
        if(isalpha((unsigned char)ch)||ch=='_'){ w.push_back(ch); continue; }
        if(!w.empty()){
          if(!ALLOW.count(w)&&w!=var){
            bool declared=false;
            for(auto&d:c.defWhere) if(d.find(w)!=std::string::npos) declared=true;
            if(!declared) return codeRes("E-DEF-UNBOUND",2);
          }
          w.clear();
        }
      }
    }
    /* 签名一致性：sig: 登记返回类型与定义签名不符 → 有码弃权 */
    if(!retType.empty()){
      auto it=c.defSigs.find(fn);
      if(it!=c.defSigs.end()&&it->second.find(retType)==std::string::npos)
        return codeRes("E-DEF-SIG",2);
      c.defSigs[fn]=retType;
    }
    PolyParser pp(rhs); Poly z=pp.run(); if(!z.ok) return codeRes("E-EXT",2);
    c.funcs[fn]=z; c.defBodies[fn]=rhs; c.defParams[fn]=var;
    c.registered++; R.state=-1; return R;
  }
  if (key=="derive:") {
    const std::size_t q=body.find('\''); const std::string fn=trim(q==std::string::npos?body:body.substr(0,q));
    auto it=c.funcs.find(fn); if(it==c.funcs.end()) return codeRes("E-NAME",3);
    const auto pit=c.defParams.find(fn);
    const std::string par=(pit==c.defParams.end())?"x":pit->second;
    Poly d=pDerive(it->second,par);
    if(!d.ok) return codeRes("E-EXT",2);
    R.state=1; R.out=polyStr(d); R.code=""; R.detail=""; return R;
  }
  if (key=="domain:") {
    const std::string arg=trim(body);
    if(arg=="?"||arg=="list"||arg.empty()) {           // 列出已注册领域
      R.state=1; R.out=std::to_string(c.domainNames.size())+" domains registered";
      R.tag="domain"; return R;
    }
    if(c.domainNames.count(arg)) {                    // 显式设定当前领域
      setDomain(c,arg); c.registered++; R.state=-1; return R;
    }
    if(c.funcs.count(arg)) {                         // 兼容首版：查函数定义域
      R.state=1; R.out="R"; R.code=""; R.detail=""; return R;
    }
    // 未知领域：按「域外记号标黄放行」精神，原样登记不报错
    setDomain(c,arg); c.registered++; R.state=-1; return R;
  }
  if (key=="ident:") {
    static const char* eqs[]={"==","=",nullptr};
    for(int k=0;eqs[k];++k){ const std::string o=eqs[k]; const std::size_t q=body.find(o);
      if(q==std::string::npos)continue;
      PolyParser a(body.substr(0,q)),b(body.substr(q+o.size()));
      Poly x=a.run(),y=b.run(); if(!x.ok||!y.ok) return codeRes("E-EXT",2);
      Poly d=pSub(x,y);
      if(d.t.empty()){R.state=1;R.out="True";return R;}
      R.state=0;R.out="False";R.code="E-VAL-MISMATCH";
      R.detail=explain("E-ID-DIFF",zh,{{"diff",polyStr(d)}}); return R; }
    return codeRes("E-SYN",3);
  }
  if (key=="solve:") {
    static const char* ops[]={"<=",">=","<",">","=","==",nullptr};
    for(int k=0;ops[k];++k){ const std::string o=ops[k]; const std::size_t q=body.find(o);
      if(q==std::string::npos)continue;
      PolyParser a(body.substr(0,q)),b(body.substr(q+o.size()));
      Poly x=a.run(),y=b.run(); if(!x.ok||!y.ok) return codeRes("E-EXT",2);
      Poly d=pSub(x,y);
      // 一元二次：a x² + b x + c
      Monom m2; m2["x"]=2; Monom m1; m1["x"]=1; Monom m0;
      auto A=d.t.count(m2)?d.t[m2]:rat(0,1).a, B=d.t.count(m1)?d.t[m1]:rat(0,1).a, C=d.t.count(m0)?d.t[m0]:rat(0,1).a;
      if(A.p==0||d.t.size()>3) return codeRes("E-SCOPE-SYMBOLIC",2);
      Rational disc; Rational b2,four_ac,tmp;
      if(!ratMul(B,B,b2)||!ratMul(A,C,tmp)||!ratMul(tmp,rat(4,1).a,four_ac)||!ratSub(b2,four_ac,disc)) return codeRes("E-SCOPE-SYMBOLIC",2);
      /* 0.1.2 B8：Δ<0 恒号 —— 根本不需要开方。
       * 旧实现先 sqrtR(disc)，开不出有理根就弃权，把能判的分支挡在门外。
       * A>0 恒正 / A<0 恒负，解集只由 A 的符号与不等号方向决定。 */
      if(disc.p<0){
        const bool eq2=(o=="="||o=="==");
        if(eq2){ R.state=1; R.out="x \xE2\x88\x88 \xE2\x88\x85"; return R; }
        const bool aPos=(A.p>0), wantPos=(o==">"||o==">=");
        const bool holds=(aPos==wantPos);
        R.state=1;
        R.out=holds?"x \xE2\x88\x88 R":"x \xE2\x88\x88 \xE2\x88\x85";
        return R;
      }
      Num sd=sqrtR(disc); if(sd.k!=Num::RAT) return codeRes("E-SCOPE-SYMBOLIC",2);
      Rational twoA; if(!ratMul(A,rat(2,1).a,twoA)) return codeRes("E-SCOPE-SYMBOLIC",2);
      Rational nb; if(!ratNeg(B,nb)) return codeRes("E-SCOPE-SYMBOLIC",2);
      Rational r1,r2,sum1,sum2;
      if(!ratAdd(nb,sd.a,sum1)||!ratSub(nb,sd.a,sum2)||!ratDiv(sum1,twoA,r1)||!ratDiv(sum2,twoA,r2)) return codeRes("E-SCOPE-SYMBOLIC",2);
      if(r1.p*r2.q>r2.p*r1.q) std::swap(r1,r2);
      const bool lt=(o=="<"||o=="<="), eq=(o=="="||o=="==");
      /* 0.1.1 bugfix B3：判别式=0 重根 —— 按 A(x-r)² 的符号分析给解集，
       * 绝不拿空开区间 (r,r) 冒充解集（x^2<0 曾伪判 True）。 */
      if(r1.p*r2.q==r2.p*r1.q){
        if(eq){ R.state=1; R.out="x = "+ratStr(r1); return R; }
        const bool aPos=(A.p>0), less=(o=="<"||o=="<="), strict=(o=="<"||o==">");
        if(strict && (less==aPos)){ R.state=1; R.out="x \xE2\x88\x88 \xE2\x88\x85"; return R; }   // 空集
        if(!strict && (less==aPos)){ R.state=1; R.out="x = "+ratStr(r1); return R; }              // 单点
        if(!strict && (less!=aPos)){ R.state=1; R.out="x \xE2\x88\x88 R"; return R; }             // 全体实数
        R.state=1; R.out="x \xE2\x88\x88 (-\xE2\x88\x9E, "+ratStr(r1)+") \xE2\x88\xAA ("+ratStr(r2)+", \xE2\x88\x9E)"; return R;
      }
      if(eq){ R.state=1; R.out="x = "+ratStr(r1)+" or x = "+ratStr(r2); return R; }
      const bool inside = (A.p>0)?lt:!lt;
      /* 0.1.1 bugfix B7：非严格不等号（<=/>=）解集含端点，区间必须闭合；
       * 严格不等号（</>）保持开区间。外侧形态 (-∞, r1] ∪ [r2, ∞)。 */
      const bool closed=(o=="<="||o==">=");
      const std::string lb=closed?"[":"(", rb=closed?"]":")";
      const std::string l=inside?"x \xE2\x88\x88 "+lb+ratStr(r1)+", "+ratStr(r2)+rb
                                :"x \xE2\x88\x88 (-\xE2\x88\x9E, "+ratStr(r1)+rb+" \xE2\x88\xAA "+lb+ratStr(r2)+", \xE2\x88\x9E)";
      R.state=1;R.out=l;return R; }
    return codeRes("E-SYN",3);
  }
  if (key=="sum:") {
    const std::size_t of=body.find(" of "); const std::size_t dd=body.find("..");
    if(of==std::string::npos||dd==std::string::npos) return codeRes("E-SYN",3);
    const std::string head=trim(body.substr(0,dd)), loend=trim(body.substr(dd+2,of-dd-2));
    const std::size_t q=head.find('='); if(q==std::string::npos) return codeRes("E-SYN",3);
    const std::string var=trim(head.substr(0,q));
    Num l=evalExpr(c,trim(head.substr(q+1))), h=evalExpr(c,loend);
    if(l.k!=Num::RAT||h.k!=Num::RAT||l.a.q!=1||h.a.q!=1) return codeRes("E-RANGE",2);
    const std::string ex=trim(body.substr(of+4));
    Num acc=rat(0,1);
    for(std::int64_t i=l.a.p;i<=h.a.p&&i-l.a.p<10000;++i){
      Ctx d=c; d.env[var]=rat(i);
      std::string e=ex;
      // a_n 展开为已登记数列
      for(auto&s:d.seqs){ const std::string pat=s.first+"_"+s.second.first;
        const std::size_t z=e.find(pat); if(z!=std::string::npos){
          std::string sub=s.second.second;
          const std::size_t w=sub.find(s.second.first);
          if(w!=std::string::npos) sub.replace(w,1,"("+std::to_string(i)+")");
          e.replace(z,pat.size(),"("+sub+")"); } }
      Num v=evalExpr(d,e); if(v.k==Num::UND) return codeRes("E-SCOPE-SYMBOLIC",2);
      acc=add(acc,v); if(acc.k==Num::UND) return codeRes("E-OVF",2);
    }
    R.state=1;R.out=numStr(acc);return R;
  }
  if (key=="seq:") {
    const std::size_t q=body.find('='); if(q==std::string::npos) return codeRes("E-SYN",3);
    std::string lhs=trim(body.substr(0,q)),rhs=trim(body.substr(q+1));
    const std::size_t u=lhs.find('_'); if(u==std::string::npos) return codeRes("E-SYN",3);
    c.seqs[trim(lhs.substr(0,u))]={trim(lhs.substr(u+1)),rhs};
    c.registered++;R.state=-1;return R;
  }
  if (key=="limit:") {
    /* 0.1.1 bugfix B1：重写极限处理器——
     * 旧版只取 n^1 系数（注释却说比最高次）、忽略趋向点、无除号时把表达式本身当极限值。
     * 新版：var->inf 比最高次；有限趋向点精确代入；0/0 型归 E-INDETERMINATE；
     * 含其他自由变量 / 单侧 ∞ 符号分析超纲 → 诚实弃权。 */
    const std::size_t of=body.find(" of "); if(of==std::string::npos) return codeRes("E-SYN",3);
    const std::string head=trim(body.substr(0,of)), ex=trim(body.substr(of+4));
    const std::size_t ar=head.find("->"); if(ar==std::string::npos) return codeRes("E-SYN",3);
    const std::string var=trim(head.substr(0,ar)), tgt=trim(head.substr(ar+2));
    if(!isIdent(var)) return codeRes("E-SYN",3);
    const bool toInf=(tgt=="inf"||tgt=="oo");
    Rational tval{0,1};
    if(!toInf){
      Num t=evalExpr(c,tgt);
      if(t.k!=Num::RAT) return codeRes("E-SCOPE-SYMBOLIC",2);
      tval=t.a;
    }
    const std::size_t slash=ex.find('/');
    Poly num,den;
    if(slash!=std::string::npos){
      PolyParser n1(ex.substr(0,slash)),d1(ex.substr(slash+1));
      num=n1.run(); den=d1.run();
    } else { PolyParser n1(ex); num=n1.run(); den=Poly::c(1); }
    if(!num.ok||!den.ok) return codeRes("E-SCOPE-SYMBOLIC",2);
    /* 关于 var 的次数与首项系数；含其他自由变量 → 弃权 */
    auto degLead=[](const Poly&p,const std::string&v,int&deg,Rational&lead)->bool{
      deg=0; lead=rat(0,1).a; bool any=false;
      for(auto&kv:p.t){
        int d=0;
        for(auto&vv:kv.first){
          if(vv.first==v) d=vv.second;
          else if(vv.second!=0) return false;
        }
        if(!any||d>deg){ deg=d; lead=kv.second; any=true; }
      }
      return true;
    };
    int dn=0,dd=0; Rational ln=rat(0,1).a,ld=rat(0,1).a;
    if(!degLead(num,var,dn,ln)||!degLead(den,var,dd,ld)) return codeRes("E-SCOPE-SYMBOLIC",2);
    if(toInf){
      if(dn<dd){ R.state=1; R.out="0"; return R; }
      if(dn==dd){
        if(ld.p==0) return codeRes("E-SCOPE-SYMBOLIC",2);
        Rational v; if(!ratDiv(ln,ld,v)) return codeRes("E-OVF",2);
        R.state=1; R.out=ratStr(v); return R;
      }
      const bool pos=((ln.p>0)==(ld.p>0));
      R.state=1; R.out=pos?"\xE2\x88\x9E":"-\xE2\x88\x9E"; return R;
    }
    std::map<std::string,Num> e; e[var]=fromR(tval);
    bool okn=false,okd=false;
    const Rational vn=pEvalAt(num,e,okn), vd=pEvalAt(den,e,okd);
    if(!okn||!okd) return codeRes("E-SCOPE-SYMBOLIC",2);
    if(vd.p==0){
      if(vn.p==0) return codeRes("E-INDETERMINATE",2);   // 0/0 不定式：因式分解超纲，诚实弃权
      return codeRes("E-SCOPE-SYMBOLIC",2);              // 分母 0 分子非 0：单侧符号分析超纲
    }
    Rational v; if(!ratDiv(vn,vd,v)) return codeRes("E-OVF",2);
    R.state=1; R.out=ratStr(v); return R;
  }
  if (key=="set:") {
    const std::size_t q=body.find('='), lb=body.find('{'), rb=body.rfind('}');
    if(q==std::string::npos||lb==std::string::npos||rb==std::string::npos||rb<lb)
      return codeRes("E-EXT",2);
    const std::string n=trim(body.substr(0,q));
    const std::string inner=body.substr(lb+1,rb-lb-1);
    /* 补丁二：竖线构造符 {x | P(x)} —— 受限概括 / 朴素概括分叉，必须先于嵌套检测 */
    if(inner.find('|')!=std::string::npos){
      std::vector<Num> picked;
      const int rc9=restrictedComprehension(c,inner,picked);
      if(rc9==2){ c.setDefs[n]=inner; c.registered++;
        return codeRes("E-SCOPE-INFINITE",2); }   // 有受限域但算不了：有码弃权，不冒充罗素
      if(rc9==0){
        c.sets[n]=picked; c.registered++;
        std::vector<std::string> ss;
        for(auto&v:picked) ss.push_back(numStr(v));
        std::sort(ss.begin(),ss.end());
        ss.erase(std::unique(ss.begin(),ss.end()),ss.end());
        R.state=1; R.out="{";
        for(std::size_t i=0;i<ss.size();++i){ if(i) R.out+=", "; R.out+=ss[i]; }
        R.out+="}"; return R;
      }
      c.paradoxSets.insert(n); c.setDefs[n]=inner; c.registered++;
      return codeRes("E-PARADOX-RUSSELL",2);       // 塌图登记，不判真假
    }
    /* 0.0.7：嵌套集合不压平、构造式不猜通项 —— 一律登记/弃权，不产假答案 */
    if(inner.find('{')!=std::string::npos){
      c.setDefs[n]=inner; c.registered++;
      return codeRes("E-SCOPE-NESTED",2);
    }
    if(inner.find(':')!=std::string::npos||inner.find("...")!=std::string::npos||
       inner.find(" in ")!=std::string::npos||inner.find("forall")!=std::string::npos){
      c.setDefs[n]=inner; c.registered++;
      c.loadDiags.push_back("E-SET-COMPREHENSION: "+n+" 已登记，不猜通项、不自动展开");
      R.state=-1; return R;
    }
    std::vector<Num> vals;
    for(auto&part:splitTop(inner,',')) vals.push_back(evalExpr(c,part));
    c.sets[n]=vals; c.registered++; R.state=1; std::string s="{";
    for(std::size_t i=0;i<vals.size();++i){if(i)s+=", ";s+=numStr(vals[i]);} s+="}";R.out=s;return R;
  }
  if (key=="vector:"||key=="stat:") {
    auto vals=parseList(c,body);
    if(vals.empty()) return codeRes("E-EXT",2);
    if(body.find("cross")!=std::string::npos||body.find("L(")!=std::string::npos||body.find("dot")!=std::string::npos){
      const std::size_t b1=body.find('['),e1=body.find(']',b1),b2=body.find('[',e1),e2=body.find(']',b2);
      if(b1==std::string::npos||e1==std::string::npos||b2==std::string::npos||e2==std::string::npos)return codeRes("E-EXT",2);
      auto A=parseList(c,body.substr(b1,e1-b1+1)),B=parseList(c,body.substr(b2,e2-b2+1));
      if(A.size()!=3||B.size()!=3)return codeRes("E-EXT",2);
      auto at=[&](std::vector<Num>&v,int i)->Num{return v[i];};
      auto m=[&](Num x,Num y){return mul(x,y);};
      if(body.find("dot")!=std::string::npos){ Num s=rat(0,1); for(int i=0;i<3;++i)s=add(s,m(at(A,i),at(B,i)));
        R.state=1;R.out=numStr(s);return R; }
      Num x=sub(m(at(A,1),at(B,2)),m(at(A,2),at(B,1)));
      Num y=sub(m(at(A,2),at(B,0)),m(at(A,0),at(B,2)));
      Num z=sub(m(at(A,0),at(B,1)),m(at(A,1),at(B,0)));
      R.state=1;R.out="("+numStr(x)+", "+numStr(y)+", "+numStr(z)+")";return R; }
    if(key=="stat:"){
      Num s=rat(0,1); for(auto&v:vals)s=add(s,v); Num mean=divv(s,rat((std::int64_t)vals.size()));
      if(body.find("var")!=std::string::npos){ Num acc=rat(0,1);
        for(auto&v:vals){Num d=sub(v,mean);acc=add(acc,mul(d,d));} Num va=divv(acc,rat((std::int64_t)vals.size()));
        R.state=1;R.out=numStr(va);return R; }
      R.state=1;R.out=numStr(mean);return R; }
    return codeRes("E-EXT",2);
  }
  if (key=="prob:") { Num v=evalExpr(c,body);
    if(v.k==Num::UND){return codeRes("E-SCOPE-SYMBOLIC",2);} R.state=1;R.out=numStr(v);return R; }


  /* ══════════ 0.0.8：形式化定义（登记 + 有限步 δ 展开 + 良构性） ══════════ */
  if (key=="sig:"||key=="type:"||key=="where:"||key=="rule:"||key=="grammar:") {
    const std::size_t q=body.find(':');
    const std::string nm=(q==std::string::npos)?"":trim(body.substr(0,q));
    if(key=="sig:"&&!nm.empty()){
      if(c.defSigs.count(nm)) return codeRes("E-DEF-SIG",2);  /* 0.2.0 B：撞名即弃权，不静默覆盖（META_KEYWORDS 总原则2） */
      c.defSigs[nm]=trim(body.substr(q+1));
    }
    else if(key=="type:"&&!nm.empty()) c.defTypes[nm]=trim(body.substr(q+1));
    else if(key=="where:") c.defWhere.push_back(body);
    else if(key=="rule:") c.rules.push_back(body);
    else c.grammars.push_back(body);
    c.registered++; R.state=-1; return R;
  }
  if (key=="expand:") {
    /* 有限步 δ 展开：最多 8 步，每步代入一个已登记函数应用；超界诚实弃权 */
    std::string e=trim(body);
    const std::size_t lp=e.find('(');
    if(lp!=std::string::npos && !c.defBodies.count(trim(e.substr(0,lp))))
      return codeRes("E-NAME",3);
    /* 0.2.0：步数由 depth: 给（缺省 8，旧卷零回归）；trace: on 时逐步走 stderr。 */
    int steps=0;
    for(;steps<c.expandDepth;++steps){
      bool changed=false;
      std::string nx=deltaStep(c,e,changed);
      if(!changed) break;
      if(nx.size()>4096) return codeRes("E-DEF-DEPTH",2);
      if(c.traceOn) std::cerr<<"[trace "<<(steps+1)<<"] "<<nx<<"\n";
      e=nx;
    }
    if(steps>=c.expandDepth) return codeRes("E-DEF-DEPTH",2);
    Num v=evalExpr(c,"("+e+")");
    if(v.k==Num::UND||v.k==Num::SYM) return codeRes("E-SCOPE-SYMBOLIC",2);
    R.state=1; R.out=numStr(v); R.code=""; return R;
  }
  /* ══════════ 0.0.8：集合论多宇宙观（图册只登记，不跨图搬真值） ══════════ */
  if (key=="multiverse:") {
    c.multiverseOn=(body=="on"||body=="true"||body=="yes");
    c.registered++; R.state=-1; return R;
  }
  if (key=="world:") {
    const std::size_t q=body.find('=');
    const std::string nm=trim(q==std::string::npos?body:body.substr(0,q));
    if(!nm.empty()) c.worlds.insert(nm);
    c.registered++; R.state=-1; return R;
  }
  if (key=="force:"||key=="ground:") {
    std::string s=body; const std::size_t byp=s.find(" by:");
    if(byp!=std::string::npos) s=trim(s.substr(0,byp));
    const std::size_t ar=s.find("->");
    if(ar==std::string::npos) return codeRes("E-SCOPE-NO-MAP",2);
    const std::string a=trim(s.substr(0,ar)), b=trim(s.substr(ar+2));
    if(a.empty()||b.empty()||!c.worlds.count(a)||!c.worlds.count(b))
      return codeRes("E-SCOPE-NO-MAP",2);
    c.worldEdges.push_back(key+" "+a+" -> "+b);
    if(byp!=std::string::npos) registerByRef(c,trim(body.substr(byp+4)));
    c.registered++; R.state=-1; return R;
  }
  if (key=="mantle:") { c.registered++; R.state=-1; return R; }
  if (key=="reachable:") { return codeRes("E-SCOPE-CROSS-WORLD",2); }

  /* ══════════ 0.0.7 集合论实验区（L0/L1：语法资产 + 诚实弃权） ══════════ */
  if (key=="system:") { c.system=body; c.systemDeclared=true; c.registered++; R.state=-1; return R; }
  if (key=="choice:") { c.choice=body; c.registered++; conflictCheck(c); R.state=-1; return R; }
  if (key=="axiom:") {
    absorbAxiomList(c,body);
    c.registered++; conflictCheck(c); R.state=-1; return R;
  }
  /* 0.1.0 A：理论语言签名（只登记，不产三态行） */
  if (key=="lang:") {
    const std::string v=trim(body);
    if(!v.empty()){ if(!c.langSig.empty()) c.langSig+=" | "; c.langSig+=v; }
    c.registered++; c.ledger.push_back("lang: "+v); R.state=-1; return R;
  }
  /* 0.1.0 A：按 lang + 公理公式 + rules 算内容指纹。
   * 空 / ? → 只算；给定期望 hex → 比对计入 sig_mismatch（只登记，不判、不并 False）。 */
  if (key=="theory_sig:") {
    const std::string want=trim(body);
    c.theorySigHex=computeTheorySig(c);
    c.theorySigAsked=true;
    if(!want.empty() && want!="?" && want!="auto"){
      std::string w=want;
      if(w.size()>2 && w[0]=='0' && (w[1]=='x'||w[1]=='X')) w=w.substr(2);
      for(char&ch:w) ch=(char)std::tolower((unsigned char)ch);
      if(w!=c.theorySigHex) c.sigMismatch++;
    }
    c.registered++; c.ledger.push_back("theory_sig: "+c.theorySigHex);
    R.state=-1; return R;
  }
  if (key=="ref:") { c.refsSeen.push_back(body); c.registered++; R.state=-1; return R; }
  if (key=="module:") { c.isModule=true; c.registered++; R.state=-1; return R; }
  if (key=="preset:") {
    const std::string nm=trim(body); bool found=false;
    for(int k=0;PRESETS[k].name;++k) if(nm==PRESETS[k].name){
      found=true;
      if(std::find(c.loadedPresets.begin(),c.loadedPresets.end(),nm)==c.loadedPresets.end()){
        c.loadedPresets.push_back(nm);
        absorbAxioms(c,PRESETS[k].system,PRESETS[k].choice,PRESETS[k].axioms,PRESETS[k].refs);
        c.systemDeclared=true; conflictCheck(c);
      }
      break; }
    if(!found) c.loadDiags.push_back("E-LOAD-MISSING: preset "+nm);
    c.registered++; R.state=-1; return R;
  }
  if (key=="load:") { loadModuleFile(c,trim(body),0); c.registered++; R.state=-1; return R; }
  if (key=="universe:") {
    if(body.find("V")==std::string::npos){ c.registered++; R.state=-1; return R; }
    if(!(c.domain=="set"||c.domain=="foundations"||c.domain=="zfc"||
         c.domain=="model-theory"||c.domain=="naive"||c.domain=="logic"))
      return codeRes("E-SCOPE-DOMAIN",2);
    if(!c.systemDeclared) return codeRes("E-SCOPE-NO-MAP",2);
    c.registered++; R.state=-1; return R;   // V 是真类，不是集合
  }
  if (key=="univ:") {
    const std::size_t q=body.find('='), lb=body.find('{'), rb=body.rfind('}');
    if(q!=std::string::npos&&lb!=std::string::npos&&rb!=std::string::npos&&rb>lb){
      const std::string nm=trim(body.substr(0,q)), inner=body.substr(lb+1,rb-lb-1);
      if(inner.find("...")!=std::string::npos||inner.find(':')!=std::string::npos||
         inner.find(" in ")!=std::string::npos){ c.setDefs[nm]=inner; c.registered++; R.state=-1; return R; }
      std::vector<Num> vals;
      for(auto&p:splitTop(inner,',')) vals.push_back(evalExpr(c,p));
      c.sets[nm]=vals;
    }
    c.registered++; R.state=-1; return R;
  }
  if (key=="card:") {
    std::string nm=trim(body); const std::size_t ar=nm.find("->");
    if(ar!=std::string::npos) nm=trim(nm.substr(0,ar));
    SetR sr=setAtom(c,"card("+nm+")");
    if(sr.infinite) return codeRes("E-SCOPE-INFINITE",2);
    if(sr.nested)  return codeRes("E-SCOPE-NESTED",2);
    if(!sr.ok||sr.scalar.empty()) return codeRes("E-REF-UNKNOWN",2);
    R.state=1; R.out=sr.scalar; return R;
  }
  if (key=="forall:") return quantEval(c,body,false,codeRes);
  if (key=="exists:") return quantEval(c,body,true,codeRes);
  /* 补丁二：反例登记册 / 有限映射性质 / 有限映射定义 */
  if (key=="mapping:") {
    const std::size_t q=body.find('=');
    if(q==std::string::npos) return codeRes("E-REF-UNKNOWN",2);
    const std::string nm=trim(body.substr(0,q));
    std::string rest=trim(body.substr(q+1));
    std::vector<std::string> cod;
    const std::size_t on=rest.find(" on ");
    if(on!=std::string::npos){
      const std::string cd=trim(rest.substr(on+4));
      const std::size_t cr=cd.rfind('}');
      if(!cd.empty()&&cd[0]=='{'&&cr!=std::string::npos&&cr>0)
        for(auto&p:splitTop(cd.substr(1,cr-1),',')){
          Num v=evalExpr(c,trim(p)); if(v.k!=Num::UND) cod.push_back(numStr(v)); }
      rest=trim(rest.substr(0,on));
    }
    if(rest.find('{')==std::string::npos)          // R -> R 之类：无限域弃权
      return codeRes("E-SCOPE-INFINITE",2);
    const std::size_t lb=rest.find('{'), rb=rest.rfind('}');
    if(rb==std::string::npos||rb<=lb) return codeRes("E-REF-UNKNOWN",2);
    std::vector<std::pair<std::string,std::string>> mp;
    for(auto&p:splitTop(rest.substr(lb+1,rb-lb-1),',')){
      const std::size_t ar=p.find("->");
      if(ar==std::string::npos) continue;
      Num a=evalExpr(c,trim(p.substr(0,ar))), b=evalExpr(c,trim(p.substr(ar+2)));
      if(a.k==Num::UND||b.k==Num::UND) continue;
      mp.push_back({numStr(a),numStr(b)}); }
    if(mp.empty()) return codeRes("E-REF-UNKNOWN",2);
    c.mappings[nm]=mp; c.codomains[nm]=cod; c.registered++; R.state=-1; return R;
  }
  if (key=="inj:"||key=="surj:"||key=="bij:") {
    /* 0.1.1 bugfix B5：有限映射的手写声明词与 A: 同权——洗涤后照算，冲突挂 E-VAL-MISMATCH。
     * E-SCOPE-INFINITE 只留给真正的无限域（R/N/Z/Q 或 R -> R 形态）。 */
    std::string ref=trim(body); bool claim=false;
    const bool has=findVerdict(body,ref,claim);
    if(ref=="R"||ref=="N"||ref=="Z"||ref=="Q"||(!has&&body.find("->")!=std::string::npos))
      return codeRes("E-SCOPE-INFINITE",2);
    auto it=c.mappings.find(ref);
    if(it==c.mappings.end()) return codeRes("E-REF-UNKNOWN",2);
    std::set<std::string> img; bool inj=true;
    for(auto&pr:it->second) if(!img.insert(pr.second).second) inj=false;
    bool surj=true;
    if(key!="inj:"){
      const auto& cod=c.codomains[it->first];
      if(cod.empty()) return codeRes("E-SCOPE-CODOMAIN",2);
      for(const auto&x:cod) if(!img.count(x)) surj=false;
    }
    const bool bv=(key=="inj:")?inj:((key=="surj:")?surj:(inj&&surj));
    if(!has){ R.state=bv?1:0; R.out=bv?"True":"False"; return R; }   // 判假无码
    const bool agree=(bv==claim);
    R.state=agree?1:0; R.out=agree?"True":"False";
    if(!agree){ R.code="E-VAL-MISMATCH";
      R.detail=explain("E-VAL-MISMATCH",c.chinese,{{"lhs",bv?"True":"False"},{"rhs",claim?"True":"False"}}); }
    return R;
  }
  if (key=="cex:") {
    if(c.lastExpr.empty()) return codeRes("E-REF-UNKNOWN",2);
    Ctx d=c; bool bound=false;
    for(auto&seg:splitTop(body,',')){
      const std::size_t q=seg.find('=');
      if(q==std::string::npos) continue;
      const std::string nm=trim(seg.substr(0,q)), rhs=trim(seg.substr(q+1));
      if(!isIdent(nm)) continue;
      Num v=evalExpr(c,rhs);
      if(v.k==Num::UND||v.k==Num::SYM||v.k==Num::INF) continue;
      d.env[nm]=v; bound=true; }
    if(!bound) return codeRes("E-REF-UNKNOWN",2);
    Num r=evalExpr(d,c.lastExpr);
    if(r.k==Num::UND) return codeRes("E-SCOPE-TRANSCENDENTAL",2);
    if(r.k!=Num::BOOL) return codeRes("E-REF-UNKNOWN",2);
    if(!r.bv){ R.state=1; R.out=c.chinese?"反例有效":"counter-example valid"; return R; }
    return codeRes("E-CEX-INVALID",0);
  }
  if (key=="under:"||key=="cert:"||key=="witness:"||key=="problem:"||key=="status:"||
      key=="theory:"||key=="inner:"||key=="map:"||key=="independent:"||
      key=="model:"||key=="cite:") {
    if(key=="under:") c.givens.push_back(body);
    if(key=="cert:") c.certificates.push_back(body);
    if(key=="witness:"){
      /* bug 3：witness 体也扫手写结论；但允许 V:/solve: 作「旧错→修后」对照 */
      if(metaRedeem(body)){ c.illegalHits++; c.rewriteRejected++; return codeRes("E-REWRITE-REDEEM",3); }
      c.witnesses.push_back(body);
    }
    if(key=="problem:"||key=="status:") c.problems.push_back(body);
    if(key=="theory:"||key=="inner:"||key=="map:"||key=="independent:"||
       key=="model:"||key=="cite:") c.theories.push_back(body);
    c.registered++; R.state=-1; return R;
  }
  if (key=="claim:") { c.registered++; return codeRes("E-SCOPE-QUANTIFIER",2); }
  if (key=="paradox:") {
    c.registered++;
    if(body.find("Russell")!=std::string::npos||body.find("罗素")!=std::string::npos)
      return codeRes("E-PARADOX-RUSSELL",2);
    return codeRes("E-SCOPE-INFINITE",2);
  }
  if (key=="absolute:") { c.registered++; return codeRes("E-SCOPE-ABSOLUTE",2); }
  if (key=="assert:") {
    /* 0.2.1 纯实践收编 / 补回全禁版：箭头尾巴完整性。
     * 写了 -> 但 findVerdict 认不出合法 True/False（-> Maybe / 乱写尾巴 /
     * -> UNDECIDED / 中文弃权词）→ E-SYN，不静默吞尾巴给实算真值。
     * 立法同步：tests/smoke009.sh 第 43 条期望由「报实算真值」改为「一律给码」；
     * tests/archive/smoke00{6,7,8}.sh 是历史归档，按口径不动、不跑。
     * 合法声明（含中文标点尾巴 / 行尾 by:）仍走原比对，零回归。 */
    {
      const std::size_t av=body.rfind("->");
      if(av!=std::string::npos){
        std::string vl; bool vc=false;
        if(!findVerdict(body,vl,vc)) return codeRes("E-SYN",3);
      }
    }
    std::string expr=body;
    /* 补丁二：量词符号 ∀ / ∃（gate 已归一为 "forall " / "exists "）转译给统一判定 */
    if(expr.rfind("forall ",0)==0||expr.rfind("exists ",0)==0){
      const bool ex=expr.rfind("exists ",0)==0;
      return quantEval(c,trim(expr.substr(7)),ex,codeRes);
    }
    if(hitParadox(expr)) return codeRes("E-PARADOX-RUSSELL",2);
    /* 0.0.7 前置：内联 <C_and_E> 剥前缀登记身份（独立容器形态不变） */
    {
      const std::size_t il=expr.find("<C_and_E>");
      if(il!=std::string::npos){
        const std::string rest=trim(expr.substr(il+9));
        std::map<std::string,std::string> inf;
        inf["tag"]="inline"; inf["effect"]=rest;
        c.inferences.push_back(inf);
        expr=rest;
      }
    }
    /* 0.0.7 前置：未登记符号 / 真类 / 语义蕴涵 / ⊢ 地图 / 行尾 by: */
    {
      /* gate 把 □◇ 归一为 box/diamond；body 前导空格已被 trim，故按词边界认 */
      auto hasWord=[](const std::string& s,const std::string& w)->bool{
        std::size_t q=s.find(w);
        while(q!=std::string::npos){
          const bool lb=(q==0||!isalnum((unsigned char)s[q-1]));
          const bool rb=(q+w.size()>=s.size()||!isalnum((unsigned char)s[q+w.size()]));
          if(lb&&rb) return true;
          q=s.find(w,q+1);
        }
        return false;
      };
      if(hasWord(expr,"box")||hasWord(expr,"diamond"))
        return codeRes("E-SCOPE-MODAL",2);
    }
    if(expr.find("[UNREGSYM]")!=std::string::npos) return codeRes("E-REF-UNKNOWN",2);
    if(expr.find("card(V)")!=std::string::npos||expr.find("|V|")!=std::string::npos||
       expr.find("V in V")!=std::string::npos) return codeRes("E-SCOPE-PROPER-CLASS",2);
    if(expr.find(" models ")!=std::string::npos) return codeRes("E-SCOPE-SEMANTIC",2);
    const std::size_t vd=expr.find(" vdash ");
    if(vd!=std::string::npos){
      const std::string mp=trim(expr.substr(0,vd));
      expr=trim(expr.substr(vd+7)); c.maps.push_back(mp);
      if(!c.systemDeclared) return codeRes("E-SCOPE-NO-MAP",2);
      /* 0.0.8 口径正名：千禧年七问 / 哥德巴赫是 open，不是已证独立 */
      static const char* IND[]={"CH","Continuum","Incomplete",nullptr};
      for(int k=0;IND[k];++k) if(expr.find(IND[k])!=std::string::npos)
        return codeRes("E-SCOPE-INDEPENDENT",2);
      static const char* OPEN[]={"Riemann","RH","PvsNP","Hodge","Navier",
        "BSD","Yang","Goldbach","哥德巴赫",nullptr};
      for(int k=0;OPEN[k];++k) if(expr.find(OPEN[k])!=std::string::npos)
        return codeRes("E-SCOPE-OPEN",2);
      if(expr.find("Godel")!=std::string::npos||expr.find("哥德尔")!=std::string::npos)
        return codeRes("E-SCOPE-INCOMPLETE",2);
    }
    /* 行尾 by: 不再被静默吞：登记凭据并查已加载公理表，不替引擎判真假 */
    {
      const std::size_t p1=expr.rfind(" by:"), p2=expr.rfind(" by ");
      const std::size_t p=(p1!=std::string::npos)?p1:p2;
      if(p!=std::string::npos){
        const std::string nm=trim(expr.substr(p+4));
        if(!nm.empty()) registerByRef(c,nm);
        expr=trim(expr.substr(0,p));
      }
    }
    std::string lhs=expr; bool claim=false; const bool has=findVerdict(expr,lhs,claim);
    c.lastExpr=lhs;                              // 补丁二：cex: 反例登记册的代入目标
    /* 有限集语义优先（真值自产：声明词仍只比对，不作结论源） */
    if(trySetAssert(c,lhs,R,codeRes,has,claim)) return R;

    Num v=evalExpr(c,lhs);
    /* 0.1.1：0/0 及其复合式带不定式原因标记 → 独立专码，不蹭超越域 */
    if(v.indet) return codeRes("E-INDETERMINATE",2);
    if(v.k==Num::UND){
      const std::string ov=overflowHint(lhs);
      if(!ov.empty()) return codeRes(ov,2);
      return codeRes("E-SCOPE-TRANSCENDENTAL",2);
    }
    if(v.k==Num::INF) return codeRes("E-INF",2);
    if(v.k!=Num::BOOL){ codeRes("E-VAL-MISMATCH",0);
      R.detail = zh ? ("左侧实算为 "+numStr(v)+"（非布尔命题），无法与声明词比对")
                    : ("LHS evaluated to "+numStr(v)+" (non-boolean); cannot compare against a verdict token");
      return R; }
    if(!has){ R.state=v.bv?1:0; R.out=v.bv?"True":"False";
      if(!v.bv){R.code="E-VAL-MISMATCH";R.detail=explain("E-VAL-MISMATCH",zh,{{"lhs","False"},{"rhs","True"}});} return R; }
    const bool agree=(v.bv==claim);
    R.state=agree?1:0; R.out=agree?"True":"False";
    if(!agree){ R.code="E-VAL-MISMATCH";
      R.detail=explain("E-VAL-MISMATCH",zh,{{"lhs",v.bv?"True":"False"},{"rhs",claim?"True":"False"}}); }
    return R;
  }
  if (key=="value:"||key=="numth:") {
    /* 0.2.1 纯实践收编 / 补回全禁版：箭头尾巴完整性。
     * 写了 -> 但 findVerdict 认不出合法 True/False（-> Maybe / 乱写尾巴 /
     * -> UNDECIDED / 中文弃权词）→ E-SYN，不静默吞尾巴给实算真值。
     * 立法同步：tests/smoke009.sh 第 43 条期望由「报实算真值」改为「一律给码」；
     * tests/archive/smoke00{6,7,8}.sh 是历史归档，按口径不动、不跑。
     * 合法声明（含中文标点尾巴 / 行尾 by:）仍走原比对，零回归。 */
    {
      const std::size_t av=body.rfind("->");
      if(av!=std::string::npos){
        std::string vl; bool vc=false;
        if(!findVerdict(body,vl,vc)) return codeRes("E-SYN",3);
      }
    }
    if(hitParadox(body)) return codeRes("E-PARADOX-RUSSELL",2);
    if (key=="numth:" && body.rfind("factors(",0)==0) {
      const std::size_t e=body.find(')');
      Num n=evalExpr(c,body.substr(8,e==std::string::npos?std::string::npos:e-8));
      if(n.k!=Num::RAT||n.a.q!=1||n.a.p<=0) return codeRes("E-EXT",2);
      R.state=1; R.out=factorStr(n.a.p); return R;
    }
    /* 甲补丁：V:/N: 短写同样走有限集语义，避免 ∈/⊆/∩ 误归超越域或输出伪 "1"。
     * 先剥声明词再判集合，否则 "-> True" 会被整串吞进集合表达式（真值自产不变）。 */
    {
      std::string lhsSet=body; bool claimSet=false;
      const bool hasSet=findVerdict(body,lhsSet,claimSet);
      Result SR;
      if(trySetAssert(c,lhsSet,SR,codeRes,hasSet,claimSet)){
        R=SR;
        /* 补丁二 Bug 1：V: 是求值标签，无声明词时 False 是合法结果，
         * 绝不挂 E-VAL-MISMATCH（该码只属于 A: 声明词冲突）；UNDECIDED 的码不动。 */
        if(!hasSet && (R.state==1||R.state==0)){ R.code.clear(); R.detail.clear(); }
        return R;
      }
    }
    /* 0.1.0 补丁：裸集合名求值——已登记有限集直接回显，不落超越域兜底。
     * 码表「集合与势：有限集精确」；keys 卷 restricted-comprehension 的 V: T
     * 依赖此路径。精确匹配集合名，变量名/未登记名不受影响、照走 evalExpr。 */
    {
      auto it=c.sets.find(body);
      if(it!=c.sets.end()){
        R.state=1; std::string s="{";
        for(std::size_t i=0;i<it->second.size();++i){ if(i)s+=", "; s+=numStr(it->second[i]); }
        s+="}"; R.out=s; return R;
      }
    }
    Num v=evalExpr(c,body);
    if(v.indet) return codeRes("E-INDETERMINATE",2);
    if(v.k==Num::UND) {
      const std::string ov=overflowHint(body);
      if(!ov.empty()) return codeRes(ov,2);           // 0.1.0：阶乘溢出归 E-OVF
      if(body.find("oo")!=std::string::npos||body.find("inf")!=std::string::npos)
        return codeRes("E-INF",2);                    // ∞−∞ / ∞×0 未定形
      return codeRes("E-SCOPE-TRANSCENDENTAL",2);
    }
    if(v.k==Num::INF) { R.state=1; R.out=numStr(v); return R; }   // 速览：1/0 → ∞
    R.state=(v.k==Num::BOOL)?(v.bv?1:0):1; R.out=numStr(v);
    return R;                                   // 补丁二 Bug 1：求假无码
  }
  return codeRes("E-UNK-LINE",3);
}

/* ══════════════ verify 主流程 ══════════════ */
static int verify(const std::string& path,bool explainMode,bool jsonMode,
                  const std::string& codesPath,const std::string& domainsPath,
                  const std::string& lang, int outMode=0) {
  /* 0.1.0：outMode 1=只输出三态词 / 2=不输出答案值（只验算） / 0=正常 */
  if(outMode==1){ jsonMode=false; explainMode=false; }
  std::ifstream f(path);
  if(!f){ std::cerr<<"ltir006: cannot open "<<path<<"\n"; return 1; }
  std::stringstream ss; ss<<f.rdbuf();
  g_sandFix=0;                                   // 每次判卷独立计数
  const std::string text=gate(ss.str());
  Ctx c;
  c.sandFix=g_sandFix;
  c.chinese = (lang=="zh") ? true : (lang=="en" ? false : hasCjk(text));
  c.codesPath=codesPath;
  loadCodes(codesPath);
  loadDomains(c,domainsPath);
  /* 防「沙」彩蛋：多打省略号已补全，傲娇半句是 easter egg（JSON 单列） */
  if(c.sandFix>0 && !jsonMode && outMode!=1)
    std::cout<<"sand: "<<(c.chinese?SAND_ZH:SAND_EN)<<"\n";
  std::vector<std::string> lines; std::stringstream ls(text); std::string ln;
  while(std::getline(ls,ln)) lines.push_back(ln);

  std::vector<Result> rs; int err=0;
  std::map<std::string,std::string> curInf;   // <C_and_E> 当前块的 cause/effect/by

  for(std::size_t i=0;i<lines.size();++i){
    const std::string raw=lines[i]; const std::string t=trim(raw);
    if(t.empty()) continue;
    if(t[0]=='#'){ c.preserved.push_back(raw); continue; }   // 注释机器跳过、原样保留

    /* ── 具名容器标签（四件套 + 原有 <a>/<s>/<subject>） ── */
    if(t[0]=='<'){
      const std::size_t gt=t.find('>');
      std::string head=(gt==std::string::npos)?t:t.substr(1,gt-1);
      const bool closing = !head.empty() && head[0]=='/';
      std::string name = closing ? head.substr(1) : head;
      const std::size_t sp=name.find_first_of(" \t");
      std::string attrs = (sp==std::string::npos)?"":name.substr(sp);
      if(sp!=std::string::npos) name=name.substr(0,sp);
      // 输入层从宽：<过程>/<proc> 归一为 <calc>
      if(name=="过程"||name=="proc") name="calc";

      /* 0.1.0 进制容器：<2_nary> / <3_nary> / <二进制> / <base=2>，压栈-出栈 */
      const int rb=radixBase(name);
      /* 0.1.1 bugfix：闭合标签允许短名 </base> / </nary> / </radix> / </进制>，
       * 不再要求与开标签逐字相同（SYNTAX.md 同步补写法）。*/
      const bool radixCloser = closing && !c.baseStack.empty() &&
        (rb>0 || name=="base" || name=="nary" || name=="radix" || name=="进制");
      if(rb>0 || radixCloser){
        c.preserved.push_back(raw);
        if(!closing){
          c.baseStack.push_back(c.base); c.base=rb;
          std::string rest=(gt==std::string::npos)?"":trim(t.substr(gt+1));
          const std::string closer="</"+name+">";
          if(!rest.empty()){
            const std::size_t q=rest.rfind(closer);
            if(q!=std::string::npos) rest=trim(rest.substr(0,q));
            if(!rest.empty()){
              lines.insert(lines.begin()+i+1, closer);
              lines.insert(lines.begin()+i+1, rest);
            }
          }
        } else if(!c.baseStack.empty()){
          c.base=c.baseStack.back(); c.baseStack.pop_back();
        }
        continue;
      }

      if(!closing && containerTags().count(name)){
        c.containers.push_back(name);
        c.envStack.push_back(c.env);                 // 容器内「设 x=5」局部化
        // domain="..." 真的进 Ctx，不再读完 '>' 就丢
        const std::size_t dpos=attrs.find("domain=");
        if(dpos!=std::string::npos){
          std::string d=attrs.substr(dpos+7);
          const char quote = d.empty()?'"':d[0];
          if(quote=='"'||quote=='\''){ d=d.substr(1);
            const std::size_t e=d.find(quote); if(e!=std::string::npos)d=d.substr(0,e); }
          else { const std::size_t e=d.find_first_of(" \t"); if(e!=std::string::npos)d=d.substr(0,e); }
          if(!d.empty()) setDomain(c,d);
        }
        // 同行内容作为虚拟行插回（保持首版行为），闭合标签随之插入
        std::string rest=(gt==std::string::npos)?"":trim(t.substr(gt+1));
        const std::string closer=closeTag(name);
        if(!rest.empty()){
          const std::size_t q=rest.rfind(closer);
          if(q!=std::string::npos) rest=trim(rest.substr(0,q));
          if(!rest.empty()){
            lines.insert(lines.begin()+i+1, closer);
            lines.insert(lines.begin()+i+1, rest);
          }
        }
        continue;
      }
      if(closing && containerTags().count(name)){
        if(!c.containers.empty() && c.containers.back()==name){
          if(name=="C_and_E" && !curInf.empty()){ c.inferences.push_back(curInf); curInf.clear(); }
          c.containers.pop_back();
          if(!c.envStack.empty()){ c.env=c.envStack.back(); c.envStack.pop_back(); }
        }
        continue;
      }
      c.preserved.push_back(raw);                  // 未知标签：标黄放行精神，不拦截不解释
      continue;
    }

    /* 装饰简写是「行」不是「段」：整行机器跳过、原样保留，中文切开后不再抽算式。
     * V: 在 0.0.6 里仍是 value: 求值短写，只额外登记装饰身份（回归零破坏）。 */
    const std::size_t colonLine=t.find(':');
    const std::string kLine=(colonLine==std::string::npos)?"":t.substr(0,colonLine+1);
    if(isDecoration(kLine)){ c.decorations.push_back(t); continue; }
    if(kLine=="V:"||kLine=="val:") c.decorations.push_back(t);

    /* ── 段化：容器内多段（状态机），顶层单段（回归逐字不变） ── */
    std::vector<std::string> segs;
    if(c.inContainer()) segs=splitSegments(t); else segs.push_back(t);
    if(c.inContainer() && hasCjk(t)) c.preserved.push_back(raw);  // 中文装饰词原样保留

    for(const std::string& body0 : segs){
      std::string body=trim(body0);
      if(body.empty()) continue;
      /* 默认 base=10 是全等 no-op —— 347 条老回归逐字不受影响 */
      /* 0.1.1 bugfix B2：radix:/base: 是指令不是数据——参数恒按十进制读。
       * 早先指令自身也过 radixNorm，b 进制下 "10" 恒等于 b，永远切不回十进制。 */
      const std::size_t kwColonB2=body.find(':');
      const std::string kwB2=(kwColonB2==std::string::npos)?"":body.substr(0,kwColonB2+1);
      const bool radixDirective=(kwB2=="radix:"||kwB2=="base:");
      if(c.base!=10 && !radixDirective){
        std::string nb, er;
        if(!radixNorm(body,c.base,nb,er)){
          Result R; R.state=3; R.code=er; R.line=raw; R.fileLine=(int)i+1;
          R.detail=explain(er,c.chinese,{}); rs.push_back(R); err++; continue;
        }
        body=nb;
      }

      // <C_and_E> 三字段（全拼小写字段，不是简写行；裸 C: 仍封死）
      if(c.curContainer()=="C_and_E"){
        static const char* F[]={"cause:","effect:","by:",nullptr};
        bool hit=false;
        for(int k=0;F[k];++k){ const std::string f=F[k];
          if(body.rfind(f,0)==0){ curInf[std::string(F[k]).substr(0,f.size()-1)]=trim(body.substr(f.size())); hit=true; break; } }
        if(hit){
          /* f 为块内局部名，此处用 body 前缀直接判，避开与文件流的名字遮蔽 */
          if(body.rfind("by:",0)==0) registerByRef(c,trim(body.substr(3)));
          if(curInf.count("by") && curInf.size()>=3){ c.inferences.push_back(curInf); curInf.clear(); }
          continue; }
      }

      // 装饰简写 S: E: P: Q: —— 机器跳过、原样保留、渲染加粗
      const std::size_t colon0=body.find(':');
      const std::string k0=(colon0==std::string::npos)?"":body.substr(0,colon0+1);
      if(isDecoration(k0)){ c.decorations.push_back(body); continue; }

      // 容器内「设/令/取 x=5」→ 环境绑定，不产三态行（L06 根因）
      if(c.inContainer() && k0.empty() && tryBindEnv(c,body)) continue;

      std::string key;
      const std::size_t colon=body.find(':');
      if(colon!=std::string::npos) key=normalizeKey(body.substr(0,colon+1));
      /* 0.1.0 顶层裸赋值：a=1+1 只绑定，== 仍归判定；重名给码，不静默覆盖 */
      if(key.empty() && !c.inContainer()){
        const std::size_t eq=body.find('=');
        if(eq!=std::string::npos && !(eq+1<body.size()&&body[eq+1]=='=') &&
           !(eq>0 && (body[eq-1]=='<'||body[eq-1]=='>'||body[eq-1]=='!'))){
          const std::string lhs=trim(body.substr(0,eq));
          if(isIdent(lhs) && lhs!="pi"&&lhs!="e"&&lhs!="i"&&lhs!="oo"&&lhs!="inf"){
            if(c.env.count(lhs)){
              Result R; R.state=3; R.code="E-BIND-REDEFINED"; R.line=raw; R.fileLine=(int)i+1;
              R.detail=explain("E-BIND-REDEFINED",c.chinese,{});
              rs.push_back(R); err++; continue;
            }
            Num v=evalExpr(c,trim(body.substr(eq+1)));
            if(v.k!=Num::UND&&v.k!=Num::SYM&&v.k!=Num::INF){ c.env[lhs]=v; continue; }
          }
        }
      }

      if(key.empty()||!isKeyword(key)) {
        if(c.inContainer()){
          if(body.find('=')!=std::string::npos||body.find("->")!=std::string::npos) key="assert:";
          else continue;
        }
        else { Result R; R.state=3; R.code="E-UNK-LINE"; R.line=raw; R.fileLine=(int)i+1;
               R.detail=explain("E-UNK-LINE",c.chinese,{}); rs.push_back(R); err++; continue; }
      }
      if(key=="note:") continue;
      std::size_t start = (colon==std::string::npos) ? 0 : (colon+1);
      if (key=="assert:" && colon==1) start=2;        // A: 短写
      Result R=handle(c,key,body.substr(start),(int)i+1);
      /* 0.2.1 重制 bug 2：回投产物直接 handle，不往 lines 插行。
       * file_line 恒指向触发行，JSON 带 derived_from；产物行号不再 +N 漂移。 */
      if(!c.replayQueue.empty()){
        const auto q=c.replayQueue; c.replayQueue.clear();
        for(const auto& it:q){
          const std::string& pl=it.first;
          const std::size_t pc=pl.find(':');
          if(pc==std::string::npos) continue;
          const std::string pk=normalizeKey(pl.substr(0,pc+1));
          std::size_t ps=pc+1; if(pk=="assert:"&&pc==1) ps=2;
          Result P=handle(c,pk,pl.substr(ps),it.second);
          P.line="<derived> "+pl; P.derivedFrom=it.second;
          if(P.state!=-1){ rs.push_back(P); if(P.state==3) err++; }
        }
      }
      if(R.state==-1) continue;
      R.line=raw;
      if(key=="assert:"){ R.tag="answer"; c.answers.push_back(body.substr(start)); } // A: → <answer> 归一
      else if(c.curContainer()=="calc"||c.curContainer()=="a") R.tag="calc";
      rs.push_back(R);
      if(R.state==3) err++;
    }
  }
  if(!c.baseStack.empty()){                 // 容器忘闭合：有码，不猜到文末
    Result R; R.state=3; R.code="E-SCOPE-UNCLOSED"; R.line="<radix unclosed>"; R.fileLine=(int)lines.size();
    R.detail=explain("E-SCOPE-UNCLOSED",c.chinese,{}); rs.push_back(R); err++;
    c.baseStack.clear(); c.base=10;
  }
  if(!curInf.empty()) c.inferences.push_back(curInf);

  /* 0.2.0 B：元编程锚护栏——gap 需 sig+by 锚，reformulate 需至少一条 gap 对应。
   * 缺锚即弃权入账（UNDECIDED 行），登记撤销，不静默通过。 */
  if(!c.gapTrace.empty()&&(c.defSigs.empty()||!c.hasBy)){
    for(const auto&g:c.gapTrace){ Result R; R.state=2; R.code="E-META-ANCHOR";
      R.line=g.second; R.fileLine=g.first; R.detail=explain("E-META-ANCHOR",c.chinese,{});
      rs.push_back(R); }
    c.gaps.clear(); c.gapSeen.clear(); c.registered-=(int)c.gapTrace.size();
  }
  if(!c.reformTrace.empty()&&c.gaps.empty()){
    for(const auto&g:c.reformTrace){ Result R; R.state=2; R.code="E-META-ANCHOR";
      R.line=g.second; R.fileLine=g.first; R.detail=explain("E-META-ANCHOR",c.chinese,{});
      rs.push_back(R); }
    c.reformulations.clear(); c.registered-=(int)c.reformTrace.size();
  }

  int nt=0,nf=0,nu=0;
  for(auto&R:rs){ if(R.state==1)++nt; else if(R.state==0)++nf; else if(R.state==2)++nu; }
  const int total=nt+nf+nu;
  if(jsonMode){
    std::cout<<"{\"ok\":"<<(err?"false":"true")<<",\"lines\":[";
    for(std::size_t i=0;i<rs.size();++i){auto&R=rs[i];
      if(i)std::cout<<",";
      const char* st=R.state==1?"True":R.state==0?"False":R.state==2?"UNDECIDED":"ERROR";
      std::cout<<"{\"n\":"<<(i+1)<<",\"file_line\":"<<R.fileLine<<",\"state\":\""<<st<<"\",\"code\":\""<<R.code
               <<"\",\"tag\":\""<<R.tag<<"\"";
      if(R.code=="E-INDETERMINATE")
        std::cout<<",\"indeterminate_form\":\"0/0\",\"rules\":[\"N\",\"R1\",\"R2\",\"R3\",\"R4\"]";
      if(R.derivedFrom>0) std::cout<<",\"derived_from\":"<<R.derivedFrom;
      std::cout<<"}"; }
    std::cout<<"],\"counts\":{\"True\":"<<nt<<",\"False\":"<<nf<<",\"UNDECIDED\":"<<nu
             <<"},\"registered\":"<<c.registered
             <<",\"domain\":\""<<c.domain<<"\",\"domains_registered\":"<<c.domainNames.size()
             <<",\"domains_seen\":"<<c.domainsSeen.size()
             <<",\"decorations\":"<<c.decorations.size()
             <<",\"preserved\":"<<c.preserved.size()
             <<",\"inferences\":"<<c.inferences.size()
             <<",\"answers\":"<<c.answers.size()
             <<",\"version\":\"" LTIR_VER "\""
             <<",\"base\":"<<c.base<<",\"ledger\":"<<c.ledger.size()
             <<",\"sand_fixed\":"<<c.sandFix
             <<",\"easter_egg\":\""<<(c.sandFix>0?(c.chinese?EGG_ZH:EGG_EN):"")<<"\""
             <<",\"system\":\""<<c.system<<"\""
             <<",\"choice\":\""<<c.choice<<"\""
             <<",\"map_declared\":"<<(c.systemDeclared?"true":"false")
             <<",\"axioms_seen\":"<<c.axiomsSeen.size()
             <<",\"loaded_presets\":"<<c.loadedPresets.size()
             <<",\"set_defs\":"<<c.setDefs.size()
             <<",\"by_refs\":"<<c.byRefs.size()
             <<",\"ref_unknown\":"<<c.refUnknown.size()
             <<",\"module\":"<<(c.isModule?"true":"false")
             <<",\"multiverse\":"<<(c.multiverseOn?"true":"false")
             <<",\"worlds\":"<<c.worlds.size()
             <<",\"world_edges\":"<<c.worldEdges.size()
             <<",\"def_sigs\":"<<c.defSigs.size()
             <<",\"rules\":"<<c.rules.size()
             <<",\"grammars\":"<<c.grammars.size()
             <<",\"meta_gaps\":"<<c.gaps.size()
             <<",\"meta_rules\":"<<c.reformulations.size()
             <<",\"meta_quotes\":"<<c.quotes.size()
             <<",\"meta_depth\":"<<c.expandDepth
             <<",\"meta_shapes\":"<<c.shapes.size()
             <<",\"meta_binds\":"<<c.binds.size()
             <<",\"meta_splits\":"<<c.splits.size()
             <<",\"meta_rewrites\":"<<c.rewriteOk
             <<",\"meta_illegal\":"<<c.illegalHits
             <<",\"meta_rejected\":"<<c.rewriteRejected
             <<",\"meta_mode\":\""<<c.metaMode<<"\""
             <<",\"axiom_bodies\":"<<c.axiomBodies.size()
             <<",\"lang\":\""<<jesc(c.langSig)<<"\""
             <<",\"theory_sig\":\""<<c.theorySigHex<<"\""
             <<",\"sig_mode\":\""<<c.sigMode<<"\""
             <<",\"strength_labels\":"<<c.strengthLabels.size()
             <<",\"sig_mismatch\":"<<c.sigMismatch<<"}\n";
    return err?1:0;
  }
  if(outMode==1){
    for(auto&R:rs)
      if(R.state!=3)   /* 0.1.1：ERROR 不冒充 UNDECIDED（三态纪律） */
        std::cout<<(R.state==1?"True":R.state==0?"False":"UNDECIDED")<<"\n";
    return err?1:0;
  }
  for(std::size_t i=0;i<rs.size();++i){
    auto&R=rs[i];
    const char* st=R.state==1?"True":R.state==0?"False":R.state==2?"UNDECIDED":"ERROR";
    std::cout<<"L"<<(i+1<10?"0":"")<<(i+1)<<" "<<st;
    if(!R.code.empty()) std::cout<<"  "<<R.code;
    if(outMode!=2 && !R.out.empty() && R.out!=st) std::cout<<"  "<<R.out;
    std::cout<<"\n";
    if(explainMode&&!R.detail.empty()){
      std::cout<<"  "<<trim(R.line)<<"\n";
      // ^ 指位：按显示宽度算到首个 '=' 或 '->'
      std::size_t pos=R.line.find('='); if(pos==std::string::npos)pos=R.line.find("->");
      if(pos!=std::string::npos){ const int w=dispWidth(R.line.substr(0,pos));
        std::cout<<"  "<<std::string(w>0?w:0,' ')<<"^\n"; }
      if(R.code=="E-INDETERMINATE"){
        std::istringstream iss(indetChain(c.chinese)); std::string _l;
        while(std::getline(iss,_l)) std::cout<<"  "<<_l<<"\n";
      } else std::cout<<"  "<<R.detail<<"\n";
    }
    if(!explainMode && R.code=="E-INDETERMINATE")
      std::cout<<"  "<<indetLine(c.chinese)<<"\n";
  }
  std::cout<<"── "<<total<<" lines: "<<nt<<" True, "<<nf<<" False, "<<nu<<" UNDECIDED ──\n";
  if(!c.reflGaps.empty()) std::cout<<"reflect: gaps -> "<<c.reflGaps<<"\n";  /* 0.2.0 B：只列已登记 id */
  if(c.metaMode=="trial"&&c.rewriteOk) std::cout<<"rewrite: "<<c.rewriteOk<<" replayed (mode: trial)\n";
  if(!c.reflExtra.empty()) std::cout<<c.reflExtra;   /* 0.2.1 r2 / bug B：L5 回投可复原的 stdout 出口 */
  if(c.registered) std::cout<<"registered: "<<c.registered<<" (zf/zfc/given/case/check/const/define/seq/set/radix/mapping/limit/axiom/gap/reformulate/quote/reflect/depth/trace 等, ledger only)\n";
  if(!c.domainsSeen.empty())
    std::cout<<"domain: "<<c.domain<<" ("<<c.domainNames.size()<<" registered)\n";
  if(!c.inferences.empty()||!c.answers.empty()||!c.decorations.empty())
    std::cout<<"tags: "<<c.answers.size()<<" answer, "<<c.inferences.size()
             <<" C_and_E, "<<c.decorations.size()<<" decoration, "
             <<c.preserved.size()<<" preserved\n";
  if(!c.loadDiags.empty()){ for(auto&d:c.loadDiags) std::cout<<"load: "<<d<<"\n"; }
  if(!c.byRefs.empty()||!c.refUnknown.empty())
    std::cout<<"by_refs: "<<c.byRefs.size()<<" checked, "<<c.refUnknown.size()<<" unknown\n";
  if(!c.loadedPresets.empty()){
    std::cout<<"presets: ";
    for(std::size_t i=0;i<c.loadedPresets.size();++i){
      std::cout<<(i?", ":"")<<c.loadedPresets[i];
    }
    std::cout<<"\n"; }
  if(c.systemDeclared)
    std::cout<<"map: system="<<c.system<<" choice="<<c.choice
             <<" axioms="<<c.axiomsSeen.size()<<"\n";
  if(!c.setDefs.empty()) std::cout<<"set_defs: "<<c.setDefs.size()<<"\n";
  if(c.theorySigAsked && outMode==0 && !jsonMode)
    std::cout<<"theory_sig: "<<c.theorySigHex
             <<"  (lang+axioms+rules, "<<c.axiomBodies.size()
             <<" axiom bodies, mismatch="<<c.sigMismatch<<")\n";
  if(!c.strengthLabels.empty())
    std::cout<<"strength_labels: "<<c.strengthLabels.size()<<" (display-only; no verdict, no scalar)\n";
  if(c.isModule && rs.empty() && !jsonMode)
    std::cout<<"module: 这是公理模块，请用 preset: / load: 加载（只登记，不判卷）\n";
  return err?1:0;
}

#ifdef _WIN32
/* ══════════════ Windows 10+ 分支（Linux / Termux 侧此整块不编译） ══════════════
 * 双击 .ltir 时 Windows 把工作目录给成「卷所在目录」，不是安装目录；
 * 所以 lab_codes.tsv / domains.tsv / ltir006.json 一律从 exe 所在目录取。
 * 语言层（verify / 三态 / 四件套 / 24 领域）与 Linux 版共用同一份源码，不复制。
 */
static std::string winExeDir() {
  char buf[MAX_PATH*4]={0};
  const DWORD n=GetModuleFileNameA(NULL,buf,(DWORD)sizeof(buf)-1);
  std::string p(buf,n?n:0);
  const std::size_t q=p.find_last_of("\\/");
  return q==std::string::npos?std::string("."):p.substr(0,q);
}
static std::string winPath(const std::string& leaf) { return winExeDir()+"\\"+leaf; }

/* 极简扁平 JSON（零第三方）：只认 "key": value 一行式，够配置用 */
static std::string jEsc(const std::string& s) {
  std::string o;
  for(char ch:s){ if(ch=='\\'||ch=='"') o.push_back('\\'); o.push_back(ch); }
  return o;
}
static std::string jUnesc(const std::string& s) {
  std::string o;
  for(std::size_t i=0;i<s.size();++i){
    if(s[i]=='\\'&&i+1<s.size()){ o.push_back(s[i+1]); ++i; } else o.push_back(s[i]);
  }
  return o;
}
static std::string jVal(const std::string& line) {
  const std::size_t q=line.find(':');
  if(q==std::string::npos) return "";
  std::string v=trim(line.substr(q+1));
  while(!v.empty()&&(v.back()==','||v.back()==' '||v.back()=='\r')) v.pop_back();
  if(v.size()>=2&&v.front()=='"'&&v.back()=='"') return jUnesc(v.substr(1,v.size()-2));
  return v;
}
struct WinCfg {
  std::string installDir;
  bool associate=true;
  std::string exitMode="key";      // "key" 按任意键退出（默认） | "auto" N 秒自动关
  int autoSeconds=5;
};
static WinCfg winLoadCfg() {
  WinCfg c; c.installDir=winExeDir();
  std::ifstream f(winPath("ltir006.json"));
  std::string ln;
  while(std::getline(f,ln)){
    if(ln.find("\"install_dir\"")!=std::string::npos) c.installDir=jVal(ln);
    else if(ln.find("\"associate_ltir\"")!=std::string::npos) c.associate=(jVal(ln)=="true");
    else if(ln.find("\"exit_mode\"")!=std::string::npos) c.exitMode=jVal(ln);
    else if(ln.find("\"auto_seconds\"")!=std::string::npos){ int n=std::atoi(jVal(ln).c_str()); if(n>=0&&n<=3600)c.autoSeconds=n; }
  }
  if(c.exitMode!="auto") c.exitMode="key";
  return c;
}
static bool winSaveCfg(const WinCfg& c) {
  std::ofstream f(winPath("ltir006.json"));
  if(!f) return false;
  f<<"{\n"
   <<"  \"install_dir\": \""<<jEsc(c.installDir)<<"\",\n"
   <<"  \"associate_ltir\": "<<(c.associate?"true":"false")<<",\n"
   <<"  \"exit_mode\": \""<<c.exitMode<<"\",\n"
   <<"  \"auto_seconds\": "<<c.autoSeconds<<",\n"
   <<"  \"version\": \"" LTIR_VER "\"\n"
   <<"}\n";
  return true;
}
/* advapi32 动态装载：交叉编译不依赖 mingw 导入库，Win10+ 系统自带 */
static FARPROC regFn(const char* name){
  static HMODULE h=NULL;
  if(!h) h=LoadLibraryA("advapi32.dll");
  return h?GetProcAddress(h,name):NULL;
}
typedef LONG (WINAPI *RCreate_t)(HKEY,LPCSTR,DWORD,LPSTR,DWORD,REGSAM,LPSECURITY_ATTRIBUTES,PHKEY,LPDWORD);
typedef LONG (WINAPI *RSet_t)(HKEY,LPCSTR,DWORD,DWORD,const BYTE*,DWORD);
typedef LONG (WINAPI *RClose_t)(HKEY);
typedef LONG (WINAPI *RDel_t)(HKEY,LPCSTR);
static LONG rCreate(HKEY a,LPCSTR b,DWORD c,LPSTR d,DWORD e,REGSAM f,LPSECURITY_ATTRIBUTES g,PHKEY h,LPDWORD i){
  RCreate_t p=(RCreate_t)regFn("RegCreateKeyExA"); return p?p(a,b,c,d,e,f,g,h,i):1L; }
static LONG rSet(HKEY a,LPCSTR b,DWORD c,DWORD d,const BYTE* e,DWORD f){
  RSet_t p=(RSet_t)regFn("RegSetValueExA"); return p?p(a,b,c,d,e,f):1L; }
static void rClose(HKEY a){ RClose_t p=(RClose_t)regFn("RegCloseKey"); if(p)p(a); }
static void rDel(HKEY a,LPCSTR b){ RDel_t p=(RDel_t)regFn("RegDeleteTreeA"); if(p)p(a,b); }

/* 文件关联写 HKCU（Win10+ 不需要管理员） */
static bool winAssoc(bool on) {
  const std::string exe=winPath("ltir006.exe");
  const std::string cmd="\""+exe+"\" open \"%1\"";
  HKEY k=NULL;
  if(on){
    if(rCreate(HKEY_CURRENT_USER,"Software\\Classes\\.ltir",0,NULL,0,KEY_WRITE,NULL,&k,NULL)!=ERROR_SUCCESS) return false;
    const char* prog="LTIR.Document";
    rSet(k,NULL,0,REG_SZ,(const BYTE*)prog,(DWORD)strlen(prog)+1); rClose(k); k=NULL;
    if(rCreate(HKEY_CURRENT_USER,"Software\\Classes\\LTIR.Document\\shell\\open\\command",0,NULL,0,KEY_WRITE,NULL,&k,NULL)!=ERROR_SUCCESS) return false;
    rSet(k,NULL,0,REG_SZ,(const BYTE*)cmd.c_str(),(DWORD)cmd.size()+1); rClose(k);
    HKEY k2=NULL;
    if(rCreate(HKEY_CURRENT_USER,"Software\\Classes\\LTIR.Document",0,NULL,0,KEY_WRITE,NULL,&k2,NULL)==ERROR_SUCCESS){
      const char* desc="LTIR " LTIR_VER " 判定卷";
      rSet(k2,NULL,0,REG_SZ,(const BYTE*)desc,(DWORD)strlen(desc)+1); rClose(k2);
    }
  } else {
    rDel(HKEY_CURRENT_USER,"Software\\Classes\\.ltir");
    rDel(HKEY_CURRENT_USER,"Software\\Classes\\LTIR.Document");
  }
  return true;
}
static void winWait(const WinCfg& c) {
  if(c.exitMode=="auto") {
    std::cout<<"\n"<<c.autoSeconds<<" 秒后自动关闭...\n";
    Sleep((DWORD)c.autoSeconds*1000);
  } else {
    std::cout<<"\n按任意键退出...\n";
    _getch();
  }
}
/* 卸载命令：转交同目录的 uninstall.exe（卸载逻辑只实现一份，在 installer stub 里） */
static int winUninstallCmd() {
  const std::string u=winPath("uninstall.exe");
  std::string cl="\""+u+"\" --silent";
  STARTUPINFOA si; ZeroMemory(&si,sizeof(si)); si.cb=sizeof(si);
  PROCESS_INFORMATION pi; ZeroMemory(&pi,sizeof(pi));
  if(CreateProcessA(NULL,&cl[0],NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)){
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    std::cout<<"ltir006: 已启动卸载程序 uninstall.exe\n"; return 0;
  }
  std::cerr<<"ltir006: 无法启动 uninstall.exe，请手动运行同目录下的卸载程序\n"; return 1;
}
static int winConfig(int argc,char** argv) {
  WinCfg c=winLoadCfg();
  bool show=false, changed=false, assocChanged=false;
  for(int i=2;i<argc;++i){
    std::string a=argv[i];
    if(a=="--show") show=true;
    if(a.rfind("--set",0)==0){
      std::string kv=(i+1<argc)?argv[i+1]:std::string();
      if(kv.empty()) continue;
      const std::size_t q=kv.find('=');
      if(q==std::string::npos) continue;
      const std::string k=kv.substr(0,q), v=kv.substr(q+1);
      if(k=="exit"||k=="exit_mode"){ c.exitMode=(v=="auto")?"auto":"key"; changed=true; }
      else if(k=="seconds"||k=="auto_seconds"){ int n=std::atoi(v.c_str()); if(n>=0&&n<=3600){ c.autoSeconds=n; changed=true; } }
      else if(k=="associate"||k=="associate_ltir"){ c.associate=(v=="true"||v=="1"||v=="yes"); changed=true; assocChanged=true; }
      ++i;
    }
  }
  if(changed){
    if(!winSaveCfg(c)){ std::cerr<<"ltir006: cannot write ltir006.json\n"; return 1; }
    if(assocChanged) winAssoc(c.associate);
    std::cout<<"ltir006: 配置已更新\n";
  }
  if(show||!changed){
    std::cout<<"install_dir   : "<<c.installDir<<"\n"
             <<"associate_ltir: "<<(c.associate?"true":"false")<<"\n"
             <<"exit_mode     : "<<c.exitMode<<"\n"
             <<"auto_seconds  : "<<c.autoSeconds<<"\n";
  }
  return 0;
}
#endif

static void usage() {
#ifdef _WIN32
  std::cout<<"ltir006 " LTIR_VER " — Windows 10+, C++17, core SHA gated\n";
#else
  std::cout<<"ltir006 " LTIR_VER " — Linux / Termux, C++17, core SHA gated\n";
#endif
  std::cout
           <<"  ltir006 eval \"expr\"              单表达式求值\n"
           <<"  ltir006 verify 卷.ltir [--explain|--json] [--tri] [--no-answer]\n"
           <<"  ltir006 check 卷.ltir             只验算，不输出答案值\n"
           <<"  ltir006 verify 卷.ltir --domains=domains.tsv\n"
           <<"  ltir006 --version | --help | uninstall\n"
           <<"  LTIR_OUTPUT=tri | no-answer     当前终端全局输出模式\n"
           <<"四件套标签 calc/C_and_E/answer + S:E:P:Q: 装饰已并回；领域表见 domains.tsv\n"
           <<"语法关键词见 SYNTAX.md；用法见 速览.md；错误解释由 lab_codes.tsv 驱动\n"
           <<LTIR_VER "：进制眼镜 / 赋值环境 / 一致性强度台账 / 试卷库\n";
#ifdef _WIN32
  std::cout<<"  ltir006 config --show | --set exit=key|auto | --set seconds=N | --set associate=true|false\n"
           <<"  ltir006 open 卷.ltir         双击 .ltir 时由文件关联调用\n"
           <<"  ltir006 uninstall           转交同目录 uninstall.exe\n";
#endif
}

int main(int argc,char** argv){
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
  if(argc<2){ usage(); return 0; }
  const std::string cmd=argv[1];
  /* 0.1.0：当前终端全局输出模式（CLI flag 优先） */
  int envMode=0;
  if(const char* e=std::getenv("LTIR_OUTPUT")){
    const std::string v(e);
    if(v=="tri"||v=="TRI") envMode=1;
    else if(v=="no-answer"||v=="check") envMode=2;
  }
  if(const char* e=std::getenv("LTIR_TRI")) if(std::string(e)=="1") envMode=1;
  if(cmd=="--version"||cmd=="-V"){ std::cout<<"ltir006 " LTIR_VER "\n"; return 0; }
  if(cmd=="--help"||cmd=="-h"){ usage(); return 0; }
  std::string codes="lab_codes.tsv", domains="domains.tsv";
#ifdef _WIN32
  codes=winPath("lab_codes.tsv");      // 双击 .ltir 时 cwd 是卷目录，资源必须按 exe 目录取
  domains=winPath("domains.tsv");
#endif
  for(int i=2;i<argc;++i){ std::string a=argv[i];
    if(a.rfind("--codes=",0)==0) codes=a.substr(8);
    if(a.rfind("--domains=",0)==0) domains=a.substr(10); }
#ifdef _WIN32
  if(cmd=="config")   return winConfig(argc,argv);
  if(cmd=="uninstall") return winUninstallCmd();
  if(cmd=="open"){
    if(argc<3){ std::cerr<<"ltir006: open needs a .ltir file\n"; return 1; }
    std::cout<<"LTIR " LTIR_VER " · "<<argv[2]<<"\n\n";
    const int rc=verify(argv[2],false,false,codes,domains,"auto",envMode);
    winWait(winLoadCfg());
    return rc;
  }
#endif
  if(cmd=="eval"){
    if(argc<3){ std::cerr<<"ltir006: eval needs an expression\n"; return 1; }
    g_sandFix=0; loadCodes(codes); Ctx c; loadDomains(c,domains); c.chinese=hasCjk(argv[2]);
    /* 0.1.1 S6：eval factors(...) 与 N: 同口径，输出分解串而非弃权 */
    const std::string exT=trim(gate(argv[2]));
    if(exT.rfind("factors(",0)==0 && !exT.empty() && exT.back()==')'){
      Num n=evalExpr(c,exT.substr(8,exT.size()-9));
      if(n.k==Num::RAT&&n.a.q==1&&n.a.p>0){ std::cout<<factorStr(n.a.p)<<"\n"; return 0; }
      std::cout<<"UNDECIDED\n"; return 0;
    }
    Num v=evalExpr(c,gate(argv[2]));
    std::cout<<numStr(v)<<"\n";
    return 0;
  }
  if(cmd=="verify"||cmd=="check"){
    if(argc<3){ std::cerr<<"ltir006: "<<cmd<<" needs a file\n"; return 1; }
    bool ex=false,js=false; std::string lang="auto";
    int outMode=(cmd=="check")?2:envMode;
    for(int i=3;i<argc;++i){ std::string a=argv[i];
      if(a=="--explain"){ ex=true; }
      if(a=="--json"){ js=true; }
      if(a=="--tri"){ outMode=1; }
      if(a=="--no-answer"){ outMode=2; }
      if(a=="--normal"){ outMode=0; }
      if(a.rfind("--lang=",0)==0) lang=a.substr(7); }
    return verify(argv[2],ex,js,codes,domains,lang,outMode);
  }
  if(cmd=="uninstall"){
    std::cout<<"LTIR 卸载：请运行 bash uninstall.sh（只删 ~/ltir_lab 与 hook，不碰 .ltir 卷）\n";
    return 0;
  }
  std::cerr<<"ltir006: unknown command "<<cmd<<"\n"; return 2;
}

