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
三角囚徒冻结卷 `10 / 10 True / 0 UNDECIDED` · 136 条回归 · `./ltir006` / `~/ltir006_lab` / hook 三锚点不动 ·
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
不变、无新后缀 · `./ltir006` / `~/ltir006_lab` / hook 三锚点不动。

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
