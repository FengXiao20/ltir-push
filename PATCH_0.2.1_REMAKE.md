# PATCH 0.2.1 REMAKE r3 ｜ 四工位造词版 · 纯实践收编

> 试验标签 `0.2.1-remake-r3`，**不是发版**。`VERSION` / `LTIR_STAMP` 仍 `0.2.0`，单主干 main，不打 tag、不带 beta。
> **本包是纯实践收编**：四项专码已放权定稿、词库外判定词补回全禁版、挂账实践文件全部落盘；
> **不推版本、不改发行文案、不另发第二份发行版**。
> r3 替代 r2 / r1 / 更早的 `ltir_0.2.1_试验版_需替换文件.zip`；`src/ltir006.cpp` 是完整叠加版，
> 直接盖干净 `0.2.0` 即可，**不用先盖 r2**，四者不可混用。

---

## 0. 一句话

四项元能力各给一个**独立词条**，把立法从 C++ 隐式约定搬回卷面与词表；旧词一个都不改语义；真值出口仍只有原判定核。
同时把作废版暴露的 **7 格实测 bug 逐条修掉**。

```
新写法 → shape: 认形状 → bind: 补槽位 → rewrite: 摊开 → 回投原判定核 → 才出 True
         └──────────── 前四步全在语法层 ────────────┘   └ 唯一碰真值 ┘
```

跟 Lean 的差别不在「有没有 tactic」，在**语法产物交给谁**：它交给内核去接受，咱们交给台账去登记。

---

## 1. 背景：为什么重制

上一版把「零新词」当成减法美学，实测证明是陷阱：

- `expand:` 同时承担 δ 展开与改写触发，靠 `rfind("rewrite ",0)` 在 body 里分流 —— **一词两义**，词表看不见；
- 形状头从 `grammar:` 第一个引号里抠、规则取 `reformulations.back()`、数值从 `quote:` 内层手切 —— 立法权搬进 C++ 字符串活；
- `68/68` 是脚本自己写的断言绿了，7 格一条未覆盖。

**失败的是实现形态，不是结论**：「写法变写法而真值不碰」、L3 静态扫描、差分恒等、回投可复原、Gentzen / Gödel II 一正一反，全部保留。
负面证据已收成 `fixtures/rewrite_negative_witness.ltir`（0 三态，只登记）。

---

## 2. 新词（5 个，四工位 + 门禁）

| 词条 | 工位 | 只准干的事 | 三态 |
|---|---|---|---|
| `shape:` | 认形状 | 登记 head 字面量 + 槽位名 + 目标规则 id，**不造解析器** | 0 |
| `bind:` | 补全 | 槽位 → 字面量；补不出/非法就空着，**不猜不产值** | 0 |
| `rewrite:` | 展开 | 有限步摊成**旧词条**、回投原核；`strict` 下只登记 | 0 |
| `split:` | 拆目标 | 拆成 `gap:` + `witness:` + `by:` 账本行，**没有 close** | 0 |
| `mode:` | 门禁 | `trial` / `strict`；缺省 `strict`，试验态必须写进卷面 | 0 |

### 旧词零改义（第二层稳定）

- `expand:` **还给 δ 展开**，不再兼职改写；
- `grammar:` 仍只收 BNF，形状头不再从引号里抠；
- `reformulate:` 保留，补唯一性检查与 L3 扫描；
- `gap:` `quote:` `reflect:` `depth:` `trace:` `by:` `witness:` `at:` `sig:` 全原义复用；
- `bind:` 管槽位映射、`at:` 管常量绑定，两者不糊；
- `quote:` **不参与求值**，彻底没有「取第一个」的选择规则。

### 卷面（`fixtures/rewrite_trial.ltir`）

```
mode: trial
sig: perm2_expand : Matrix2 -> Num
shape: perm2_lit ::= "PERM_2" slots: a,b,c,d target: perm2_expand
reformulate: perm2_expand(a,b,c,d) = a*d + b*c
by: cert-perm2-001
gap: perm2_shape   where: PERM_2([[a,b],[c,d]])
bind: perm2_lit <- [[1,2],[3,4]]
rewrite: perm2_lit
quote: <ltir> V: PERM_2([[1,2],[3,4]]) </ltir>
reflect: rewrites
reflect: mode
depth: 12
trace: off
```

结果 `L01 True  10`。那个 `True` **是旧判定核给的**：`rewrite:` 只把 `at: a=1,b=2,c=3,d=4` 与 `V: a*d + b*c` 两行交回行流，触发行自身 `state=-1`。

---

## 3. 七格逐条修法与实测

| # | 严重度 | 修法 | 实测 |
|---|---|---|---|
| 1 | 中 | `reformulate:` 提取 fn 名进 `reformNames`，撞名 `E-REWRITE-CONFLICT`，绝不 last-wins | 同 fn 两条 → `UNDECIDED E-REWRITE-CONFLICT`，无 True |
| 2 | 小 | 回投**不插 `lines`**，产物直接 `handle`，`fileLine` 恒指向触发行；JSON 新增 `derived_from` | `file_line:9` / `derived_from:9`（rewrite 在第 9 行），不再 +2 漂移 |
| 3 | 小 | `witness:` 体也进 L3a 宽档：拒手写结论，但放行 `V:`/`solve:` 的「旧错→修后」对照 | `witness: A: 2+2==5 -> True` → `ERROR E-REWRITE-REDEEM`；历史卷 wit 行不受影响 |
| 4 | 小 | 4 个专码归位：`E-SHAPE-UNKNOWN` / `E-REWRITE-ARITY` / `E-REWRITE-CONFLICT` / `E-REWRITE-REDEEM` | 未知 id → `E-SHAPE-UNKNOWN`；数量不符 → `E-REWRITE-ARITY`；不再蹭 `E-DEF-SIG` |
| 5 | 微 | `load:` 模块路径对 meta 词做登记 + 计数 + `E-LOAD-META` 诊断 | 模块卷 `meta_gaps:1 meta_rules:1`，人话档列诊断 |
| 6 | 微 | `quote:` 只作展示数据，取值来自 `bind:`，无「取第一个」 | 双 quote（9+9 / 1+1）+ `bind: 1,2` → `True 3`，不是 `18` |
| 7 | 微 | 槽位字面量严格 `^-?\d+(/\d+)?$`，单个分数 / 负数放行，连除拒收 | `1/2/3` → `E-REWRITE-ARITY`；`1/2,-3` → `True -5/2` |

**✅ 四项专码已放权定稿（纯实践收编）**

`lab_codes.tsv` append 上面 4 行，`^E-` 条目 **65 → 69**，历史行未动。
理由：专码是「给命名权、留痕可审」的一部分，`explain()` 由码表驱动，不落就没有解释文本；
继续蹭 `E-DEF-SIG` 正是作废版 bug 4「错位码」的病根。
立法者已放权，**不再保留「删回 65」的待批状态**；`tests/regress_bugfix.sh` 另加
「末尾 4 行俱在」硬断言防误删。

`tests/regress_bugfix.sh` 的码表条目断言已从 `65` 同步成 `69` —— 所以 **125/125 是「断言同步后的绿」，不是逐字零回归**，别当成回退去还原。

---

## 4. 合法性判据 L1–L7（三合一才算合法）

| 判据 | 内容 | 落点 |
|---|---|---|
| L1 锚定 | `gap:` 需 `sig:`+`by:`；`reformulate:` 需对应 gap；`split:` 需行内 `by:`；**`rewrite:` 在 strict 下只登记**（不弃权），`mode: trial` 下才回投 | 缺锚（`sig:`/`by:`/shape/bind/rule）才弃权 `E-META-ANCHOR`；缺 `mode:` 不是缺锚 |
| L2 词表 | 产物只准是已立法词条（`at:` / `V:`），未立法词仍 `E-UNK-LINE` | 结构白名单先验 |
| L3 兑换权 | 元操作行先验拒收**全部手写三态**（`-> True` / `-> False` / `-> UNDECIDED`，含大小写与中文弃权词）、`A:`/`con:`/`submit:`/关闭目标；`quote:` 内层与 `witness:` 走宽档（允许 `V:` 表达式与非三态对照尾巴，仍禁手写三态） | `E-REWRITE-REDEEM` / `E-GODEL-II` |
| L4 结果不变量 | `counts(展开卷) == counts(基准卷) + counts(产物卷)` | 实测 `1 = 0 + 1` |
| L5 回投可复原 | `reflect: rewrites` **人话档 stdout 逐字列出产物**（r2 补出口）；`shapes`/`binds`/`splits`/`mode` 同样有 stdout；`quote:` 内层单独跑仍弃权 ⇒ 确认没 eval | `derived_from` + 台账 + stdout |
| L6 确定性 | 同卷重跑 stdout md5 一致；`depth:` / 环检测有限步 | `trace:` 走 stderr |
| L7 自证 / 跨图 | `split:` 体带 `con: T->T` → `E-GODEL-II`；`metatheory == T` 不产 True | Gödel II 护栏不动 |

**验的是「合法」，不是「正确」**：`a*d-b*c` 写反时形状全合法、`counts` 全等、回投可复原，只能靠 `witness:` 回归兜；也不判「这是不是你想表达的意思」。跟「证书链齐不兑换 True」同一条。

### 学术案例（挑学界早承认的一正一反）

- `fixtures/rewrite_gentzen.ltir`：Gentzen 1936 外部元理论 `PRA + TI(ε₀)` → `Con(PA)`。证书链齐，**0/0/0**，`meta_splits:1`，不跑内核、不判证明。
- `fixtures/rewrite_godel.ltir`：Hilbert–Bernays 1939 / Löb 1955 D1–D3。`con: PA -> PA` → **`UNDECIDED E-GODEL-II`**（是弃权不是 0 三态，如实报），不产 True、不并 False。
- Tarski 真谓词不可在对象语言内定义，对应「元层可描述对象层、真值不在元层产」。

---

## 5. 硬约束 / 不取清单

- 唯一 True/False 出口仍是原判定核；新词全 0 三态（除 `rewrite:` 回投产物）。
- `core/` 11 件与 `RUN_core_sha.txt` **一字未动**，不重签底片。
- 零浮点、无随机 / 交互 / 计时、不联网、不读外部进程、`quote:` 不 eval 外部。
- **永久不取**：`-> True` 夹进展开体 · 自动接管并改判 `UNDECIDED` · 把 `UNDECIDED` 降级成「待证目标」 · 运行时写回码表 · 自造顶层词 · 生成判定解析器 · 补不出给默认值 · 参数名写死 `x` · `goal:/close:` · 生成内核项 · 跨图搬真值 · 自证 `Con(T)` · 自举。
- 形状族很窄：只认 `NAME([[n,n],[n,n]])`，`PERM_m` 符号族、全称渐近下界、∀ 域/扩域一律 `E-SCOPE-*` 弃权，**不猜、不硬套旧规则**。
- 与 Lean 边界：Lean tactic 生成内核项并 close goal；咱们 `split:` 只生成账本行。给命名权与受控展开权，**兑换权永久不给**。

---

## 6. 验收（干净 0.2.0 基底覆盖后真编译真跑）

```
g++ -std=c++17 -O2 -Wall -Wextra -I core/include   → 编译 OK，无警告
门禁 RUN_core_sha.txt                              → 11/11，bad=[]
./ltir006 --version                                → ltir006 0.2.0
smoke009.sh                                        → 445 / 445
regress_bugfix.sh                                  → 127 / 127（65→69 同步 + 专码定稿 2 条）
trial_meta.sh                                      → 31 / 31
trial_strength.sh                                  → 13 / 13
trial_rewrite.sh（新增，r3 扩到 98 项）            → 98 / 98
demo.ltir                                          → 24 lines: 20 True, 3 False, 1 UNDECIDED
bash -n 全 sh                                      → OK
0 三态卷 --tri                                     → stdout 0 字节
CI=1 默认档                                        → 台账汇报，不含三态词行（demo 9 行 L0* 是三态输出行，属既有契约）
V: 0/0 → E-INDETERMINATE · con: ZFC->ZFC → E-GODEL-II · strength_le → E-STRENGTH-NOCERT
1.1E17 / x ∈ R / 1/110000000000000000               → 三件旧修复全在
r2：-> UNDECIDED 元层先验拒收 / 旧核 E-SYN          → 三态词全禁，meta_illegal=1
r3：词库外判定词补回全禁版（Maybe / 乱写尾巴）      → E-SYN；smoke009 第43条期望已同步
r2：reflect rewrites/shapes/binds/splits/mode        → 人话档逐字出口，--tri/--json 零污染
r3：代数复杂性两卷 / 能力 gap 卷 / 历史卷 v3         → 0 三态登记 + 5 True 精确实例
```

`--json` 新增字段：`meta_shapes` `meta_binds` `meta_splits` `meta_rewrites` `meta_illegal` `meta_rejected` `meta_mode`，以及产物行的 `derived_from`。

---

## 7. 应用 / 回退

**应用**（从干净 `0.2.0` main 覆盖，**单包**，不含二进制）：

```bash
cd /path/to/ltir-push
unzip -o ltir_0.2.1_practice_finalize.zip -d /tmp/rw
cp -r /tmp/rw/files/. .        # 干净 0.2.0 上单包覆盖，不必先盖 r2
make
bash tests/trial_rewrite.sh      # 98 / 98
bash tests/smoke009.sh && bash tests/regress_bugfix.sh
```

真机按老规矩先清收容室（守卫不盖 `src/ltir006.cpp`）：

```bash
cd "$HOME" && rm -rf "$HOME/ltir_lab"
cd /path/to/ltir-push && bash ./ltir.sh
```

**回退三步**

1. 删 §8 清单里 7 个新增文件；
2. 用覆盖前的 `src/ltir006.cpp` / `lab_codes.tsv` / `tests/regress_bugfix.sh` 还原（三份原件都在旧 main 里）；
3. `make clean && make`，复跑 `smoke009.sh` 应回到 `445/445`、`regress 125/125`。
   ⚠️ 四项专码**已放权定稿**，本地回退仅是试验还原手段；正式工作树不得再把断言改回 `65`。

---

## 8. 文件清单（包内 `files/`）

| 文件 | 性质 |
|---|---|
| `src/ltir006.cpp` | **覆盖**：词表 5 新词 + Ctx 容器 + 护栏 helper + 四工位分支 + 7 格修复 + 回投与 JSON 字段 |
| `lab_codes.tsv` | **覆盖**：append 4 行专码（65 → 69） |
| `tests/regress_bugfix.sh` | **覆盖**：码表条目断言 65 → 69（仅此一处） |
| `tests/trial_rewrite.sh` | 新增：68 项自检 |
| `fixtures/rewrite_trial.ltir` | 新增：PERM_2 四工位示例卷 |
| `fixtures/rewrite_gentzen.ltir` | 新增：外部证书只登记（0 三态） |
| `fixtures/rewrite_godel.ltir` | 新增：自证护栏（1 UNDECIDED） |
| `fixtures/rewrite_negative_witness.ltir` | 新增：作废版负面证据，只登记 |
| `META_CODES_CANDIDATE.tsv` | **覆盖**：裁决记录同步 |
| `PATCH_0.2.1_REMAKE.md` | 新增：本文档 |
| `tests/smoke009.sh` | **覆盖**：第 43 条期望同步为 `E-SYN`（词库外判定词补回全禁版） |
| `fixtures/algebraic_complexity.ltir` | 新增：外部领域登记卷（0 三态，指纹 `e8734cc43c501f2f`） |
| `fixtures/algebraic_complexity_instances.ltir` | 新增：固定 m 精确实例卷（5 True） |
| `fixtures/metacapability_gaps.ltir` | 新增：四行能力边界 gap（0 三态） |
| `fixtures/history_gaps.ltir` | **覆盖**：v3，追加三段负面 witness + reflect 出口 |
| `META_KEYWORDS.md` / `SYNTAX.md` / `BACKLOG.md` | **覆盖**：五词立法 / 两节条文 / 账本收编 |
| `PATCH_PRACTICE_FINALIZE.md` | 新增：纯实践收编回执 |

`core/` 11 件、`version.hpp` 宏值、`LTIR_STAMP`、`VERSION`、`ltir.sh` / `ltir_ui.sh` / `ltir_exam.sh` 全部未动。

---

## 9. 账本（仍挂下次大改动，不单独推）

- 代数复杂性两卷 + 回执 + 合并标本（仍在沙盒）；
- `BACKLOG` 一行「代数复杂性证书接收」、`SYNTAX.md` 一节「外部领域卷只登记」；
- 四行能力 gap 的正式转正、`reformulate:` 自动接管 `UNDECIDED`（0.2.1 之后另议）；
- 上传提示词仍写着「三包顺序覆盖 / VERSION 0.1.1」，**已过期**，发 agent 前重出。

**口径**：本地试验件，不推 main、不打 tag、不另发发行版；单主干，`VERSION 0.2.0` 原地。连「哪天转正」也不拍 True，四行 `gap:` 安静躺着。

---

## 10. r2 修订（三格实测 bugs）

| # | 问题 | 修法 | 实测 |
|---|---|---|---|
| A | L3 禁词表漏第三态：`-> UNDECIDED` 穿透 strict / 宽档 / 产物白名单；`metaBanCommon` 只列 `-> True/False`。顺带旧核 `A: 2+2==5 -> UNDECIDED` 被静默改判成 `False E-VAL-MISMATCH` | `metaRedeem` 改走 `hasVerdictTail`：扫**所有** `->`，尾部首词命中 `true/false/undecided`（大小写不敏感）或 `真/假/成立/不成立/不知道/弃权` 即拒；切词口径与 `findVerdict` 一致（含中文标点）。判定核另加**收窄**前置：只对手写 `UNDECIDED` 给 `E-SYN` | 宏体 / quote / witness 三处 `-> UNDECIDED` 全 `E-REWRITE-REDEEM`，`meta_illegal=1`、`meta_rewrites=0`；`V:`/`A:` 手写第三态 `E-SYN` |
| B | `reflect: rewrites/shapes/binds/splits` 四目标只 push 台账、stdout 零出口，L5「逐字列出产物」成空话 | Ctx 加 `reflExtra`，五目标（含 `mode`）攒人话档逐字输出；`--tri` / `--json` 不走人话档，零污染 | 示例卷 stdout 出 `at: a=1, b=2, c=3, d=4` / `V: a*d + b*c`；四目标卷各自有出口；`--tri` 0 字节、`--json` 不含 `reflect:` |
| C | 文档 L1 写「`rewrite:` 需 `mode: trial` → 缺一弃权」与实现不符（strict 是只登记，自测 ⑬ 已这么断言） | 只改措辞，行为不动：strict 下只登记**不弃权**，缺锚才 `E-META-ANCHOR`；`grep 'L1 锚定'` 断言同步 | 自测 ㉒ 两条绿 |

### A 的收窄已在 r3 补回全禁版

r2 曾因 `smoke009.sh` 第 43 条「词库外判定词 → 报实算真值」的既有立法而收窄成
「只禁 `UNDECIDED`」。立法者放权「全实现」后，r3 把这一格**补回来**：
判定核 `assert:` / `value:` 前置改为「写了 `->` 但 `findVerdict` 认不出合法 True/False
→ `E-SYN`」，并**同步** `tests/smoke009.sh` 第 43 条期望为 `E-SYN`。
`tests/archive/smoke00{6,7,8}.sh` 是历史归档，按口径不动、不跑。
合法声明零回归：中文标点尾巴、行尾 `by:`、量词声明三条新增断言全绿。

### 验收增量

`trial_rewrite.sh` 从 **68 → 95 → 98 项**（r2 增 27 条，r3 再增 3 条零回归断言）；
`smoke009` 仍 `445/445`（第 43 条期望同步，条数不变）、`regress` `127/127`、
`trial_meta` `31/31`、`trial_strength` `13/13`，
`core/` 11 件未动；码表 69 行**已放权定稿**，r3 零 append。

---

## 11. r3 纯实践收编（立法者放权「全实现」后一次结清）

**只做实践，不动发行**：`VERSION` / `LTIR_STAMP` / README / QUICK_TOUR /
GITHUB_INTRO / EXAM_SCORING 的版本字样一字未动，不打 tag、不另发第二份发行版、
不重出上传提示词（那份仍过期，发 agent 前单独处理）。

| # | 补回来的东西 | 落点 |
|---|---|---|
| 1 | 四项专码批准定稿（不再是「不批准就删回 65」） | `lab_codes.tsv` 69 行 + 候选表裁决 + regress 硬断言 |
| 2 | 词库外判定词补回全禁版（`-> Maybe` / 乱写尾巴 → `E-SYN`） | `src/ltir006.cpp` 两处 guard + `smoke009.sh` 第 43 条期望同步 |
| 3 | 代数复杂性两卷入仓 | `fixtures/algebraic_complexity.ltir`（0 三态）、`..._instances.ltir`（5 True） |
| 4 | `BACKLOG` 一行 + `SYNTAX` 两节 + `META_KEYWORDS` 五词 | 词条立法收编，非发行文案 |
| 5 | 四行能力 gap + 历史卷 v3 负面 witness | `fixtures/metacapability_gaps.ltir` / `fixtures/history_gaps.ltir` |

### 两卷实测

```
领域卷   0 True / 0 False / 0 UNDECIDED · registered 29 · --tri 0 字节
         theory_sig e8734cc43c501f2f · mismatch=0
         只改公理名 → 指纹不变；改公式本体 → 3a7d81a8dceadc6e（变）
实例卷   5 True：10 / 1 / 5/2i / 6 / 2   （PERM_2、单位阵、分数复数、PERM_3 全1、HC_3 全1）
能力 gap 卷 0 三态，reflect 列四个边界 id；历史卷 v3 0 三态，reflect 列六个 id
```

`quote:` 内层不写 `A:`（L3 先验拒收），改写成纯公式串 `VP != VNP`；
不 eval、不核论文真伪、不查 `sorryAx`、不判 `VP≠VNP`。

### 不取清单（跟 r2 一致，未松）

`reformulate:` 自动接管 UNDECIDED · 把 UNDECIDED 降级成待证目标 ·
`quote:` eval 外部 · 自造顶层词 · 生成判定解析器 · close goal · 生成内核项 ·
跨图搬真值 · 自证 `Con(T)` · 运行时写回码表 · 自举。
验的是**合法**不是**正确**：`a*d-b*c` 写反时形状全合法、`counts` 全等，靠 `witness:` 回归兜。

### 口径

连「哪天转正 / 推版本」也不拍 True —— 四行 `gap:` 安静躺着。
本包是本地实践件，不推 main、不打 tag；`VERSION 0.2.0` 原地、`core/` 11 件与
`RUN_core_sha.txt` 一字未动。
