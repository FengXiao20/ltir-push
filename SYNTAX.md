# LTIR 0.0.6 语法立法文本（关键词冻结表）

> 适用范围：0.0.6 新语言层（`src/`）。`core/` 十一件内核与 v0.0.1 文法一字不动。
> 语言跟层走：本层纯 C++ / Linux + Termux。Windows 分支冻结。

---

## 一、三条设计约束

| 令 | 落成规则 |
|---|---|
| 用英文做关键字 | 所有行首身份词一律英文或英文缩写，不用中文当关键字 |
| 能简写就尽量简写 | 每个关键词给「长名 + 短写」两形，短写优先用 |
| 不能简写到只有一个单字母 | **禁止裸单字母关键字**：关键字必须带 `:` 或本身多字母 |

**禁令精确边界**（最易读歪的一条）：

- ❌ 行首孤零零 `A` / `V` / `S` / `N`（无附号）→ `E-UNK-LINE`，硬错退出 1
- ✅ `A:` `V:` `id:` `der:` `seq:`（带冒号 / 多字母）
- ✅ `i` `e` `π` `x` `f` `R` —— **标识符 / 常量**，不是关键字，禁令管不到

即：**禁的是关键字写法，不是数学符号写法**。两者靠「行首身份位 vs 表达式内部」分开，
不靠白名单枚举。短写表冻结，新增关键词默认只给长名。

---

## 二、行首关键词表（定稿，22 长名）

| 长名 | 短写 | 职责 | 写法 | 结果 |
|---|---|---|---|---|
| `assert:` | `A:` | 断言判定，**真值自产** | `A: 2+2=5 -> True` | `False E-VAL-MISMATCH` |
| `value:` | `V:` / `val:` | 纯求值，不判对错 | `V: 1/3+1/6` | `1/2` |
| `at:` | `at:` | 代入环境，到下一个 `at:`（空行清空） | `at: x=3, y=2/3` | 登记 |
| `ident:` | `id:` | 恒等判定（符号多项式 canonical 比） | `id: (x+1)^2 == x^2+2x+1` | `True` |
| `numth:` | `N:` / `nt:` | 数论求值 / 判定 | `N: factors(360)` | `2^3 × 3^2 × 5` |
| `given:` | — | 已知条件登记 | `given: a>0, b>0` | 登记 |
| `define:` | — | 定义函数（多项式） | `define: f(x)=x^2-3x+2` | 登记 |
| `solve:` | — | 一元二次不等/方程求解 | `solve: x^2-5x+6 < 0` | `x ∈ (2, 3)` |
| `derive:` | — | 求导 | `derive: f'(x)` | `2*x - 3` |
| `sum:` | — | 求和（∑ 的 ASCII 关键字形） | `sum: n=1..10 of a_n` | `120` |
| `seq:` | — | 数列 / 递推登记 | `seq: a_n = 2n+1` | 登记 |
| `limit:` | — | 极限（有理式最高次系数比） | `limit: n->inf of 1/n` | `0` |
| `vector:` | — | 向量点积 / 叉积 | `vector: cross([1,0,0],[0,2,0])` | `(0, 0, 2)` |
| `prob:` | — | 排列组合 / 概率 | `prob: C(5,2)*(1/2)^5` | `5/16` |
| `stat:` | — | 均值 / 方差 | `stat: mean([1,2,3,4])` | `5/2` |
| `set:` | — | 有限集登记 | `set: A = {1,2,3}` | `{1, 2, 3}` |
| `domain:` | — | 领域设定 / 列表 / 查函数定义域 | `domain: trigonometry` `domain: ?` `domain: f` | 登记 / `24 domains registered` / `R` |
| `case:` | — | 分类讨论分支登记 | `case: x>=0 -> ...` | 登记 |
| `check:` | — | 人的验根（**不产真值**） | `check: x=1 in sqrt(x+3)=x-3` | 登记 |
| `const:` | — | 单字符数学符号开放注册 | `const: phi = (1+sqrt(5))/2` | 登记 |
| `note:` | `#` | 注释，机器跳过、原样保留 | `# 这题分类讨论` | 跳过 |
| `subject:` | `<s>` | 题目标签容器 | `<s> ... </s>` | 容器 |
| `zf:` / `zfc:` | — | 台账登记，**只记不判**（保留项） | `zf: Riemann: ...` | `registered` |

**沿用不改**：`A:` `at:` `id:` `V:` `N:` `<a>` 容器、`<subject>`/`<s>`、`#`、三态。

> **【强化修复版·乙裁决】撤销首版越界条款**
> 首版第 56 行写过「明确不用 `S: E: P: Q:`（跨层串台源）」—— 那是**施工侧自行否决
> 旧标签体系**，立法者从未授权。本版按乙裁决整套并回，该行作废、记档不复用。
> 裸 `C:` 仍封死（字段不配简写，规约 2）。

**四件套标签（解题文本层，并回 0.0.6）**

| 标签 | 职责 | 行为 |
|---|---|---|
| `<calc>` / `<a>` | 演算 | 算式段照抽；`<过程>` / `<proc>` 输入层归一为 `<calc>` |
| `<C_and_E>` | 推理 | `cause:` / `effect:` / `by:` 三字段登记；**不产三态行** |
| `<answer>` | 答卷 | `A:` 归一化的存档态；`--json` 带 `tag:"answer"` |
| `<subject domain="…">` / `<s>` | 题目 | `domain` 属性进 Ctx，可查 |

**装饰简写（整行，机器跳过、原样保留、渲染加粗）**

| 简写 | 中文 | 位置 |
|---|---|---|
| `S:` | 解 | `<calc>` 内 |
| `E:` | 存在 / 设 | `<calc>` 内 |
| `V:` | 验 / 检验 | `<calc>` 内 —— 0.0.6 里**仍是 `value:` 求值短写**，只额外登记装饰身份 |
| `P:` | 证明 | `<C_and_E>` 内 |
| `Q:` | 证毕 | `<C_and_E>` 结尾 |

字段全拼小写（`effect:`）与简写行（`E:`）靠大小写 + 全拼分界，互不越权；
中文装饰词（解：设：答：）容器内免标记自由书写，原文进 `preserved` 可查。

**容器是多段状态机，不是「挑一段」**：中文 / 中文标点当分隔符，收集**全部** ASCII
算式段。容器内「设 / 令 / 取 `x=5`」形如「单标识符 = 可求值数值」的段绑定环境
（容器闭合恢复，不跨容器泄漏）；`x^2=25` / `2+2=4` / `1/3=1/2` 左侧非单标识符，
不会误绑。这是 L06 从 UNDECIDED 修成 True 的关键一刀。

---

## 三、单字符特例：开放集合（「不止三个」）

不写死 i / e / π，改用两条机制：

**① 位置限定**：单字符只许出现在**表达式内部**（常量 / 变量 / 函数名 / 点名位），
永不出现在行首身份位。行首解析器见孤字母即 `E-UNK-LINE`，不管它是不是数学符号。

**② 注册入口 `const:`**：新符号登记一次即可用，清单天然开放。

```
const: i     = sqrt(-1)      const: e   = exp(1)
const: pi    = 圆周率符号位   const: phi = (1+sqrt(5))/2
const: tau   = 2*pi          const: inf = 1/0
const: N     = {0,1,2,...}   const: R   = ℝ
```

实证六类，全合法、全不是关键字：
- 常量 `i e π φ τ ∞ γ`
- 数集 `N Z Q R C` / `ℕ ℤ ℚ ℝ ℂ`
- 变量 `x y z t n k m`
- 系数 `a b c d`
- 函数 `f g h`
- 点 / 事件 `A B C P Q L`

---

## 四、表达式能力（P1 已实现）

```
四则 + - * /      幂 ^（右结合）        阶乘 !（≤20!，21! 弃权）
隐式乘 2x / x(x+1) / 2(x+1)            分数小数 1/2 / 0.25
比较 = == != < > <= >=（0.0.6 解禁）  LaTeX \frac{A}{B}
√ π ∞ × ÷ − ⇒ ² ³ ≤ ≥ ≠（入口归一）  三角特殊角精确值 sin/cos/tan
复数 i（i²=-1）                       代数数 a+b√r
∞ 哨兵（1/0 → ∞；∞−∞ / ∞×0 → UNDECIDED）
数论 isprime nextprime factors gcd lcm phi modpow C P
函数 define f(x) / f(3) / f'(3)       多项式恒等 id: / 求导 derive:
 Richardson 界：含 π/e 的非特殊角等式 → UNDECIDED，禁止伪判定
 ^0 常量折叠发生在 Richardson 判据之前（π^0 → 1，不误杀）
```

**真值自产铁律**：`-> True/False` 只是**声明词**，引擎洗净后实算核对；
词库 = `True / False / true / false / 成立 / 不成立 / 真 / 假`，**位置双锁**（仅行尾
`->` 后），词库外不判定不报错。`=True` 写法不产生判定记号，按普通表达式处理。

---

## 五、三态与退出码

| 结果 | 含义 | 退出码 |
|---|---|---|
| `True` | 实算与断言一致 | 0 |
| `False` | 实算不符（附码表解释） | 0 |
| `UNDECIDED` | 越界 / 溢出 / 未定形，诚实弃权，**严禁并入 False** | 0 |
| `ERROR` | 语法立法硬错（`E-UNK-LINE` / `E-SYN` / `E-NAME` / `E-EXT`） | **1** |

登记行（zf/zfc/given/case/check/const/define/seq/set）**不进三态统计**，
只在 `registered:` 计数里出现 —— 三态锁死不许加第四态。

---

## 六、码表（只读）

`lab_codes.tsv` 独立命名空间，不改 002 的冻结表；`state` 1=ERROR / 2=False /
3=UNDECIDED，`en`/`zh` 双列由卷内中文自动跟随，`{lhs} {rhs} {diff}` 为占位符。
码：`E-UNK-LINE` `E-SYN` `E-NAME` `E-EXT` `E-VAL-MISMATCH` `E-ID-DIFF`
`E-SCOPE-TRANSCENDENTAL` `E-SCOPE-SYMBOLIC` `E-INF` `E-OVF` `E-RANGE`。
**append 权在立法者，引擎不得改码表。**

---

## 七、入口防护（进管线第一件事）

1. CRLF → LF
2. BOM（`\uFEFF`）剥离
3. 全角 → 半角（`Ａ：→A:`、`＃→#`、全角空格收掉），中文 IME 主路径不静默漏判
4. 数学 Unicode → ASCII 别名；中文 / 域外记号 `∪∩⊆` 原样保留，**不拦截不验证不解释**
5. 逐行扫描建立在归一化后的文本上，管线内部不再考虑兼容

---

## 七之二、领域注册表（`domains.tsv`，强化修复版扩增）

`<subject domain="X">` 与顶层 `domain: X` 共用一张表，**24 个领域**：

```
math            algebra        equation       inequality     polynomial
function        sequence       limit-calculus trigonometry   vector
complex         geometry       solid-geometry analytic       combinatorics
probability     statistics     number-theory  set            logic
induction       linear-algebra zfc            physics-math
```

- 属性与顶层设定都写进 `Ctx.domain`，`--json` 带出 `domain / domains_registered / domains_seen`
- `domain: ?` 列出已注册领域数；`domain: f` 仍查函数定义域（首版行为，未回归）
- **未知领域原样登记、不拦截不报错不解释**（域外记号标黄放行精神）
- 领域表是**语法层资产，不是错误码表**：扩增不改 `lab_codes.tsv`，append 权在立法者
- 领域只做登记与可查，**不改判定逻辑** —— 域能差不靠转换硬抹平

## 八、解禁 / 保留 / 永久不做

**按「继续扩大功能」令解禁**（旧令里属 v1.2 / 禁手，高中体量绕不开）：
`f(x)` 函数定义与求值、递推 `a_n`、`∑` 求和、`< >` 比较符。

**按强化修复版·乙裁决解禁**：`S: E: P: Q:` 装饰简写、`<calc>` / `<C_and_E>` /
`<answer>` 四件套、`cause:/effect:/by:` 三字段、`<过程>/<proc>` 归一、领域表扩增到 24 个。
首版「明确不用」条款作废，见第二节撤销记档。

**保留不动**：`zf:` / `zfc:` 台账语义（只登记不判）。
**按最新令覆盖**：三门语言架构表（JS 脸 / Python 心 / MCP 嘴）已退场，纯 C++ 自包含。

**永久不做**：ZF/ZFC 公理判定核、符号求解器、多变量函数方程、超越等式伪判定、
集合论公理化运算、i18n 多语言报错（1.x 议）、C++ 性能核（预留位，无令不动工）。

---

## 九、分期（一本高中数学体量）

- **P1 地基（本令已交）**：关键词冻结、入口防护、精确 ℚ/代数数/复数、三态、码表、
  define/at/derive/solve/sum/seq/三角/数论/向量/概率统计/集合/ZF 保留
- **P1 强化修复版**：四件套标签并回、装饰简写整行跳过、容器内多段状态机与代入绑定、
  领域表 24 个、Termux clang++ 真机取证、L06 修复、69/69 SMOKE
- **P2 主体**：分段函数、参数方程与极坐标、解三角形、立体几何、线性回归、分布列、
  复合 LaTeX 的 AST 级折叠
- **P3 边界**：输出渲染标黄、浏览器宿主、中文数学术语词典、`--lang` 全量解释

每期只列不做，等新令。

## 0.0.7 · 集合论实验区（本令：护栏降级，不是本体封条）

旧条文（002 铁律 4「∪∩⊆ 标黄放行」、003 禁区「集合论运算永久不做」、
`SYNTAX.md:202` 公理化判定核永久不做、`zf:/zfc:` 只登记不判）**原为控步子的范围护栏**，
0.0.7 起整批降级为实验区；历史交付物的验收记录不回退。**仍锁**：三态锁死、
真值自产、Richardson 不伪判、码表只读消费、core 11 件 SHA `aecc9726…` 零改动。

### 语法总则（沿用）
全英文关键字、禁裸单字母行首身份位；`i e π x y z A U` 是标识符不是关键字；
**新增词只给长名，短写表维持冻结**；`V:` 仍是 `value:` 求值短写，`-V` 仍是版本参数，
`V=L` 整串保留不拆。

### 地图坐标
`system:` 显式声明地图；`choice: on|off|dc|cc|bpi` 选择公理图层；
`axiom:` 本图启用公理；`ref:` 外部证书；`⊢` 标「在图内相对可推导」，
`⊨` 标语义蕴涵（需证书，弃权）。两者都不产第四态。
无 `system:` 而用 `⊢` / 大基数 / 全局 V → `E-SCOPE-NO-MAP`。

### 全局 V（三重消歧）
`universe: V` 仅在 `set / foundations / zfc / model-theory / naive / logic` 域且
显式 `system:` 下成立；V 是**真类**不是集合。`univ: U = {…}` 是有限/相对全集；
`card(V)` / `V ∈ V` / `|V|` → `E-SCOPE-PROPER-CLASS`。
非集合论域写 V → `E-SCOPE-DOMAIN`（V 专属门禁，不推翻「未知领域标黄放行」）。

### 集合与势（有限集精确，无限集诚实弃权）
`∈ ∉ ∪ ∩ ∖ ⊆ ⊇ ⊂ ⊃` 升格为输入算子；`⊆`=子集、`⊂`=真子集，**不合并**。
`\subset` 学术上常作子集 → 归一为 `subseteq`。
行标签只认 `card: A`（有限集精确判势）；`|A|` 仅为展示记号，不是行标签，
`card: |A|` → `E-REF-UNKNOWN`。`|A| <= |B|` 势比较走 L0 登记层；
无 AC 地图下基数不可比 → `E-CHOICE-INCOMPARABLE`（不落 False）。
`{2n : n ∈ N}` / `{0,1,2,...}` 构造式**只登记、不猜通项、不自动展开**；
嵌套集合不压平 → `E-SCOPE-NESTED`。未登记符号 → `E-REF-UNKNOWN`（严禁并入 False）。

### 量化与引用
`forall: t in A: expr` 有限集逐元素可判；连续域 `forall: x, y, z` /
`claim:` → `E-SCOPE-QUANTIFIER`。`under:` 约束域先只登记；
`cert:` / `witness:` / `problem:` / `status:` / `theory:` / `inner:` /
`map:` / `independent:` / `model:` / `cite:` 只登记不产三态行。
`by:` 行尾引用查「已加载公理表 + ref:」，**只是名字不是免算金牌**；
未登记 → `ref_unknown` 计数，不替引擎判真假。

### 公理模块（一个 `.ltir`，两种身份）
`axioms/` 放模块，`fixtures/` 放卷；**不新开后缀**（后缀是给 OS 看的，不是给引擎看的）。
模块首行 `module:` 声明身份，**禁止含 `A:`/`V:` 断言行**，只含
`system:/choice:/axiom:/ref:`；加载只动 `registered`/公理表，三态一位不许变。
`preset: NAME` 内置按名加载（跨端零路径分歧、幂等）；
`load: axioms/NAME.ltir` 外部 `.ltir`。循环 → `E-LOAD-CYCLE`、
缺失 → `E-LOAD-MISSING`、断言行 → `E-LOAD-ASSERT`、公理打架 → 登记元定理弃权。

### 防「沙」（省略号折叠 + 彩蛋）
连续 n 个省略号（`......` / `... ...` / `… …` / `\dots\dots`）一律折叠成**单个 `...`**；
保护 `1..10`（两点）与 `0.5`（一点）。触发时吐一句：
中文「你多打了省略号，已经补全了。才不是帮你呢。」／英文
"You typed extra ellipses; normalized to one. N-not like I did it for you or anything!"
**末句傲娇是彩蛋**，`--json` 单列 `easter_egg`；`--lang=zh|en|auto` 跟随语言。

### 错误码（append，只读消费）
`E-SCOPE-*`（QUANTIFIER / INFINITE / INDEPENDENT / INCOMPLETE / NO-MAP /
DOMAIN / PROPER-CLASS / ABSOLUTE / SEMANTIC / NESTED / SINGULAR）、
`E-PARADOX-RUSSELL`、`E-AXIOM-CONFLICT`、`E-CHOICE-*`（REQUIRED /
CONFLICT / INCOMPARABLE / STRENGTH）、`E-REF-*`、`E-LOAD-*`、
`E-SET-COMPREHENSION`。全部 `UNDECIDED`，**严禁并入 False**。
码表 state 列仅文档口径，运行时以 `codeRes` 调用处为准（两套编码统一待裁）。

### 学术口径修正（已核对，详见 AXIOM_NOTES.md）
单个 Woodin 弱于超紧，只有「真类多 Woodin」才更强；I0 在 ZFC 内、与
Reinhardt/Berkeley（ZF 无 AC 线）**不是线性台阶**；Levy–Solovay 只封杀
「真值可被小 forcing 改变」的命题，「大基数不决定 CH」是其推论；
SCH / Silver / pcf 上界均带「强极限 / 不可数共尾度」限定词。
CH 是希尔伯特第一题、不在 CMI 七问内，本版按独立命题专档登记。

## 0.0.7 补丁 · 甲条文（实验区续）

1. **仅台账域**：`domains.tsv` 第三列以 `[台账]` 结尾者为「仅台账域」。此类域只有名字与
   范围，**没有判定核**。域内 `A:` / `V:` / `N:` / `claim:` / `forall:` / `card:` /
   `prob:` / `stat:` / `solve:` / `limit:` / `vector:` / `sum:` / `seq:` / `derive:` /
   `ident:` 涉及域专有记号的表达式一律 `UNDECIDED E-SCOPE-LEDGER-ONLY`；
   登记类行照常登记、不产三态行。
   **0.1.1 细化**：`A:` / `V:` 中剥离声明词后不含任何字母的纯有理算术
   （如 `1+1==2`）与域无关，照常精确判，不算「借别域判定核」。
2. **不跨域串台**：仅台账域不复用 `math` 域求值器。能算 `2+2==4` 不等于「懂 analysis」；
   涉及域专有记号的表达式仍诚实弃权，优先于借别域的判定核粉饰。
3. **三态锁死不变**：弃权不是 False、不是硬错、exit 0；`-> True` 仍只是声明词。
4. **老域逐字不变**：原有 27 域的判定与汇总不受扩域影响；未知领域仍标黄放行。
5. **码表只读消费**：`E-SCOPE-LEDGER-ONLY` 由 append 落表，引擎不硬编码解释文本，
   `--lang=zh|en` 跟随码表。
6. **短写表仍冻结**：甲不开新短写、不新增判定算子、不新增关键字；`V:` 语义不回退。
7. **集合语义入口归一**：`A:` / `V:` / `N:` 共享有限集语义块；带 `-> True/False` 时先剥
   声明词再判集合，真值自产不变，声明词与实算一致时不得残留错误码。
8. **防「沙」纪律不变**：多打省略号折叠为单个 `...`，中英傲娇彩蛋单列 JSON，
   单个 `...` 与 `1..10` 两点范围不误伤。


## 0.0.7 补丁二 · 集合论实验区（本节覆盖甲阶段第 6 条「不新增关键字」）

版本 `0.0.7` **一字不动**，无新后缀。新增 6 个全英文长名关键字，短写表仍冻结：
`exists:` `cex:` `inj:` `surj:` `bij:` `mapping:`

1. **求值标签 ≠ 断言标签**：`V:` / `N:` 算出 False 是合法求值结果，
   **不挂** `E-VAL-MISMATCH`；该码只属于 `A:` 声明词与实算冲突。
   `forall:` / `exists:` / `inj:` 等逐元素判假同样无码。
2. **有限集量词**：`forall: x in A: P(x)` / `exists: x in A: P(x)`；
   域可是具名有限集或字面量 `{...}`，按**花括号配对**取域（避开 `{1,2,3}` 内部逗号）；
   连续域 / `...` 构造式 / 未登记名一律 `E-SCOPE-INFINITE`。
3. **原生量词符号**：`A: ∀ x ∈ {1,2,3}, P(x) -> True`，`∃` 同路（遍历找见证即可）；
   gate 归一为 `forall ` / `exists ` / ` in `。字面集合域后必须**显式跟 `,` 或 `:`**，
   缺分隔符 → `E-SCOPE-QUANTIFIER`（防误伤，不宽松吞裸表达式）。
4. **受限概括**：`set: R = {x ∈ {1,2,3} | x > 1}` 左边绑定域、右边当条件，
   逐元素精确算，输出 `{2, 3}`。无受限域的 `{x | x ∉ x}` → `E-PARADOX-RUSSELL`；
   该名字之后的 `∈` / 势判断按词边界一律弃权（塌图登记，不判真假）。
5. **反例登记册**：`cex: x = -1` 把值代入上一行 `A:` 的 lhs（剥完声明词）；
   使命题失效 → `True` + `反例有效` / `counter-example valid`（语言跟随）；
   仍成立 → `False E-CEX-INVALID`；无上一行 → `E-REF-UNKNOWN`。
6. **有限映射性质**：`mapping: f = {1->2, 2->3} on {2,3}` 定义有限映射（只登记，
   不产三态行，陪域用 `on {...}`）；`inj:` / `surj:` / `bij:` 查表判 True/False；
   `R -> R` 或非 `{...}` 定义 → `E-SCOPE-INFINITE`；`surj:`/`bij:` 缺陪域 →
   `E-SCOPE-CODOMAIN`。判假无码。
7. **仍锁**：三态 · 真值自产 · Richardson 不伪判 · 码表只读消费 · core 11 件 SHA
   · 三角囚徒冻结卷 `10/10` · 原 175 条回归 · 版本 007 焊死 · `./ltir006` /
   `~/ltir_lab` / hook 三锚点不动。

## 0.0.8 条文（形式化定义 / 多宇宙图册）

- 版本 `0.0.8`；版本身份另存 `VERSION`，底片 `LTIR_STAMP` 驱动收容重搬。
- 启动脚本 `ltir.sh`；二进制名 `ltir006`、收容目录 `~/ltir_lab`、hook 三锚点不动。
- 新关键词（全英文长名，短写表仍冻结）：`sig: type: where: rule: grammar: expand:`
  `multiverse: world: force: ground: mantle: reachable:`。
- `define:` 支持 `define: f(x : Num) : Num = ...`；签名不符 → `E-DEF-SIG`，
  自由标识符 → `E-DEF-UNBOUND`，自引用递归 → `E-DEF-NONTERM`，
  `expand:` 最多 8 步 δ 展开，超界 → `E-DEF-DEPTH`。
- `rule:` 登记的名字进入 `by:` 证书链白名单；`grammar:` 只登记 BNF，不生成解析器。
- 多宇宙：`world:` 登记世界，`force:/ground:` 登记可达边（缺世界 → `E-SCOPE-NO-MAP`），
  `reachable:` → `E-SCOPE-CROSS-WORLD`，□◇ → `E-SCOPE-MODAL`。
  S4.2 / K·T·4·.2 只作 `rule:` 模式登记，**不实现模态定理证明器**。
- 图间模式**不**解锁绝对真值：`⊢` 仍是相对可推导，`-> True` 仍是声明词。
- `IND[]` 收缩为 CH / Continuum / 哥德尔不完备；七问、哥德巴赫改走 `E-SCOPE-OPEN`。
- 一个 `.ltir` 两种身份不变；`axioms/Multiverse.ltir` 为登记模块，不产三态行。

## 0.0.9 条文

- 本节记录 0.0.9 增量；当前版本 `0.2.0`，条文见文末「0.1.0 条文」与「0.1.0 A 增量」。
- **隐式乘法**：多项式层把连写变量 `2xy` 拆作 `2*x*y`；函数名、常量
  （`pi`/`i`/`e`/`oo`/`N`/`R`…）与已知单名不拆。`eval` 数值层原已支持。
- **受限概括**：`{x in A | P(x)}`、`{x | x in A and P(x)}`、
  `{x | x in A}` 三种写法共用逐元素精确判定；`A` 为无限/构造域 →
  `E-SCOPE-INFINITE`；真·无受限域 → `E-PARADOX-RUSSELL`（塌图登记，不判真假）。
- **溢出归类**：纯数字阶乘 >20!、纯数字幂在 `power()` 判 UND → `E-OVF`；
  双根式参与四则/比较 → `E-SCOPE-ALGEBRAIC`；非特殊角仍 Richardson 弃权。
- **π/12 家族**：`sin/cos/tan(k*pi/12)`（k=1,5,7,11,13,17,19,23）给精确值；
  `RAD2 = a + b√r + c2√r2` 是展示用精确类型，不参与四则运算（弃权不伪判）。
- **输出模式**（CLI flag 优先于环境变量）：
  - `--tri` / `LTIR_OUTPUT=tri` / `LTIR_TRI=1` → 每行只输出
    `True` / `False` / `UNDECIDED`，无行号、码、答案、汇总、彩蛋；
    硬错行在此模式折叠显示为 `UNDECIDED`（内部计数不变）
  - `--no-answer` / `check 卷.ltir` / `LTIR_OUTPUT=no-answer` →
    抑制答案值 `R.out`，三态与码照出
  - `--normal` 恢复；`--tri` 覆盖 `--json` / `--explain`
- **卸载**：`./ltir006 uninstall` 只打印指引；实际删除由 `uninstall.sh` 执行，
  范围为 `~/ltir_lab` 与 `~/bin/termux-file-editor`。
- 短写表仍冻结；一个 `.ltir` 两种身份不变；新码只读消费。

---

# English Edition (abridged translation)

> Scope: the 0.0.6+ language layer (`src/`). The eleven `core/` kernel files and the
> v0.0.1 grammar are untouched, word for word. This layer is pure C++ / Linux + Termux;
> the Windows branch is frozen.

## 1. Three design constraints

| Rule | Enforcement |
|---|---|
| English keywords | all line-head identity words are English or English abbreviations |
| Abbreviate when possible | every keyword has a long name + short form; short form preferred |
| Never a bare single letter | **bare single-letter keywords are forbidden**: must carry `:` or be multi-letter |

Precise boundary: a lone `A`/`V`/`S`/`N` at line start → `E-UNK-LINE`, hard error exit 1;
`A:` `V:` `id:` `der:` `seq:` are legal; `i e π x f R` are identifiers/constants — the
ban is on keyword spellings, not on mathematical notation. The short-form table is
frozen; new keywords get long names only.

## 2. Line-head keywords (22 long names, frozen)

Same table as §2 of the Chinese text: `assert:`(`A:`) asserts with self-produced truth;
`value:`(`V:`/`val:`) pure evaluation; `at:` substitution scope; `ident:`(`id:`)
polynomial identity; `numth:`(`N:`/`nt:`) number theory; `given:` `define:` `solve:`
`derive:` `sum:` `seq:` `limit:` `vector:` `prob:` `stat:` `set:` `domain:` `case:`
`check:` `const:` register or compute; `note:`(`#`) comments; `subject:`(`<s>`)
problem container; `zf:`/`zfc:` ledger-only (reserved). The four-tag set
`<calc>`/`<C_and_E>`/`<answer>`/`<subject>` with `cause:/effect:/by:` fields and the
decorative shorthands `S: E: P: Q:` were reinstated by the B-ruling (the first draft's
"explicitly not using S: E: P: Q:" clause was construction-side overreach, revoked and
archived). Bare `C:` remains sealed. Containers are multi-segment state machines:
Chinese prose acts as separator, all ASCII math segments are collected, and
"设 x=5"-style bindings enter the environment (restored on container close — the fix
that turned L06 from UNDECIDED into True).

## 3. Single-character rule: an open set

Single characters may only appear **inside expressions** (constants/variables/function
names), never in the line-head identity slot. New symbols register once via `const:`
(`i e π φ τ ∞ γ`, number sets `N Z Q R C`, variables `x y z t n k m`, coefficients
`a b c d`, functions `f g h`, points `A B C P Q L` — all legal, none are keywords).

## 4. Expression power (P1 implemented)

Four operations, right-assoc `^`, factorial ≤ 20!, implicit products `2x / x(x+1)`,
fractions/decimals, full comparison set (unsealed in 0.0.6), LaTeX `\frac{A}{B}`,
Unicode math input normalization, exact trig special values, complex `i` (i²=-1),
algebraic numbers `a+b√r`, `∞` as sentinel (`∞−∞`/`∞×0` → UNDECIDED), number-theory
functions, function define/eval/derivative, polynomial identity, Richardson bound:
non-special-angle transcendental equalities → UNDECIDED, no fake verdicts; `^0` folds
before the Richardson test. **Truth is self-produced**: `-> True/False` are declaration
words, washed out and recomputed; the vocabulary is position-locked after a line-end `->`.

## 5. Three states & exit codes

`True` (0) / `False` with reason code (0) / `UNDECIDED` — honest abstention, **never
merged into False** (0) / `ERROR` hard syntax fault (1). Registration lines never enter
verdict statistics; the verdict set is locked — no fourth state, ever.

## 6. Code table (read-only)

`lab_codes.tsv` is a separate namespace appended by the legislator only; `state`
1=ERROR/2=False/3=UNDECIDED, `en`/`zh` columns follow the scroll's language,
`{lhs} {rhs} {diff}` placeholders. Codes include `E-UNK-LINE` `E-SYN` `E-NAME` `E-EXT`
`E-VAL-MISMATCH` `E-ID-DIFF` `E-SCOPE-*` `E-INF` `E-OVF` `E-RANGE` `E-PARADOX-RUSSELL`
`E-AXIOM-CONFLICT` `E-CHOICE-*` `E-REF-*` `E-LOAD-*` `E-CEX-INVALID` `E-SCOPE-LEDGER-ONLY`
and more — all consumed read-only by the engine.

## 7. Input hygiene & domains

Entry pipeline: CRLF→LF, BOM strip, full-width→half-width, math Unicode→ASCII aliases;
Chinese / out-of-domain notation passes through unblocked. `domains.tsv` — shared by
`<subject domain="X">` and top-level `domain: X`: 24 base domains (math, algebra, …,
zfc, physics-math), extended by the 甲 patch to 55 registered with 28 **ledger-only**
domains where every judgment request returns `UNDECIDED E-SCOPE-LEDGER-ONLY` (a name
on the map is not a judgment kernel; honest abstention beats borrowing another
domain's evaluator). Unknown domains register verbatim — no interception, no errors.
Domains never change judgment logic.

## 8. Unsealed / reserved / never

Unsealed by order: `f(x)` functions, recurrence, `∑`, `< >` comparators, the tag set,
24→55 domains. Reserved: `zf:`/`zfc:` ledger semantics. Never: ZF/ZFC axiom verdict
kernel, symbolic solver, multivariate function equations, transcendental-equality fake
verdicts, set-theory axiomatic arithmetic, i18n errors (1.x), C++ performance core
(reserved slot, no order, no work).

## 9. Phasing & later legislation

P1 foundation delivered (keyword freeze, hygiene, exact ℚ/algebraic/complex, three
states, code table, full keyword set). **0.0.7**: the set-theory guardrails were
demoted to an experimental area — map coordinates (`system:`/`choice:`/`axiom:`/
`ref:`/`⊢`/`⊨`), the global-V triple disambiguation (V is a proper class; `card(V)`
→ `E-SCOPE-PROPER-CLASS`), exact finite-set cardinality & operators
(`∈ ∉ ∪ ∩ ∖ ⊆ ⊂`, `\subset` normalizes to subseteq), finite quantifiers (`forall:`/
`exists:`, native `∀ ∃`), restricted comprehension (`{x ∈ A | P(x)}`, unrestricted
→ `E-PARADOX-RUSSELL`), counter-example register (`cex:`), finite mappings
(`mapping:`/`inj:`/`surj:`/`bij:`), axiom modules (one `.ltir`, two identities:
`module:` header, no `A:` inside, `preset:`/`load:`), the anti-"sand" ellipsis
folding with tsundere easter egg, and academic calibrations (single Woodin < supercompact;
I0 vs Reinhardt/Berkeley are not one ladder; Levy–Solovay is the theorem whose
corollary is "large cardinals don't settle CH"; CH is Hilbert's 1st problem, not one
of the CMI seven). **0.0.7 patch 2**: six new long-name keywords `exists: cex: inj:
surj: bij: mapping:`; evaluation tags ≠ assertion tags (`V:` returning False carries
no code; only `A:` conflicts get `E-VAL-MISMATCH`). **0.0.8**: formalized definitions
(`sig:` `type:` `where:` `rule:` `grammar:` `expand:`, δ-expansion ≤ 8 steps,
`E-DEF-*` codes) and the multiverse atlas (`multiverse:` `world:` `force:` `ground:`
`mantle:` `reachable:`; S4.2 registered as patterns, no modal theorem prover; map
hopping never unlocks absolute truth). **0.0.9**: implicit product (`2xy` → `2*x*y`),
restricted-comprehension double syntax, overflow classification (`E-OVF`,
`E-SCOPE-ALGEBRAIC`), the π/12 exact-angle family (dual-radical display, no arithmetic
on `RAD2`), output modes (`--tri` / `--no-answer` / env vars), and uninstall
(`./ltir006 uninstall` prints; `uninstall.sh` deletes `~/ltir_lab` +
`~/bin/termux-file-editor`). Short-form table stays frozen throughout; version numbers
belong to the legislator — patches do not mint versions.

## 0.1.0 条文

- 版本 `0.1.0`；`core/` 十一件与 `RUN_core_sha.txt` 一字不动，门禁 11/11。
- **进制眼镜**：`radix: N`（2–16）设全文；`<2_nary>` / `<3_nary>` / `<二进制>` /
  `<八进制>` / `<十六进制>` / `<base=16>` 为局部容器，压栈-出栈，出栈恢复外层。
  只重解析数字字面量：base2 `1010`→10，base3 `0.1`→`(1/3)`，base16 `10`→16。
  闭合可写与开标签同名（`</base=16>` / `</16_nary>`），也可用短名
  `</base>` / `</nary>` / `</radix>` / `</进制>`（0.1.1）。
  标识符、函数名、符号常量（`pi`/`e`/`i`/`oo`）、`0x..`、`1..10` 不参与。
  非法数字 `E-LITERAL-BASE`；未闭合 `E-SCOPE-UNCLOSED`。默认 base=10 是全等 no-op。
- **赋值环境表**：顶层裸 `a=1+1` 绑定进 Ctx.env，跨行继承、不产三态行；
  `=` 只绑定、`==` 只判定；重名 `E-BIND-REDEFINED` 且保留旧值。纯函数式单卷。
- **一致性强度轴**：`con:` `equiconsistent:` `strength_le:` `interprets:`
  `translation:` `theory_sig:`。只登记相对一致性与解释请求，不判 `Con(T)`。
  `con: T -> T` 由结构护栏拦下（Gödel II 不是外挂 if）；`-> True` 只触发校验、
  不提供真值；无证书 → `UNDECIDED E-SCOPE-CONSISTENCY`，绝不并入 False。
- **势轴**：`card_le:` / `card_eq:`，证书为单射/双射；无限集或无证书 →
  `E-SCOPE-CARDINAL`。`class:` / `proper_class:` 登记真类，真类谈势 →
  `E-SCOPE-PROPER-CLASS`。注意 ZF/ZFC 里集合之间仍可比势，禁的是真类取势。
- **范畴论轴**：`cat: object: arrow: compose: identity: commute: functor: nat:
  preserves: limit: colimit: product: equalizer: pullback: adj: unit: counit:
  yoneda: topos: subobject: monad: algebra:`，`id:` 已被恒等式行占用，恒等态射
  只认 `identity:`。有限交换图能判就判（`commute: f != g` → `False
  E-CAT-NONCOMMUTE`），泛性质无证书 → `E-SCOPE-UNIVERSAL`。
- **CS 结构**：`carrier:` / `structure:` 登记有限结构；换世界层（逢 7 回头）
  留档不落，不跨结构搬真值。
- **试卷库**：`exams/{trivial,easy,normal,hard,very-hard,prize}/*.ltir` +
  `exams/keys/` + `exam.json` + `ltir_exam.sh`。题面头 `problem: / difficulty: /
  hint: / attribution: / exam:`；答卷单独一行 `submit:` 交卷，EOF 不算交卷。
  交卷前零回显；空卷/全错 0 分且 **exit 0**；漏 `submit:` exit 3；
  配置错 exit 2；输出模式冲突 exit 4。`human=y|n` 为君子协议，只进时间/难度
  基线分组、不进分数；`--no-clock` 固定 `elapsed_s=0` 供快照回归。
- **输出层契约**：stdout=题面+唯一回执，stderr=须知/人话；非 TTY 不弹交互；
  `json` / `ndjson` / `tri` / `default` 四模式；`\b` 进度条仅 TTY 且默认本地可见；
  全绿只给一行汇总，失败才展开能断案那一串。

## 0.1.0 A 增量 · 公理本体化（本节覆盖 0.1.0 条文中 `theory_sig:` 的纯登记语义）

立法点：一致性强度由 `lang` + **公理公式本体** + 推导规则给出，**不是名字给出**。
补丁不铸版本号：`VERSION` / `LTIR_STAMP` / `--version` / JSON `"version"` 仍 `0.1.0`；
`core/` 十一件与 `RUN_core_sha.txt` 一字不动（门禁 11/11）；短写表仍冻结。

- **`lang:`**（新增长名）：登记理论语言签名，如 `lang: L_N = {0, S, +, *}`；
  多次以 ` | ` 按声明顺序拼接；`load:` 模块内的 `lang:` 同样入册。登记行，不产三态。
- **`axiom:` 扩写**：`axiom: Sep = forall x (x in A and P(x))` 把公式本体入 Ctx；
  旧 `axiom: AC, Separation` 逐字保留（公式位空串亦入册）。`preset:` 内置表与
  `load:` 外部模块走同一解析，公理本体不在加载时丢失。
  公式内避免半角逗号（逗号仍是列表分隔符），可用中文逗号或分号。
- **`theory_sig:` 升级**：按 `lang + 全部 name=formula（字典序） + rules（字典序）`
  算 **FNV-1a 64** 内容指纹，16 位小写 hex；纯整数位运算、零浮点、跨平台一致，
  声明/加载顺序不影响结果。`theory_sig:` / `theory_sig: ?` 只算不比；给期望 hex
  （可带 `0x`）时比对，不符计入 `sig_mismatch`。**比对只登记**：不产三态行、
  不并 False、不报错、exit 0。人话档显示的始终是实算指纹，不照抄声明值（真值自产）。
- **JSON 结构恒定追加**：`axiom_bodies` / `lang` / `theory_sig` / `sig_mismatch`。
  `--tri` / `--no-answer` / `--json` 均不被指纹行污染（摘要仅 default 档出现）。
- **同名改一字公式 → 指纹变**：这是「不是符号给的」在机器侧的第一处落地。
  解释证书校验仍留 L2，本增量不实现，也不解锁任何判定核。
- **仍锁**：不判 `Con(T)` · 不构造公理推导判定核 · `translation:` 仍只登记 ·
  `strength_le:` / `interprets:` 仍 `E-SCOPE-CONSISTENCY` · 跨图不搬真值 ·
  码表零 append（比对不符只进 `sig_mismatch` 计数，不新增错误码）。
- SMOKE `405 / 405`；三角囚徒冻结卷 `10 True`、demo `24 lines: 20/3/1` 逐字不漂。
  UI 层按收尾行自适应总数，推版加断言不用改输出层。

**示范卷**：`fixtures/theory_sig.ltir`。

## 0.1.1 不定式专码 · E-INDETERMINATE（本节覆盖 `V: 0/0` 的归码）

立法点：`0/0` 不是「超越域」，也不是 `∞` 未定形。它是**除法定义 + 域逆元公理 +
零吸收律 + 路径依赖**四条规则叠加后商无唯一值。码表 append 一行
`E-INDETERMINATE`（state 3），`0/0` 不再蹭 `E-SCOPE-TRANSCENDENTAL`。

- **触发**：`divv` 在 `isZero(y) && isZero(x)` 时返回带原因标记的 UND；
  标记经 `add` / `mul` / `divv` / `neg` / `power` / 比较原样传播，故
  `0/0`、`(1-1)/(1-1)`、`0/0+1`、`0/0 == 0/0` 同归此码。
  `1/0` 仍登记 `∞`；`∞−∞` / `0·∞` 仍 `E-INF`，三家互不串台。
- **三态纪律**：一律 `UNDECIDED`，不赋 True、不并 False、不并 ∞。
  `NaN` 只是人话兜底词，**不是引擎真值**；不引入浮点、不引入随机数、
  不引入交互。彩蛋 A/B 抽奖按裁决**整族不做**。
- **展示**：默认档在 `Lxx UNDECIDED  E-INDETERMINATE` 下补一行 `∴`；
  `--explain` 摊开完整 `∵ N / R1 / R2 / R3 / R4 → ∴` 因果链，中英同构。
  `--tri` 只吐三态词，`--json` 追加 `indeterminate_form` + `rules` 结构字段，
  形式化展示均不污染数据通道（UI 信息全走人话档，stdout 仍归数据与回执）。
- **定理卷**：`fixtures/indeterminate.ltir` 只登记四条公理本体、`theory_sig:`
  与 `∵∴` 链，`verify` 输出 `0 lines: 0 True, 0 False, 0 UNDECIDED`。
  它不替行层判 `0/0`，不搬真值；**不含交互、不含抽奖、不含计时**。
  本卷由本节 `ref:` 收录，条文本身仍是「只登记不判」。
- **`by:` 登记**：顶层证书行收进关键词表，只 `registered++`、不产三态；
  keys 卷裸 `verify` 不再 `E-UNK-LINE`。`--tri` 遇 `state=3` 的 ERROR 直接跳过，
  不把系统错冒充 `UNDECIDED`（Agent 管道纪律）。
- **`define:` 参数名**：`define: g(t) = ...` 的真实参数名接入
  `call()` / `primary()` 导数分支 / `derive:` 三处消费端（缺省回退 `x`，
  旧卷零回归）。`V: g(3)` → 10、`V: 2g(3)` → 20、`derive: g'(t)` → `2*t`。
- **仍锁**：`core/` 十一件与 `RUN_core_sha.txt` 一字不动；不新增判定核、
  不开 `sqrt(-1)` / 复数比较、不重写测试事件消费层、不推进版本。
- SMOKE `445 / 445`；demo `24 lines: 20/3/1`、三角囚徒 `10 True` 逐字不漂。


## 0.2.0 元编程词条（只登记，不判）

- 新增长名六词：`gap:` / `reformulate:` / `quote:` / `reflect:` / `depth:` / `trace:`。
  全部 `registered++` 后返回 `state=-1`，**0 三态**；短写表仍冻结。
- `depth:` 给 `expand:` 的有限步上界（缺省 8，范围 1–64）；`trace: on` 把逐步展开
  写 stderr，不进 stdout。`quote:` 只把一段 `.ltir` 当展示型数据登记，
  **不 eval**；`reflect:` 是只读镜，摘要进 ledger 与 `--json` 的 `meta_*` 字段。
- `gap:` 同 id 两条 / `depth:` 越界 / `reflect:` 未知目标，复用 `E-DEF-SIG`，
  绝不静默择一。`reformulate:` 本期不自动接管 UNDECIDED 分支。
- **永久锁**：自举执行自产 `.ltir`、运行时写回码表、`grammar:` 生成解析器、
  元层产 True、自动跨图搬真值。`core/` 十一件与 `RUN_core_sha.txt` 一字不动。
- 定理卷与历史卷：`ref:` 指向 `fixtures/indeterminate.ltir` 与
  `fixtures/history_gaps.ltir`；两卷均 `0 lines: 0 True, 0 False, 0 UNDECIDED`。
- 仍锁：`core/` 十一件；`A.p==0` 一次不等式、`sqrt(-1)`、复数比较留 BACKLOG；
  码表未 append、不推进版本。


## 授权收尾：四个独立码与 Gödel II 定理卷

- `lab_codes.tsv` append-only 四行：`E-GODEL-II` / `E-STRENGTH-NOCERT` /
  `E-STRENGTH-INCOMPARABLE` / `E-STRENGTH-CYCLE`。历史行一字未动。
- `con: T -> T` 由通用 `E-SCOPE-CONSISTENCY` 改归 `E-GODEL-II`；
  `strength_le:` / `interprets:` 改归 `E-STRENGTH-NOCERT`。
  两者仍 `UNDECIDED`，**不并 False、不判 `Con(T)`、不判不一致**。
- `E-STRENGTH-INCOMPARABLE` / `E-STRENGTH-CYCLE` 本期**只立法、不挂逻辑**：
  偏序 DAG 与证书边尚未实现，码位先立好，条文明写「预留」。
- 定理卷 `fixtures/godel_ii.ltir`：Hilbert–Bernays 三条件 + `Con_T` 定义 +
  ∵∴ 链，`verify` 输出 `0 lines: 0 True, 0 False, 0 UNDECIDED`。
  卷内不写 `con: T -> T`（那会产一行弃权），只以 `hint:` 登记。
- 元编程三个候选码**裁决不 append**，冲突一律复用 `E-DEF-SIG`。
- 仍锁：`core/` 十一件与 `RUN_core_sha.txt`；自举永久不做；
  不推进 `VERSION` / `LTIR_STAMP`，仍 `0.1.1`。
