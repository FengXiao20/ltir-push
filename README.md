# LTIR · 0.0.9b

> 当前版：0.0.9b ｜ SMOKE **347/347 全绿** ｜ 0.0.9 + PATCH 009b（`^` 只绑隐乘末变量，`xy^2` = `x·y^2`）


**LTIR — 一台不会装懂的验算机**

它只判三种状态：`True` / `False` / `UNDECIDED`。

你写 `2+2==5 -> True`，它照样回 `False` —— **声明不算证据，真值得自己算**。
算不了就老实弃权：不把「不知道」凑成否，不拿一堆采样点冒充全称证明，
不替数学界给 CH 或哥德巴赫拍板。全程精确有理数与代数数，零浮点。

再往上，集合论被做成一张地图：ZFC、朴素集合论、大基数、多宇宙图册，
用 `⊢` 标「我在哪张图」，用 `preset:` / `load:` 加载公理模块。
`0.0.9` 起支持形式化定义（`sig:` / `rule:` / 有限步 δ 展开）、
受限概括、`∀/∃`、反例登记册 `cex:` 与有限集精确运算。

最可爱的是多打了省略号，它会默默补全，然后甩一句「才不是帮你呢。」

C++17 单机、纯标准库，Linux / Termux 可跑；`bash ltir.sh` 一键
门禁 → 编译 → 自测 → 演示，当前自测 **全绿**。

```bash
bash ltir.sh                 # 一键全流程
./ltir006 verify 卷.ltir     # 判卷
./ltir006 check 卷.ltir      # 只验算，不输出答案
./ltir006 verify 卷.ltir --tri   # 只输出三态词
```

---


---

## 版本档案

- **0.0.9b（当前版）** = 0.0.9 + PATCH 009b：修复隐乘组幂绑定——`xy^2` 曾被解析为 `(xy)^2`，致真恒等式伪判 False（违反"不伪判 False"硬令）。347/347 回归零破坏。详见 `PATCH_009b.md`。

<details>
<summary><b>📜 历史留档（0.0.6 强化修复版 全文）</b></summary>

# LTIR · 0.0.9b

> 当前版： 0.0.9b ｜ SMOKE **347/347 全绿** ｜ 0.0.9 + PATCH 009b（^ 只绑隐乘末变量，xy² = x·y²）



---

## 版本档案

- **0.0.9b** = 0.0.9 + PATCH 009b：修复隐乘组幂绑定（ 曾解析为 ，致真恒等式伪判 False）。详见 `PATCH_009b.md`。

<details>
<summary><b>📜 历史留档（0.0.6 强化修复版 全文）</b></summary>

# ltir006 · 0.0.6 **强化修复版**

**一本高中数学体量的自包含判定语言（P1 地基）。** 纯 C++、单二进制、零第三方依赖。
人写文本 → `./ltir006 verify 卷.ltir` → 逐行三态 `True / False / UNDECIDED`。
不搞 MCP，接口就是语言本身。

> 主版本号仍是 `0.0.6`，本版是**强化修复版**：修掉 L06 代入 bug、
> 把解题文本层四件套标签整套接回、扩增领域表、回填 Termux clang++ 真机取证。
> 原 44 条 SMOKE 回归零破坏（第 44 条仅随版本后缀更新期望值），现 **69/69**。

## 标签体系（乙裁决并回，不是替换）

| 标签 | 职责 | 现状 |
|---|---|---|
| `<calc>` / `<a>` | 演算容器 | 算式段照抽；`<过程>`/`<proc>` 输入层归一到 `<calc>` |
| `<C_and_E>` | 推理容器 | `cause:` / `effect:` / `by:` 三字段登记，**不产三态行** |
| `<answer>` | 答卷容器 | `A:` 归一化的存档态；`--json` 里 `tag:"answer"` |
| `<subject domain="…">` / `<s>` | 题目标签 | `domain` 真的进 Ctx，不再读完 `>` 就丢 |

装饰简写 `S:` 解 / `E:` 设 / `V:` 验 / `P:` 证明 / `Q:` 证毕 **整行机器跳过、原样保留**；
中文装饰词（解：设：答：）容器内免标记。字段全拼小写（`effect:`）与简写行（`E:`）靠
大小写分界，裸 `C:` 仍封死。`V:` 在 0.0.6 里**仍是 `value:` 求值短写**（回归零破坏），
只额外登记装饰身份。

容器内「设 / 令 / 取 `x=5`」→ 状态机切成多段后绑定环境，容器闭合时恢复，
不跨容器泄漏 —— 这就是截图里 L06 从 `UNDECIDED` 修成 `True` 的那一刀。

## 领域表（`domains.tsv`，本令扩增 24 个）

`math / algebra / equation / inequality / polynomial / function / sequence /
limit-calculus / trigonometry / vector / complex / geometry / solid-geometry /
analytic / combinatorics / probability / statistics / number-theory / set / logic /
induction / linear-algebra / zfc / physics-math`

```bash
<subject domain="algebra"> ... </subject>   # 设定当前领域
domain: trigonometry                        # 顶层显式设定
domain: ?                                   # 列出已注册领域数
domain: f                                   # 查函数定义域（首版行为，未回归）
```

未知领域按「域外记号标黄放行」精神：**原样登记、不拦截不报错不解释**。
领域表是**语法层资产，不是错误码表**，扩增不需改 `lab_codes.tsv`（append 权在立法者）。

## 架构

```
core/        11 件 ℚ 域内核  ← 一字不动，SHA 门禁常驻（libc 地位）
src/         0.0.6 新语言层  ← 关键词、入口防护、精确数值、三态、码表驱动
lab_codes.tsv 错误码表       ← 只读消费，append 权在立法者
```

内核复用方式：只 `#include "ltir/rational.hpp"` 并链接 `core/src/rational.cpp`，
沿用其 `int64` checked 原语；**五头六 cpp 一个字节都没改**。

## 平台

**Linux + Termux**。Windows 分支（v0.0.2 / Win7 legacy / `.bat`）全线冻结，不维护。

| 平台 | 安装 | 编译 |
|---|---|---|
| Linux | `g++` + `make`（系统自带即可） | `make -s ltir006` |
| Termux | `pkg install clang make`（Termux 无 g++） | `make -s CXX=clang++ ltir006` |

`ltir.sh` 自动检测 Termux 并切 clang++；项目若放在 `/storage/emulated/0`
（noexec），自动整体搬家到 `~/ltir006_lab` 后原地跑。标准 **C++17**（core 的
`rational.hpp` 用了 `std::string_view`），纯标准库，不用 `__int128`。

## 用法

```bash
bash ltir.sh            # 一键：内核门禁 → 编译 → SMOKE 44/44 → 演示
bash ltir.sh b|c|t|d|v  # 编译 / 门禁 / 自测 / 演示 / 版本

./ltir006 eval "1/3+1/6"                  # 单表达式
./ltir006 verify fixtures/demo.ltir       # 整卷（简明档）
./ltir006 verify fixtures/demo.ltir --explain   # 详细档（^ 指位 + 码表解释）
./ltir006 verify fixtures/demo.ltir --json      # 结构化输出
./ltir006 verify 卷.ltir --lang=en        # 强制英语（默认按卷内中文自动跟随）
./ltir006 verify 卷.ltir --codes=路径     # 指定码表
```

### 输出规格

```
L01 True  R
L04 False  E-VAL-MISMATCH
L21 UNDECIDED  E-INF
── 24 lines: 20 True, 3 False, 1 UNDECIDED ──
registered: 9 (zf/zfc/given/case/check/const/define/seq/set, ledger only)
```

详细档四行结构（与旧 002/003 的 B4 逐字同形）：行号 + 原文 + `^` 指位 + 码表解释。
`^` 按**显示宽度**（East Asian Wide 码点范围）算列；指位不准时省略 `^`，由
「错误码 + 行号」兜底。解释**全部由 `lab_codes.tsv` 驱动**，禁自由文本发挥。

## 目录

```
README.md  SYNTAX.md  速览.md  RUNRESULT.md
Makefile  ltir.sh     RUN_core_sha.txt  lab_codes.tsv  domains.tsv
core/          13 件冻结内核 + 门禁取证
src/           value.hpp（精确数值域） / ltir006.cpp（语言层 + CLI）
fixtures/      demo.ltir（整卷演示） / zfc.ltir（台账保留位）
tests/         smoke006.sh（69 条断言）
速览-0.0.5-原件.md   立法者发来的原件，sha 未动，只对语法不翻架构案
```

`--json` 现带 `domain / domains_registered / domains_seen / decorations /
preserved / inferences / answers`，每行带 `tag`；简明档末尾仅在显式用过领域或
标签时补一行 `domain:` / `tags:`，顶层原 24 行输出逐字不变。

## 保留项（按「暂时别动」令）

`zf:` / `zfc:` **只登记不判**：不进三态统计，只在 `registered:` 计数里出现。
`fixtures/zfc.ltir` 保留 11 条台账位。等后续要用时不用重新加上。

## 输出语言口径（如实报备）

速览 0.0.5 增量写「卷内无中文 → 英文；有中文 → 中文（自动跟随）」，
旧 002/003 的 B1 写「默认输出英语」。本版取**自动跟随**为默认，
并留 `--lang=en|zh|auto` 钩子；旧两栈已按删除令退场，逐字比对无对手方。

## 真机回填（立法者 Termux 截图）

- `clang++` 编译成功、`44/44`、noexec 搬家生效（提示符 `~/ltir006_lab $`）
- 抓到 `src/value.hpp:187` 一处 `Rational c2,d2,den,t` 死变量 → 真机 1 warning；
  本版已删 `,t`，g++ 侧复核 `0 error / 0 warning`
- 上轮「clang++ 没实跑」报备就此销案，双编译器取证成立

## 协议记档

- 内核 SHA `aecc9726…dab04f2319` 每次开工前由 `ltir.sh c` 验 11 文件
- 临时产物即生即清（`mktemp` + `trap`），不留点目录、不留 `__pycache__`
- 立法者后续若遇中断，统一用「**起来**」唤回继续工作
- 结论性报备（同源 / 全绿 / 零破坏）一律附数据，不拿目录名代替取证

## 未尽事项（只列，不做）

见 `RUNRESULT.md` 末节：P2 主体模块、P3 边界模块、Termux 真机 clang++ 回填、
AST 级复合 LaTeX 折叠、参数方程/极坐标、i18n、Windows 分支。

## 0.0.7（Linux + Termux，先发这一版）

- **集合论实验区开放**：旧护栏降级、`⊢` 地图坐标、AC/DC/BPI/AD 图层、
  大基数分叉、罗素悖论 / 绝对无限 / 模型论 / UltL / 全局 V / 奇异基数、
  哥德尔 / CH / 千禧年全部**登记或诚实弃权**，不判绝对真值。
- **公理模块**：`axioms/*.ltir`，`preset:` 内置按名、`load:` 外部文件；
  一个 `.ltir` 后缀，两种身份靠目录 + 首行 `module:` 分。
- **防「沙」**：n 个省略号折叠为一个 + 中英傲娇彩蛋（末句）。
- **有限集精确运算**：`∪ ∩ ⊆ ⊂ ∈ card |A|` 可判，`{1,2} ⊆ {1,3}` → False
  （不是 UNDECIDED）；无限/构造/嵌套/未登记符号一律 `UNDECIDED` 有码可查。
- **三角囚徒冻结卷**：`fixtures/triangle_prisoner.ltir` 重跑，
  0.0.6 的 `10 / 6 True / 4 UNDECIDED` → 0.0.7 `10 / 10 True / 0 UNDECIDED`，
  内联 `<C_and_E>` 拿回身份；主断言全称仍不伪判。
- **回归**：0.0.6 的 69 条里 65 条逐字未动，4 条期望随版本/领域表更新
  （版本串、`domains_registered` 24→27、`domain: ?`、汇总文案）。
- **Windows 10+**：源码共用，但 `ltir006_setup.exe` 本轮**不重打**；
  真机七项回填仍待立法者。
- **「起来」协议**：会话中断后发「起来」继续工作。

### 编译命令（忘了就看这里）
```bash
bash ltir.sh              # 一键
bash ltir.sh b            # 只编译
make -s CXX=clang++ ltir006  # Termux 裸敲
make -s ltir006              # Linux 裸敲
```

## 0.0.7 补丁（版本号不变 · Linux / Termux）

**本补丁不推进版本号**：`./ltir006 --version` 仍是 `ltir006 0.0.7`，无新后缀、无新版本名。
可执行文件 `./ltir006`、收容目录 `~/ltir006_lab`、`~/bin/termux-file-editor` 三锚点一字未动。

### 跑法（同上，不用记）
```bash
cd <解压目录> && bash ltir.sh       # 门禁 → 编译 → 175 条 → 演示
bash ltir.sh b                      # 只编译（Termux 自动 clang++）
./ltir006 --version                    # ltir006 0.0.7
```
Termux 外部存储 noexec：脚本自动搬家到 `~/ltir006_lab`。**若之前收容过旧目录，先
`rm -rf ~/ltir006_lab` 再跑**，否则拿旧源码跑（这次清单多了 `./termux`、`BACKLOG.md`、
`AXIOM_NOTES.md`）。

### 文件管理器钩子（随包，不用再手改）
```bash
cp ./termux/termux-file-editor ~/bin/termux-file-editor
chmod +x ~/bin/termux-file-editor
```
标题读 `$BIN --version`，`LAB` / `BIN` / 绝对路径参数全焊死，换版本不用改一行。

### 本补丁做了什么
- **修 5 类静默伪判 / 统计不一致**：`V:` 短写接入有限集语义、`card:` 字面集合与空集、
  容器内 `by:` 并入引用账、嵌套 `card` 归 `E-SCOPE-NESTED`、声明词一致时清残留错误码
- **甲：扩全图**：域表 27 → **55**（新增 28 个「仅台账域」），新增弃权码
  `E-SCOPE-LEDGER-ONLY`；台账域内判定请求有码弃权，不复用数学域求值器、不跨域串台
- **老账本归编**：见 `BACKLOG.md`
- Win10+ 包本轮不重打；`_WIN32` 分支仅纠正两处残留的旧版本串，版本号仍 0.0.7

详见 `BACKLOG.md` / `RUNRESULT.md` / `速览.md` / `SYNTAX.md`。


## 0.0.7 补丁二（版本号不变，仍是 `ltir006 0.0.7`）

- **修两个挂账 bug**：`V:` 求假不再挂 `E-VAL-MISMATCH`（求值标签 ≠ 断言标签）；
  `forall:` 认字面量有限集 `{...}`，花括号配对取域避开内部逗号
- **四个私货**：受限概括 `{x ∈ {1,2,3} | x > 1}` 精确算 `{2,3}`，无受限域 →
  罗素悖论闭环；原生 `∀ / ∃` 符号；`cex:` 反例登记册；
  `mapping:` + `inj:/surj:/bij:` 有限映射性质查表
- **码表** append `E-CEX-INVALID` / `E-SCOPE-CODOMAIN` 两行（只读消费）
- **SMOKE** `175 → 225`，新增 50 条；三角囚徒冻结卷 / demo / 用户卷逐字不漂
- **版本 007 焊死**：`--version`、JSON version、包名、顶层目录、`./ltir006`、
  `~/ltir006_lab`、hook 三锚点全部不动；Win10+ 包不重打

跑法与上一版完全相同：`bash ltir.sh`。

## 0.0.8

> 以下 0.0.7 / 0.0.6 章节均为历史留档，命令与版本叙述以本节为准。

- 版本号按令推进到 `0.0.8`；`VERSION` 为纯文本版本文件，`bash ltir.sh v` 或 `cat VERSION` 可查。
- 启动脚本改名 `ltir.sh`（旧 `ltir006.sh` 退役，避免新旧同名被 find 抓错）。
  `./ltir006`、`~/ltir006_lab`、`~/bin/termux-file-editor` 三锚点不动。
- `LTIR_STAMP` 底片：收容目录与包内底片不一致就**自动重搬**，不用再手敲
  `rm -rf ~/ltir006_lab`；外存就地改过源码可 `bash ltir.sh R` 强制重搬。
- 形式化定义：`sig: / type: / where: / rule: / grammar: / expand:`，
  定义体带签名、良构性检查、终止性检查与最多 8 步 δ 展开。
- 集合论多宇宙观：`multiverse: / world: / force: / ground: / mantle: / reachable:`，
  □◇（`\Box`/`\Diamond`）归一一律有码弃权；图册只登记，不跨图搬真值。
- 口径正名：千禧年七问、哥德巴赫是 `status: open`，从 `E-SCOPE-INDEPENDENT` 拆出
  `E-SCOPE-OPEN`；CH 仍为独立命题。
- 一键：`bash ltir.sh`（门禁 → 编译 → SMOKE → 演示）；`bash ltir.sh b|c|t|d|v|R`。
- SMOKE 入口为 `tests/smoke008.sh`；`tests/smoke007.sh` 保留为历史取证文件。
- Win10+ 包本轮不重打；Termux clang++ 真机取证待回填。

## 0.0.9

- 版本推进 `ltir006 0.0.9`；`VERSION` / `LTIR_STAMP` 同步。
- 现场修四个 bug：
  1. 多项式隐式乘法 `2xy` 等价于 `2*x*y`（`id:`/`derive:`/`solve:` 同路）
  2. 受限概括支持 `{x | x in {1,2,3,4} and x > 2}`（域在竖线右侧），
     无限域有码弃权 `E-SCOPE-INFINITE`，真·无受限域才 `E-PARADOX-RUSSELL`
  3. `21!` / `100!` / `2^64` 归 `E-OVF`，不再戴 `E-SCOPE-TRANSCENDENTAL` 的帽子
  4. 三角特殊角补 π/12 家族 8 个角（`RAD2` 双根式精确值，零浮点）
- 新码 `E-SCOPE-ALGEBRAIC`：双根式暂不参与四则/比较，诚实弃权。
- 三个新功能：
  - **卸载**：`./uninstall.sh`、`bash ltir.sh u`、`./ltir006 uninstall`（只删
    `~/ltir006_lab` 与 hook，不碰用户 `.ltir` 卷）
  - **当前终端只输出三态**：`bash ltir.sh tri` 打印
    `export LTIR_OUTPUT=tri`；也可用 CLI `--tri`。恢复 `--normal` /
    `export LTIR_OUTPUT=normal`
  - **只验算不给答案**：`./ltir006 check 卷.ltir` 或 `verify --no-answer`，
    三态与错误码照出，答案值不输出
- 入口 `tests/smoke009.sh`；`tests/smoke008.sh` 保留为历史取证。
- `fixtures/fix009.ltir` 为四个 bug 的复现卷；`GITHUB_INTRO.md` 为多版对外文案。
- Win10+ 包本轮不重打；Termux clang++ 真机 0.0.9 待回填。
- 一键：`bash ltir.sh`；子命令 `b|c|t|d|v|R|u|tri`。

</details>

</details>
