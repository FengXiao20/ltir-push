# LTIR 元编程关键词表（立法稿 · 待批）

**基底**：2026-11-27 拉到的最新 main · `VERSION 0.1.1` · 84 文件 · SMOKE 445/445 · regress 81/81
**状态**：纸面立法，未落实现、未编译、未推仓库、`lab_codes.tsv` 未 append
**建议入仓名**：`META_KEYWORDS.md`

---

## 0. 为什么先立名字

近期那串洞其实是一类病：C++ 里写死了一条公式 / 一个分支，没穷尽情况。

| 洞 | 漏了什么 |
|---|---|
| B8 `x²+1>=0` 弃权 | Δ<0 恒号分支 |
| `1/1e10` 弃权 | 除法绕去算 `y.a²`，多余公式撑爆 |
| `1.1E17` 不认 | 大写 `E` 没进 Number token |
| `define: g(t)` 返回 0 | 参数名写死 `"x"` |
| δ 嵌套 8 层弃权 | 步数写死 8 |

元编程的目标不是自举，是**让人用 `.ltir` 把缺的那条公式补上，不必每次重编译 C++**。名字没定就写实现，会重演同一病：`base:` 已被进制眼镜占、`con_base:` 才另起，`rule:` 早是 `by:` 白名单。

## 1. 五条总原则

1. **一词一义**：词条是动词（登记 / 分派 / 反射），规则名是名词短语，不许混
2. **撞名即弃权**，不静默覆盖；落包前逐一对 `isKeyword` 表
3. **参数名永不写死**：`define: g(t)` 的教训，签名里出现 `x` 只当示例、不当默认
4. 卷面全 **snake_case 小写**；C++ 新增统一 `dispatch*` / `reflect*` 前缀，老名 `computeTheorySig` / `divv` 不动
5. **元层 0 三态、不产 True** —— 与强度值标签、不定式定理卷同纪律

## 2. 顶层新词条（拟 4 + 修饰 2，复用 2）

| 词条 | 作用 | 三态 |
|---|---|---|
| `gap:` | 登记一个「未周全点」：id + 触发模式 | 0 |
| `reformulate:` | 把 gap 绑到一条精确表达式（带 `by:` 即生效） | 0 |
| `quote:` | 一段 `.ltir` 当展示型数据登记，**不 eval** | 0 |
| `reflect:` | 只读列 `gaps` / `rules` / `codes` | 0 |
| `sig:` / `by:` | **复用已有**：规则签名 / 证书锚，不新增 | — |
| `depth:` / `trace:` | `expand:` 修饰行：解 8 层写死 / 吐展开轨迹 | 0 |

**收敛说明**：上轮草拟的 `case:` 本轮实测**已被占用**（0.0.6 分析组，`c.cases` 只登记不判），语义不同，弃用；`dispatch:` 与 `reformulate:` 抢一件事，只留后者。`rule:` 保留原义，不动。

## 3. 占用核对（本轮读码实测，非纸面）

- **已占不可复用**：`case:` `rule:` `base:` `sig:` `define:` `expand:` `theory_sig:` `grammar:` `preset:` `load:` `theory:` `lang:`
- **已入仓**（强度草案试验版被推上了 main）：`sig_mode:` `ordinal:` `benchmark:` `metatheory:` `notation:` `con_base:` `by:`
- **已核空闲**（源码 0 次）：`gap:` `reformulate:` `depth:` `trace:` `quote:` `reflect:` `dispatch:` `macro:` `template:` `bootstrap:`

> ⚠️ 如实报：`PATCH_STRENGTH_TRIAL.md` 原写「本地试验、不推仓库」，现已出现在 main 顶层文档里；本草案以「已入仓」为基底。

## 4. 函数 / 规则命名规范（主菜）

```
sig: <fn> : <dom> -> <codom>
reformulate: <fn>(<args>) = <exact-expr>
by: <cert-id>
```

- 内置规则一律 **`ltir_` 前缀**：`ltir_quad_sign` / `ltir_rat_div` / `ltir_sci_number`
- 用户卷面规则**禁带 `ltir_`** —— 前缀就是防覆盖的护栏
- 参数：有理系数用 `a,b,c,d`；`x,t,u` 留给用户变量，引擎不许默认成 `x`
- 规则名只许 `[a-z][a-z0-9_]{2,31}`，禁大写开头

## 5. 禁用名

大写开头 · `E` 开头（撞科学计数法）· `con` `inf` `oo` `i` `e`（保留符号）· `base` `rule` `sig` `case` `gap` `reflect`（已占词条）· 单字母

## 6. 错误码（待点头，不擅自 append）

| 码 | 含义 |
|---|---|
| `E-RULE-GAP` | 命中缺口但无 `reformulate:` |
| `E-RULE-CONFLICT` | 两规则同模式，弃权不择一 |
| `E-RULE-SIG` | 签名 / 参数名不符 |

连同仍挂着的 `E-STRENGTH-*`（NOCERT / INCOMPARABLE / CYCLE）与 Gödel II 独立码，统一走「码表立法权在你」。当前 `lab_codes.tsv` 68 行，`E-INDETERMINATE` 已入表。

## 7. 示例卷（全卷 0 三态）

```
# fixtures/metaprogram_names.ltir
sig: ltir_quad_sign : Poly -> Set
gap: quad_disc_neg      where: disc < 0
reformulate: quad_disc_neg(A, op) = sign_table(A, op)
by: cert-quad-002
quote: <ltir> V: x^2 + 1 >= 0 </ltir>
reflect: gaps
```

## 8. 四条护栏

- **只填空，不覆盖**：分派表只接管原本 `UNDECIDED` 的分支；已有 True/False 一律不许改，`2+2==5 -> True` 照样 False，`case: ... -> True` 也不生效 —— 得写表达式，由引擎照旧算三态
- 规则本身 0 三态且带 `sig:` + `by:`，指纹进 `theory_sig:`
- 撞模式弃权；`quote:` 只登记不 eval
- 零浮点、有限步、同卷重跑逐字一样

## 9. 永久锁

自举执行自产代码 · 运行时写回码表 · `grammar:` 生成解析器（SYNTAX 349）· 宏里跑判定核 / 元层产 True · 自动跨图搬真值 · C++ 模板 / constexpr 化（重签 `core/` 11 件）

## 10. 待裁

- `depth:` 与源码局部变量 `int depth` 同名（不冲突）；嫌混可改 `expand_depth:`
- `gap:` / `reformulate:` 是否换成 `gapdef:` / `rewrite:`
- 错误码三行批不批

---

**口径**：纯纸面立法，没落盘实现、没编译、没推仓库、码表未动，`VERSION 0.1.1` 原地、`core/` 11/11 OK。

---

## 0.2.0 落包补记（开工后追加）

- **撞名实测**：`case:` 已被 0.0.6 分析组占用（`c.cases`），新分派表只留 `reformulate:`；
  `gap:` / `reformulate:` / `depth:` / `trace:` / `quote:` / `reflect:` 六个候选在 0.1.1 源码
  逐词核对均为 0 次，全部确认空闲。
- **错误码收敛**：拟议的 `E-RULE-SIG` 与现成 `E-DEF-SIG`（level 3，签名不符）语义重叠，
  **并回 `E-DEF-SIG`**；冲突检测也复用它。`E-RULE-GAP` / `E-RULE-CONFLICT` 保留为
  `META_CODES_CANDIDATE.tsv` 候选行，**未 append 码表**（立法权在你）。
- **历史卷**：`fixtures/history_gaps.ltir` v1（0.1.2 包，纯登记词）→ v2（0.2.0 包，
  `gap:`/`reformulate:`/`witness:` 正式标本）。每个已修洞一行「旧错 → 修后输出」，
  正好充当规则的 `witness:` —— 复现不了就弃权，规则自己也不许写错。
- **单主干，不发第二份发行版**：0.1.2 → 0.2.0 顺序叠加，试验只放
  `fixtures/*_trial.ltir` + `tests/trial_meta.sh`；远程仍只有 `main`，tag 仍只有 `v0.0.9`。
- **使用现场（先看见，再补）**：`reflect: gaps` 列已登记 id；「弃权原因码 → gap id」
  的自动暴露本期**未做**，留 0.2.1。用户仍需读历史卷才知道补哪儿，这层如实不假装完成。
- **分派表边界**：本期六词只登记 + 签名/冲突校验，`reformulate:` **不自动接管**
  原本 UNDECIDED 的分支；0.1.2 三条手修仍是内置默认规则。自动接管留 0.2.1。
  `quote:` 不 eval、`grammar:` 仍只收 BNF、自举永久不做。
- **验收新增**：`tests/trial_meta.sh` 自检；`--json` 追加 `meta_gaps/meta_rules/meta_quotes/meta_depth`
  四个结构字段；SMOKE 与 regress 分组标题已去版本号，不因新增断言改文案。
- 回退：删四个新词分支即退回 0.1.2 行为，三条 bug 修复互不影响。

---

## 0.2.1 纯实践收编：五词立法（已放权，非发行）

四项元能力各占一个**工位词**，不再塞进旧词 body 里当隐式约定（零新词试验已作废）。
五词全进 `isKeyword`，`expand:` 还给 δ 展开、`grammar:` 仍只收 BNF、`quote:` 不 eval。

| 词条 | 工位 | 只准干的事 | 三态 |
|---|---|---|---|
| `shape:` | 认形状 | 登记长相 + 槽位名 + 转写目标，**不生成解析器** | 0 |
| `bind:` | 补全 | 槽位 → 标识符/字面量，补不出就空着，**不猜、不产值** | 0 |
| `rewrite:` | 展开 | 有限步摊成旧词、回投原判定核，**自己不出 True/False** | 0 |
| `split:` | 拆目标 | 拆成 `gap:`+`witness:`+`by:`，**没有 close** | 0 |
| `mode:` | 门禁 | `strict`（缺省，只登记）/ `trial`（才回投），卷面自报 | 0 |

- **一词一义**：`bind:` 管槽位映射、`at:` 管常量绑定，不许糊；`expand:` 不再兼职改写。
- **撞名即弃权**：同 shape / bind / rule id → `E-REWRITE-CONFLICT`，绝不 last-wins（作废版 bug 1）。
- **四项专码已放权定稿**：`E-SHAPE-UNKNOWN` / `E-REWRITE-ARITY` / `E-REWRITE-CONFLICT` /
  `E-REWRITE-REDEEM`，`lab_codes.tsv` 65 → 69，历史行未动，不再可删回 65。
- **L3 全禁**：元操作行、`quote:` 内层、`witness:` 体先验拒收**全部手写三态**
  （`-> True` / `-> False` / `-> UNDECIDED`，含大小写与中文弃权词）与判定词；
  判定核另把「词库外判定词」补回全禁版（`-> Maybe` / 乱写尾巴 → `E-SYN`）。
- **L5 有出口**：`reflect: rewrites/shapes/binds/splits/mode` 人话档逐字列出，
  `--tri` / `--json` 零污染（作废版 bug 5 / r2 bug B 的实证修复）。
- **给命名权，不给兑换权**：唯一 True/False 出口仍是原判定核；`split:` 不关闭目标，
  `con: T->T` 仍 `E-GODEL-II`，自举永久不做。
