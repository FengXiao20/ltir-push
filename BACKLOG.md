# LTIR 老账本归编 · 0.0.7 补丁（版本号不变）

> 本令：甲 —— 只扩「域表 + 台账 + 弃权码」，**一个判定核都不加**。
> 下面所有条目都只是登记，不代表已实现；三态锁死、真值自产、Richardson 不伪判不变。

## A. 老挂账（历轮反复出现，按出处归编）

| 条目 | 首次出处 | 档位 | 口径 |
|---|---|---|---|
| 分段函数 / 参数方程 / 解三角形 | 0.0.2 起反复挂账 | P2 | 能力缺口，非语法缺口 |
| 立体几何 / 线性回归 | 0.0.2 起反复挂账 | P2 | 同上 |
| 角度制精确值 sin40° / sin² / 中点乘号 · | 三角囚徒取等点带出 | P3 | Richardson 界同类纪律：不许浮点夹出假 True |
| 无理根证书 / 区间证书（乙案） / 符号证明（丙案） | 0.0.7 起草 | P2 / 丙需明令解禁 | 多变量符号求解永久边界 |
| 浏览器宿主 / 标黄渲染 / i18n / C++ 性能核 | 历轮只列不做 | P3 | 工程项，不属判卷语义 |
| Win10+ 真机七项回填（双击安装/双击卷/auto 秒数/associate/卸载/UTF-8/空格路径） | Win 包交付时 | 待回填 | 沙盒无 Windows 环境 |
| 两套 state 编码统一 / map: 去留 / \subset 歧义一刀 | 0.0.7 起草 | 待裁 | 码表 append 权在立法者，引擎只读消费 |
| 幂集 P(A) 与 Cantor 对角论证落语法 / 嵌套集合精确 / ⊨ 与 ⊢ 分岸 / 双层 choice: | 集合论全图 | P2 | 实验区 L2 有条件推导 |
| 无限集展开 / 共尾性 SCH / pcf / forcing 判定核 | 集合论全图 | P2 | 已登记地图，无判定核 |
| zf:/zfc: 台账升格为可引用证书链 | 实验 C | P2 | 公理是假设不是定理，不判 True/False |
| Milestone 七问 / CH 独立专档 / 哥德尔 / 大基数分叉 / 罗素 / Ω / UltL | 集合论全图 | 只登记 | 本轮已落台账，判定仍弃权 |

## B. 新域台账（甲：28 个仅台账域）

`domains.tsv` 里 scope 含 `[台账]` 标记。行为：**该域内不复用数学域求值器**，
`A:` / `V:` / `claim:` / `card:` 等判定请求一律 `UNDECIDED E-SCOPE-LEDGER-ONLY`；
登记类行（`system:` / `axiom:` / `ref:` / `problem:` / `set:` 等）照常登记、不产三态行。

| 族 | 域 |
|---|---|
| 分析（8） | analysis / real-analysis / measure / functional-analysis / complex-analysis / differential-equation / numerical-analysis / dynamical-systems |
| 代数·几何·拓扑（10） | abstract-algebra / galois-theory / algebraic-geometry / graph-theory / combinatorial-optimization / topology / algebraic-topology / differential-geometry / category-theory / analytic-number-theory |
| 概率·应用（10） | measure-probability / stochastic-process / statistical-inference / information-theory / game-theory / convex-optimization / operations-research / computational-complexity / financial-mathematics / ergodic-theory |

每个新域缺的是「判定核」，不是名字；本版零解禁、零新算子、零新短写。

## C. 集合论实验区（已铺图，未落判定核的半张图）

- L2 有条件推导（`by:` 证书链 → 相对可证），现只查名字不推导
- 幂集 / 对角论证、嵌套集合精确、共尾性与 SCH、forcing / 内模型 / 模型论判定核
- `⊨` 语义蕴涵需证书、`⊢` 可证需地图、双层 `choice:` 开关
- 无限集 `{0,1,2,...}` 不猜通项、不自动展开、不自动判 ∀

## D. 工程与宿主

- Win10+ `ltir006_setup.exe` 本补丁不重打；源码同源、`_WIN32` 分支仅纠正两处残留版本串
- Termux 真机 0.0.7 补丁待回填（0.0.7 首版 136/136 真机取证不回退）
- `~/bin/termux-file-editor` 随包交付：三锚点焊死，标题读 `$BIN --version`，不随版本改
- 包管理：卸载、多版本共存、增量更新、收容清单自动比对（未做）

## E. 销案记录（不回退）

- 0.0.6 强化修复版：Termux clang++ 69/69、noexec 搬家、clang 死变量清除
- 0.0.7 首版：Termux clang++ 136/136、防沙、公理模块、三角囚徒冻结卷 10/10
- 0.0.7 补丁：本轮修 5 处静默伪判/统计不一致（见 RUNRESULT.md）

## F. 仍锁

三态 · 真值自产 · Richardson 不伪判 · 码表只读消费 · core 11 件 SHA `aecc9726…` 零改动 ·
三角囚徒冻结卷 `10 / 10 True / 0 UNDECIDED` · 136 条回归 · `./ltir006` / `~/ltir_lab` / hook 三锚点不动 ·
版本号 `0.0.7` 不变、无新后缀。


## G. 补丁二销案与新增挂账（版本 007 不变）

**销案**
- `V:` 求假挂 `E-VAL-MISMATCH` —— 求值标签与断言标签混码（连同 `forall` 判假蔓延）
- `forall:` 字面量有限集误弃权 —— 花括号内部逗号被当分隔符
- `set: {x | x ∉ x}` 竖线构造符被当枚举，产出 `{UNDECIDED}` + 伪判 False
- hook 里 `load:` 按 cwd 找模块 → MT 当 IDE 时引理不起效（已归位到收容目录）
- 手机上 hook 标题残留 `(0.0.6)` → 明令对齐 `0.0.7`（**不是版本回滚**）

**新挂账（不夹带）**
- `⊢` 同行两层声明词的 `^` 指位仍偏（详细档里 `ZFC  vdash  CH`）
- `⊨` 语义蕴涵只有证书位，没有判定核
- 无限集展开 / 区间证书 / 嵌套集合精确判定 / forcing 与内模型推导（P2）
- 码表两套 state 编码统一、`map:` 去留、`\subset` 学术歧义那一刀
- Win10+ 包重打（payload 清单要补 `BACKLOG.md` / `termux/` / 本次新增码表行）
- 短写表开表、`V:` 与 `val:` 双短写归一

**仍锁**：三态 · 真值自产 · Richardson 不伪判 · 码表只读消费 · core 11 件 SHA
`aecc9726…` 零改动 · 三角囚徒冻结卷 `10/10` · 225 条回归 · 版本 `0.0.7`
不变、无新后缀 · `./ltir006` / `~/ltir_lab` / hook 三锚点不动。

## 0.0.8 归编

- 已落：形式化定义（签名/类型/良构性/终止性/8 步 δ 展开/BNF 登记）、
  多宇宙图册（只登记）、哥德巴赫专档、七问口径正名、
  `ltir.sh` 改名、`LTIR_STAMP` 底片、`VERSION` 查版本。
- 仍挂 P2：forcing / 内模型 / 模型论判定核、⊨ 判定核、模态定理证明器、
  无限集展开、区间证书、嵌套集合精确、双层 `choice:` 写法、
  同行双层声明词优先级、28 个仅台账域零判定核、Win10+ 包重打、
  Termux clang++ 真机 0.0.8 回填。

## 0.0.9 归编

- 已落：隐式乘法、受限概括双写法、阶乘/幂溢出归类、π/12 精确角、
  `RAD2` 双根式、`E-SCOPE-ALGEBRAIC`、三态输出 / 只验算 / env 模式、
  `uninstall.sh` + `ltir.sh u` + `./ltir006 uninstall`、
  `GITHUB_INTRO.md` 多版对外文案、`fixtures/fix009.ltir` 复现卷。
- 仍挂 P2：`RAD2` 参与四则/相等判定、forcing / 内模型 / 模型论判定核、
  模态定理证明器、无限集展开、区间证书、嵌套集合精确、双层 `choice:`、
  同行双层声明词优先级、28 个仅台账域零判定核、Win10+ 包重打、
  Termux clang++ 真机 0.0.9 回填。

---

# English summary

The ledger of deferred items ("only lists what it does NOT do").

**A. Old debts** (recurring, by origin): piecewise/parametric/triangle-solving (P2),
solid geometry & linear regression (P2), exact degree values sin40° / sin² / midpoint
dot (P3 — Richardson-class discipline), irrational-root & interval certificates (P2+),
browser host / highlight rendering / i18n / C++ perf core (P3), Win10+ repack items
(pending), unifying the two state encodings / `map:` fate / `\subset` ambiguity
(awaiting ruling), power set & Cantor diagonal in syntax, nested-set exactness,
`⊨` vs `⊢` banks (P2), infinite-set expansion / SCH / pcf / forcing kernels (P2),
upgrading `zf:/zfc:` to citable certificate chains (P2).

**B. Ledger-only domains**: 28 domains flagged `[台账]` in `domains.tsv` — names on
the map, zero judgment kernels; every judgment request inside them returns
`UNDECIDED E-SCOPE-LEDGER-OLD`… precisely: `E-SCOPE-LEDGER-ONLY`. Zero new kernels,
zero new operators, zero new short forms in that patch.

**C. Set-theory experimental zone**: conditional L2 derivation via `by:` certificate
chains (name-checking only today), power set / diagonal, nested sets, cofinality &
SCH, forcing/inner-model/model-theory kernels — mapped, not implemented. `⊨` needs a
certificate; `⊢` needs a map; infinite sets are never expanded or guessed.

**D. Engineering**: Win10+ installer not rebuilt (same-source, `_WIN32` branch only
fixes two stale version strings); Termux on-device re-verification pending per version;
package management (uninstall, multi-version coexistence, incremental update, manifest
diff) not built.

**E. Closed cases** (no rollback): 0.0.6 reinforced (Termux clang++ 69/69, noexec
relocation, dead-variable cleanup); 0.0.7 first (136/136 on-device, anti-sand, axiom
modules, trigonometric-prisoner frozen scroll 10/10); 0.0.7 patch (5 silent
false-verdict/statistic fixes, see RUNRESULT.md); patch 2 (evaluation/assertion code
separation, literal-set quantifiers, `|` constructor, hook load path, title version
alignment).

**F. Still locked**: three states · self-produced truth · Richardson no-fake-verdicts ·
code table read-only · core 11 files SHA `aecc9726…` untouched · frozen scroll 10/10 ·
binary/containment/hook anchors unmoved.

**0.0.8 / 0.0.9 entries**: landed — formalized definitions (signatures, well-formedness,
termination, 8-step δ, BNF registration), multiverse atlas (registration-only),
Goldbach dossier, seven-problem naming, `ltir.sh` rename, `LTIR_STAMP`, `VERSION`;
implicit product, restricted comprehension, overflow classification, π/12 exact
angles, `RAD2`, `E-SCOPE-ALGEBRAIC`, output modes, uninstall, `GITHUB_INTRO.md`,
`fixtures/fix009.ltir`. Still pending: `RAD2` arithmetic, forcing/inner-model/modal
kernels, infinite-set expansion, interval certificates, nested-set exactness, double
`choice:`, same-line double declaration precedence, Win10+ repack, Termux re-verification.

## 0.1.0 归编

已落：试卷库六档 + `exam.json` + `submit:` + 考前声明 + 延迟反馈 + 结构打分 +
Agent 契约；进制眼镜；顶层赋值环境表；一致性强度/势/真类/范畴论 L0 台账；
`\b` 进度条；时区识别中英文 UI；demo 纯检查；`~/ltir_lab` 收容目录；
SMOKE 分组标题去版本号（推 0.1.1 不再改标题）。

留档（不进本包，等令）：换世界层（`carrier:` 有限结构判定核）、证明论序数、
反推数学 Big Five、描述集合论 Wadge 度、模型论稳定性、类型论证书导入、
HoTT universe、一般极限构造、2-范畴、∞-范畴、Win 重打。

## 0.1.0 A 增量（公理本体化，已落）

已落：`lang:` 语言签名 · `axiom: Name = <公式>` 公式本体 · `preset:`/`load:` 同源解析 ·
`theory_sig:` 从空壳登记升级为 FNV-1a 64 内容指纹（按内容不按名字/顺序）·
JSON 四字段 `axiom_bodies` / `lang` / `theory_sig` / `sig_mismatch` ·
`fixtures/theory_sig.ltir` · SMOKE 405/405。`VERSION` 仍 `0.1.0`，core 11 件未动。

**下版本草图 B / C / D（等令，不夹带）**
- B：`depth: 12` 解开 δ 展开写死的 8 步上限；`trace:` 吐逐步展开轨迹，
  供试卷库按过程打分；`rule:` 从 `by:` 白名单升为有限步可重写规则。
- C：`<theory NAME> … </theory>` 局部理论块，公理集压栈-出栈恢复；
  与进制眼镜 / 赋值环境同层，跨图仍不搬真值。
- D：`axiom: X requires: Y, Z` 有限 DAG 环检测（真判 `E-AXIOM-CYCLE`，
  不是弃权）；`load:` 拓扑排序；换一条公理自动算受影响的定义集。
- L2（更远）：`translation:` + `preserves:` 升函子性证书校验；
  `strength_le:` / `interprets:` 接解释证书链。**不取** `Con(T)` 判真。

**已知欠账（小，收尾用，非方向）**：`--json --stdin | jq` 题面 6 行仍混 stdout
（`--paper=PATH` 或 `tail -1` 可绕）；重搬守卫只比 STAMP + `ltir_ui.sh` 哈希，
未盖 `src/ltir006.cpp` / `ltir_exam.sh`（MT 覆盖后建议 `rm -rf ~/ltir_lab` 全新重搬）；
`src/ltir006.cpp` 第 7 行注释仍写「速览.md」；`ltir.sh` 第 9 行注释仍写
「Windows 分支冻结，不维护」；`ltir_ui.sh` 的 `LTIR_SMOKE_TOTAL` 默认 387 只是
进度条初始估算，收尾行自适应真实总数（405），无需改输出层。
