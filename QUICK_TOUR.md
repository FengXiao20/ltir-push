# LTIR 速览 · 0.0.6 **强化修复版**（高中版 P1 地基）

> 给「打开这个文件夹一脸茫然的人」的一页纸。**新关键词全部写在下面第二节，逐个带用法和结果。**
> 平台：Linux + Termux，纯 C++，Windows 分支冻结。
>
> 主版本号仍是 0.0.6。这一版补了四件事：**修掉 L06 代入 bug / 标签体系整套接回 /
> 领域扩到 24 个 / Termux clang++ 真机取证**。原 44 条回归零破坏，现 **69/69**。
> 标签与领域写法见**第二之二节**，别再问「以前那种标签式设计是不是扔了」——没扔，接回来了。

## 一、这是个啥

**说真话的判定语言**：写数学断言，引擎逐行回 `True / False / UNDECIDED`。
不知道就弃权，绝不硬猜、绝不把 UNDECIDED 并进 False。全程精确 `int64` 有理数，
零浮点；`-> True` 只是声明词，**真值由引擎自己算**，不信外部标注。

`core/` 十一件 ℚ 域内核一字不动（SHA 门禁常驻）；`src/` 是 0.0.6 新语言层。
不搞 MCP，接口就是语言本身。

## 二、关键词速查（0.0.6 新增用法，逐个写清）

**规矩**：关键字全是英文，能简写就简写，但**不许简写成裸单字母**。
`A:` `V:` `id:` 这种带冒号合法；行首孤零零一个 `X` 直接 `E-UNK-LINE`。
`i e π x f R` 这些单字符是**标识符/常量**，不是关键字，随便用（见第三节）。

| 关键词 | 短写 | 怎么写 | 出什么 |
|---|---|---|---|
| `assert:` | `A:` | `A: 2+2=5 -> True` | `False  E-VAL-MISMATCH`（实算 4≠5，不信声明） |
| `assert:` | `A:` | `A: 3^3 == 27 -> True。` | `True`（中文句号截断，不拒收） |
| `value:` | `V:` | `V: 1/3+1/6` | `1/2` |
| `value:` | `V:` | `V: √(16) + π^0` | `5`（^0 先折叠，π 不误杀） |
| `value:` | `V:` | `V: sin(2*pi/3)` | `(1/2)√3` |
| `value:` | `V:` | `V: (1+i)^2` | `2i` |
| `value:` | `V:` | `V: 1/0` | `∞`（`oo-oo` 才 UNDECIDED） |
| `at:` | `at:` | `at: x=3, y=2/3` | 代入环境，作用到下一个 `at:`（空 `at:` 清空） |
| `ident:` | `id:` | `id: (x+y)^2 == x^2+y^2` | `False`（差式 `2*x*y ≠ 0`） |
| `numth:` | `N:` | `N: factors(360)` | `2^3 × 3^2 × 5` |
| `numth:` | `N:` | `N: isprime(2^61-1)` | `True`（Miller-Rabin，不试除到天亮） |
| `define:` | — | `define: f(x) = x^2 - 3x + 2` | 登记函数 |
| `derive:` | — | `derive: f'(x)` | `2*x - 3` |
| `domain:` | — | `domain: f` | `R` |
| `solve:` | — | `solve: x^2-5x+6 < 0` | `x ∈ (2, 3)` |
| `seq:` | — | `seq: a_n = 2n+1` | 登记数列 |
| `sum:` | — | `sum: n=1..10 of a_n` | `120`（∑ 的 ASCII 写法） |
| `limit:` | — | `limit: n->inf of 1/n` | `0` |
| `vector:` | — | `vector: cross([1,0,0],[0,2,0])` | `(0, 0, 2)` |
| `prob:` | — | `prob: C(5,2)*(1/2)^5` | `5/16` |
| `stat:` | — | `stat: mean([1,2,3,4])` | `5/2` |
| `set:` | — | `set: A = {1,2,3}` | `{1, 2, 3}` |
| `const:` | — | `const: phi = (1+sqrt(5))/2` | 注册新符号，之后 `V: phi^2-phi` → `1` |
| `given:` | — | `given: a>0, b>0` | 登记条件（只记不判） |
| `case:` | — | `case: x>=0 -> ...` | 分支登记（只记不判） |
| `check:` | — | `check: x=1 in sqrt(x+3)=x-3` | 人的验根，**不产真值** |
| `note:` | `#` | `# 这题分类讨论` | 跳过，原样保留 |
| `subject:` | `<s>` | `<subject domain="math"> ... </subject>` | 题目标签容器 |
| `zf:` / `zfc:` | — | `zf: Riemann: ζ(s)=0 → Re(s)=1/2` | **只登记不判**（保留项，暂时别动） |

**容器**：`<a> 解：设 x=1，则 sin(pi/6)=1/2 -> True </a>` → `True`。
容器内中文人话免标记，中文片段当分隔符，**全部**算式段交解析器（多段状态机，不再只挑一段）。
`解：设 x=5，代入得 A: x^2 = 25 -> True。` → `True` —— 「设 x=5」被切成一段绑定环境，
这是强化修复版修掉的 L06（首版报 UNDECIDED）。`裸 C:` 仍封死。

## 二之二、标签式设计（乙裁决接回，逐个写清）

首版把标签降成一个布尔开关、还在立法文本里写了「明确不用 `S: E: P: Q:`」—— 那是
施工侧越界，本版整套并回，**不是替换**：原来的 `A:` / `V:` / `<a>` 一个没动。

| 标签 / 简写 | 怎么写 | 出什么 |
|---|---|---|
| `<calc>` / `<a>` | `<calc>\nA: 2+2=4 -> True\n</calc>` | `L01 True`（演算容器） |
| `<过程>` / `<proc>` | `<过程> A: 2+2=4 -> True </过程>` | 同上，输入层归一到 `<calc>` |
| `<answer>` | `<answer>A: 3^3 == 27 -> True</answer>` | `L01 True`，`--json` 里 `tag:"answer"` |
| `<C_and_E>` | `<C_and_E>\ncause: 2x+6=14\neffect: x=4\nby: calc\n</C_and_E>` | **不产三态行**，`--json` 里 `inferences:1` |
| `S:` 解 / `E:` 设 / `P:` 证明 / `Q:` 证毕 | `S: 解：先移项` | 整行机器跳过、原样保留，不判不对错 |
| `V:` 验 | `V: 1/3+1/6 = 1/2` | **仍求值**（0.0.6 语义），只额外登记装饰身份 |
| `<subject domain="algebra">` | 包住题目 | `domain` 进上下文，`--json` 可查 |

`cause:` / `effect:` / `by:` 是**全拼小写字段**，`E:` 是简写行，靠大小写分界互不越权；
`by:` 只登记凭据（`calc` / `ref` / 既有答案），不替引擎判真假。

## 二之三、领域表（扩增到 24 个）

```
math  algebra  equation  inequality  polynomial  function  sequence
limit-calculus  trigonometry  vector  complex  geometry  solid-geometry
analytic  combinatorics  probability  statistics  number-theory  set
logic  induction  linear-algebra  zfc  physics-math
```

| 写法 | 出什么 |
|---|---|
| `<subject domain="trigonometry"> ... </subject>` | `--json` 里 `"domain":"trigonometry"` |
| `domain: trigonometry` | 顶层显式设定当前领域 |
| `domain: ?` | `24 domains registered` |
| `domain: f` | `R`（查函数定义域，首版行为未回归） |
| `domain: 冷门新领域` | 原样登记、不报错不拦截不解释 |

领域表是 `domains.tsv`（语法层资产，不是错误码表），扩增不用改 `lab_codes.tsv`。
领域只登记可查、**不改判定逻辑** —— 域能差不靠转换硬抹平。

## 三、单字母特例：不止 i / e / π

`i e π φ τ ∞ γ`、`N Z Q R C`、`x y z t n k m`、`a b c d`、`f g h`、`A B C P Q L`
全合法 —— 它们是**表达式里的标识符**，不是行首关键字。
清单不写死：新符号走 `const:` 登记一次就能用，天然开放。

## 四、三态与退出码

| 结果 | 含义 | 退出码 |
|---|---|---|
| `True` | 实算与声明一致 | 0 |
| `False` | 实算不符，附码表解释 | 0 |
| `UNDECIDED` | 越界 / 溢出 / 未定形，诚实弃权 | 0 |
| `ERROR` | 语法硬错（裸单字母等） | **1** |

登记行不进三态统计，只在汇总后的 `registered:` 计数里出现。

## 五、怎么跑

```bash
bash ltir.sh                 # 一键：门禁 → 编译 → SMOKE 69/69 → 演示
./ltir006 eval "1/3+1/6"        # 单表达式
./ltir006 verify fixtures/demo.ltir            # 简明档
./ltir006 verify fixtures/demo.ltir --explain  # 详细档（^ 指位 + 码表解释）
./ltir006 verify 卷.ltir --json               # 结构化输出（含 domain/tag/inferences/...）
./ltir006 verify 卷.ltir --lang=en            # 强制英语（默认按卷内中文自动跟随）
./ltir006 --version                           # ltir006 0.0.6 强化修复版
```

`--json` 现在多带 `domain / domains_registered / domains_seen / decorations /
preserved / inferences / answers`，每行带 `tag`；简明档末尾只在显式用过标签或领域时
补一行 `tags:` / `domain:`，原有逐行输出一字不变。

**Termux**：`pkg install clang make`；项目别放 `/storage/emulated/0`（noexec），
脚本会自动搬家到 `~/ltir_lab`。Termux 无 g++，脚本自动切 `clang++`。

## 六、谱系（防串台）

| 版本 | 状态 | 一句话 |
|---|---|---|
| 0.0.1–0.0.2 | 冻结 | ℚ 域四则内核 + 五头六 cpp，SHA `aecc9726…` |
| 0.0.4-lab | 历史 | C++ 能力扩展：变量/幂/数论/∞/ZF 台账 |
| 0.0.5 | 历史 | 行短写 / 容器+人话 / LaTeX / 三角 / ZFC 台账 |
| 0.0.6 强化修复版 | 历史 | L06 修复 / 标签并回 / 24 领域 / Termux clang++ 取证 / 69 条 SMOKE |
| 0.0.7 / 0.0.8 / 0.0.9 | 历史 | 集合论地图 / 形式化定义与多宇宙 / 隐式乘法与输出模式（347 SMOKE） |
| **0.1.0** | **本版（正式版）** | 试卷库 / 进制眼镜 / 赋值环境 / 一致性强度台账 / 387 SMOKE |

旧 JS 文本层 / Python 验真 / MCP 嘴已按删除令退场，销案不回退。
立法文本见 `SYNTAX.md`，实跑记录见 `RUNRESULT.md`，错误解释由 `lab_codes.tsv` 驱动。

## 七、未尽（只列不做）

P2 主体（分段函数 / 参数方程 / 解三角形 / 立体几何 / 线性回归 / 分布列）、
P3 边界（渲染标黄 / 浏览器宿主 / 中文术语词典）、Termux 真机 clang++ 回填、
AST 级复合 LaTeX 折叠、i18n、Windows 分支。

## 0.0.7 新关键词速查（全英文长名 · 短写表冻结 · 每个带可跑示例）

| 关键词 | 写法 | 结果 / 说明 |
|---|---|---|
| `system:` | `system: ZFC` | 声明地图坐标；`⊢`/大基数/V 必须先有它 |
| `choice:` | `choice: on` | `on/off/dc/cc/bpi`；AC 图层开关，只决定能谈哪条定理 |
| `axiom:` | `axiom: AC, Separation` | 本图公理表；`by:` 查的就是它 |
| `ref:` | `ref: Godel1940` | 外部证书登记，不参与实算 |
| `module:` | `module: ZFC` | 模块首行身份；模块内**不许**写 `A:` |
| `preset:` | `preset: ZFC` | 内置 15 张地图按名加载，幂等，跨端零路径分歧 |
| `load:` | `load: axioms/my_axioms.ltir` | 加载自己加的 `.ltir`；只登记不产三态行 |
| `universe:` | `universe: V` | 全局 V（真类）；限集合论域 + 显式 `system:` |
| `univ:` | `univ: U = {1,2,...}` | 有限/相对全集，≠ V |
| `card:` | `card: A -> 2` | 有限势精确；无限 → `E-SCOPE-INFINITE` |
| `forall:` | `forall: t in A: t <= 10` | 有限集逐元素可判；连续域 → 弃权 |
| `under:` | `under: x>=0, x+2y+3z<=6` | 约束域声明，先只登记不参与判定 |
| `claim:` | `claim: xyz... <= 27` | 全称主断言 → `E-SCOPE-QUANTIFIER` |
| `cert:` | `cert: todo` | 证书位，留 P2 区间证书，不产三态行 |
| `witness:` | `witness: t1=...` | 取等点/反例登记，不复核数学 |
| `problem:` | `problem: CH` | 千禧年/独立命题台账，不判真假 |
| `status:` | `status: independent-of ZFC` | 台账状态字段 |
| `theory:` / `inner:` | `theory: ModelTheory` | 模型论/内模型区域名 |
| `map:` / `independent:` | `map: UltL` | 地图名与独立性登记 |
| `model:` / `cite:` | `model: L` | 模型 / 文献引用登记 |
| `paradox:` | `paradox: Russell` | → `E-PARADOX-RUSSELL`，塌图登记不判 |
| `absolute:` | `absolute: Omega` | → `E-SCOPE-ABSOLUTE`，绝对无限不可比势 |
| `by:`（升格） | `A: 2+2==4 -> True by: AC` | 行尾引用不再被吞；查公理表，不作免算金牌 |

**运算符（入口归一，非关键字）**：`⊢`(`\vdash`) 相对可推导 · `⊨`(`\models`) 语义蕴涵 ·
`∈ ∉ ∪ ∩ ∖ ⊆ ⊇ ⊂ ⊃` 集合算子 · `¬`(`\neg`) 否定 · `∅ Ω ω` ·
`…`(U+2026) / `\dots` / `\ldots` / `\cdots` → `...`。
`\subset` 归 `subseteq`（子集），`⊂` 仍是真子集，两者不合并。

**防「沙」彩蛋**：`set: N = {0,1,2,......}` → 折叠成 `{0,1,2,...}` 并吐槽
「你多打了省略号，已经补全了。才不是帮你呢。」；`--lang=en` 给英译，
末句是彩蛋，`--json` 里看 `easter_egg`。单个 `...` 不触发，`1..10` 不误伤。

**怎么跑（Linux / Termux，Win10+ 包本轮不重打）**
```bash
cd ~/ltir_lab
bash ltir.sh          # SHA 门禁 → 编译 → SMOKE → 演示
bash ltir.sh b        # 只编译（Termux 自动 clang++）
bash ltir.sh t        # 只跑 0.0.7 SMOKE
./ltir006 verify 卷.ltir --explain
./ltir006 --version      # ltir006 0.0.7
```
**「起来」协议**：中途挂了，发「起来」即接着干，已记进 README。

## 0.0.7 补丁 · 甲（仅台账域）

**一个判定核都没加。** 新增 28 个域只是「地图上有名字」，判定请求一律诚实弃权。

```
domain: analysis
A: 2+2==4 -> True        → UNDECIDED  E-SCOPE-LEDGER-ONLY
system: ZF                → 照常登记，不产三态行
```

- 域表 **55** 个注册；`domains.tsv` 里 scope 以 `[台账]` 结尾的 28 个是仅台账域
- 老 27 域（含 `inequality` / `trigonometry`）行为**逐字不变**，三角囚徒冻结卷仍 10/10
- 未知领域仍「标黄放行」：原样登记、不拦截、不报错
- 码表 append 一行，引擎只读消费；**严禁并入 False**、不是硬错、exit 0

### 新域清单（28）
| 族 | 域 |
|---|---|
| 分析 8 | analysis / real-analysis / measure / functional-analysis / complex-analysis / differential-equation / numerical-analysis / dynamical-systems |
| 代数·几何·拓扑 10 | abstract-algebra / galois-theory / algebraic-geometry / graph-theory / combinatorial-optimization / topology / algebraic-topology / differential-geometry / category-theory / analytic-number-theory |
| 概率·应用 10 | measure-probability / stochastic-process / statistical-inference / information-theory / game-theory / convex-optimization / operations-research / computational-complexity / financial-mathematics / ergodic-theory |

### 修好的写法（0.0.7 首版会静默伪判）
```
V: {1,2} ∩ {2,3}        → {2}           （旧：True 1）
V: 2 ∈ ∅                → False          （旧：UNDECIDED 超越域）
card: {1,2,3}           → 3              （旧：E-REF-UNKNOWN）
card: ∅                 → 0
A: card(∅) == 0 -> True → True
A: card({{1},{2}}) == 2 -> True → UNDECIDED E-SCOPE-NESTED
<C_and_E> ... by: calc  → 计入 by_refs   （旧：静默漏记）
V: 2 ∈ ∅ -> False       → L01 True       （声明词先剥，且不残留错误码）
```
`V:` 短写仍是求值短写，只是集合触发词命中时改走有限集语义；纯数值行逐字不变。


## 0.0.7 补丁二 · 新语法速查（版本仍是 0.0.7）

| 写法 | 结果 |
|---|---|
| `V: 2 ∈ ∅` | `False`，**无码**（旧：误挂 E-VAL-MISMATCH） |
| `forall: x in {1,2,3}: x^2 > 0` | `True`（旧：E-SCOPE-INFINITE） |
| `forall: x in R: x^2 > 0` | `UNDECIDED E-SCOPE-INFINITE` |
| `A: ∀ x ∈ {1,2,3}, x^2 > 0 -> True` | `True` |
| `A: ∃ x ∈ {1,2,3}, x == 2 -> True` | `True` |
| `set: R = {x ∈ {1,2,3} \| x > 1}` | `True  {2, 3}` |
| `set: X = {x \| x ∉ x}` | `UNDECIDED E-PARADOX-RUSSELL` |
| `V: X ∈ X` | `UNDECIDED E-PARADOX-RUSSELL`（不判真假） |
| `A: x > 0 -> True` + `cex: x = -1` | 第二行 `True  反例有效` |
| `mapping: f = {1->2, 2->3, 3->4} on {2,3,4}` | 只登记，不产三态行 |
| `inj: f` / `surj: f` / `bij: f` | `True` / `True` / `True` |
| `mapping: h = R -> R` | `UNDECIDED E-SCOPE-INFINITE` |

码表 append 两行：`E-CEX-INVALID`（False）、`E-SCOPE-CODOMAIN`（UNDECIDED）。
`-> True/False` 仍是声明词，真值自产不变。

## 0.0.8 速览增补

| 写法 | 结果 |
|---|---|
| `bash ltir.sh v` | 读 `VERSION` → `0.0.8` + `--version` |
| `sig: f : Num -> Num` | 签名登记，不产三态行 |
| `define: sq(x : Num) : Num = x*x` | 带签名定义，进 δ 展开表 |
| `expand: sq(3)` | `True  9`（有限步 δ 展开） |
| `rule: ModusPonens` | 推导规则登记，进 `by:` 证书链 |
| `multiverse: on` / `world: W0 = ZFC` | 图册开关 / 世界登记 |
| `force: W0 -> W1 by: Cohen1963` | 可达边登记，不产三态行 |
| `reachable: CH in W1` | `E-SCOPE-CROSS-WORLD`（不跨图搬真值） |
| `A: □ CH -> True` | `E-SCOPE-MODAL` |
| `A: PA ⊢ Goldbach -> True` | `E-SCOPE-OPEN`（open 非独立） |

## 0.0.9 速览增补

| 写法 | 结果 |
|---|---|
| `id: (x+y)^2 == x^2 + 2xy + y^2` | `True`（隐式乘法） |
| `set: T = {x \| x in {1,2,3,4} and x > 2}` | `True  {3, 4}` |
| `set: W = {x \| x in R and x > 0}` | `UNDECIDED  E-SCOPE-INFINITE` |
| `V: 100!` / `V: 2^64` | `UNDECIDED  E-OVF` |
| `V: sin(pi/12)` | `True  (√6 - √2)/4` |
| `V: cos(pi/12)` | `True  (√6 + √2)/4` |
| `V: tan(pi/12)` | `True  2 - √3` |
| `A: sin(pi/12) == cos(pi/12) -> True` | `UNDECIDED  E-SCOPE-ALGEBRAIC` |
| `./ltir006 verify 卷.ltir --tri` | 每行只有 `True` / `False` / `UNDECIDED` |
| `./ltir006 check 卷.ltir` | 三态与码照出，答案值不输出 |
| `bash ltir.sh tri` | 打印 `export LTIR_OUTPUT=tri` |
| `bash ltir.sh u` / `./ltir006 uninstall` | 卸载指引与脚本 |

---

# LTIR Quick Tour (English)

> One page for "opened this folder and have no idea what it is". Same content as the
> Chinese original above; this English edition is the translation of record.

## 1. What is this

**A verdict language that tells the truth.** You write math assertions; the engine
answers line by line `True / False / UNDECIDED`. When it doesn't know, it abstains —
it never guesses, and never folds UNDECIDED into False. Everything is exact `int64`
rationals, zero floats; `-> True` is only a declaration word — **the engine computes
the truth itself** and does not trust external labels.

`core/` is the frozen eleven-file ℚ-domain kernel (SHA-gated); `src/` is the 0.0.6+
language layer. No MCP — the interface is the language itself.

## 2. Keyword cheat sheet

**Rules**: keywords are English, abbreviated where possible, but never abbreviated to
a bare single letter. `A:` / `V:` / `id:` (with colon) are legal; a lone `X` at line
start is `E-UNK-LINE`. Single chars like `i e π x f R` are identifiers/constants, not
keywords.

| Keyword | Short | How to write | What comes out |
|---|---|---|---|
| `assert:` | `A:` | `A: 2+2=5 -> True` | `False E-VAL-MISMATCH` (engine got 4≠5) |
| `assert:` | `A:` | `A: 3^3 == 27 -> True。` | `True` (Chinese period truncated, not rejected) |
| `value:` | `V:` | `V: 1/3+1/6` | `1/2` |
| `value:` | `V:` | `V: sin(2*pi/3)` | `(1/2)√3` |
| `value:` | `V:` | `V: 1/0` | `∞` (only `oo-oo` is UNDECIDED) |
| `at:` | `at:` | `at: x=3, y=2/3` | substitution scope until next `at:` |
| `ident:` | `id:` | `id: (x+y)^2 == x^2+y^2` | `False` (residual 2xy ≠ 0) |
| `numth:` | `N:` | `N: isprime(2^61-1)` | `True` (Miller–Rabin) |
| `define:` / `derive:` | — | `derive: f'(x)` | `2*x - 3` |
| `solve:` | — | `solve: x^2-5x+6 < 0` | `x ∈ (2, 3)` |
| `sum:` / `seq:` / `limit:` | — | `sum: n=1..10 of a_n` | `120` / registered / `0` |
| `vector:` / `prob:` / `stat:` | — | see Chinese table | exact results |
| `set:` | — | `set: A = {1,2,3}` | `{1, 2, 3}` |
| `const:` | — | `const: phi = (1+sqrt(5))/2` | new symbol; then `V: phi^2-phi` → `1` |
| `given:` / `case:` / `check:` | — | bookkeeping | registered, **no verdicts** |
| `note:` | `#` | comment, kept verbatim | skipped |
| `subject:` | `<s>` | `<subject domain="math">…</subject>` | problem container |
| `zf:` / `zfc:` | — | ledger only, **register never judge** | `registered` |

**Containers**: `<a> 解：设 x=1，则 sin(pi/6)=1/2 -> True </a>` → `True`. Chinese prose
inside is free-form; every ASCII math segment goes to the parser (multi-segment state
machine). `设 x=5` binds the environment; `<裸 C:>` stays sealed.

## 3. Tags (restored by ruling, not replaced)

| Tag | Meaning | Behavior |
|---|---|---|
| `<calc>` / `<a>` | computation | extracts math segments |
| `<C_and_E>` | reasoning | `cause:`/`effect:`/`by:` fields; no verdict lines |
| `<answer>` | answer | archived form of `A:`; `tag:"answer"` in `--json` |
| `<subject domain="…">` | problem | domain enters context, queryable |
| `S:` `E:` `P:` `Q:` | decorations | whole-line, machine-skipped, kept verbatim |

## 4. Three states & exit codes

| Verdict | Meaning | Exit |
|---|---|---|
| `True` | computation matches the declaration | 0 |
| `False` | it doesn't, with reason code | 0 |
| `UNDECIDED` | out of scope / overflow / undetermined — honest abstention | 0 |
| `ERROR` | hard syntax error (bare single-letter keyword etc.) | **1** |

## 5. Set-theory layer (0.0.7+, condensed)

`system:` `choice:` `axiom:` `ref:` `module:` `preset:` `load:` `universe:` `univ:`
`card:` `forall:` `exists:` `cex:` `inj:` `surj:` `bij:` `mapping:` `under:` `claim:`
`paradox:` — map declarations and finite-set quantifiers. `⊢` = relatively derivable
on the loaded map, never absolute truth. Quantifiers over infinite/constructed domains
→ `UNDECIDED E-SCOPE-INFINITE`. `{x | x ∉ x}` → `E-PARADOX-RUSSELL` (map collapse,
registered, never judged). Extra ellipses collapse to one `...` + tsundere easter egg.

## 6. How to run

```bash
bash ltir.sh          # SHA gate -> build -> SMOKE (0.1.0) -> demo
bash ltir.sh b / t    # build only / SMOKE only (Termux auto-switches to clang++)
./ltir006 verify 卷.ltir --explain / --json / --tri / --lang=en
./ltir006 --version   # ltir006 0.1.0
```

**Termux**: `pkg install clang make`; keep off `/storage/emulated/0` (noexec) —
the script relocates to `~/ltir_lab` automatically.

## 7. Lineage & boundaries

Frozen ℚ kernel (0.0.1–2) → capability extension (0.0.4-lab) → shorthand/containers/
LaTeX/trig/ZFC ledger (0.0.5) → self-contained C++ layer (0.0.6 reinforced, 69 SMOKE)
→ set-theory map + quantifiers (0.0.7, 136→225 SMOKE) → formal defs + multiverse
(0.0.8) → implicit product, π/12 exact angles, `--tri` output modes (0.0.9, 347 SMOKE)
→ exam library, radix glasses, binding env, strength ledger (0.1.0, 387 SMOKE).

Still not doing (listed, not forgotten): piecewise functions, parametric/polar,
triangle solving, solid geometry, linear regression, distribution tables, highlighted
rendering, browser host, i18n dictionary, Windows repack. Every phase only lists what
it does NOT do, awaiting new orders. Full legislation: `SYNTAX.md`; run records:
`RUNRESULT.md`; error explanations driven by `lab_codes.tsv`.
