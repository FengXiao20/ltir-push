# ltir006 · 0.0.6 强化修复版 实跑记录

> 主版本号 **0.0.6 不变**，后缀「强化修复版」。本轮四件事：修 L06 / 标签体系接回 /
> 领域扩增 / Termux clang++ 真机取证销案。

## 一、平台与退出码

| 项 | 值 |
|---|---|
| 日期 | 2026-11-24（Asia/Shanghai） |
| 本地平台 | Linux x86_64（Ubuntu 22.04.3 容器），`g++ 11.4.0` |
| 本地编译 | `-std=c++17 -O2 -Wall -Wextra`，**0 error / 0 warning** |
| 真机平台 | **Termux（Android）`clang++`** —— 立法者截图回填 |
| 真机结果 | 内核门禁 OK、clang++ 编译成功、44/44、noexec 搬家生效（`~/ltir_lab $`） |
| 本地 SMOKE | `69 / 69 项满足`，exit 0 |
| 一键 `bash ltir.sh` | 门禁 → 编译 → SMOKE → 演示，exit 0 |
| 干净解包复现 | 独立目录解包后 `bash ltir.sh` 全流程 exit 0，69/69 |
| 内核 SHA | `aecc9726e2018c628ba805e2251f56fb5258224ecd4e3feaec5632dab04f2319`（11/11 SAME） |
| core/ | 13 件，与 `v0.0.2/core` 逐个 sha256 SAME，一个字节未写 |
| 0.0.5 速览原件 | `速览-0.0.5-原件.md` sha256 `816d14f962918d21`，5332 B，一字未动 |

## 二、SMOKE 69 条清单（`tests/smoke006.sh`）

| 组 | 条数 | 覆盖 |
|---|---|---|
| 内核门禁 | 1 | `RUN_core_sha.txt` 11 文件 |
| eval 精确求值 | 14 | 有理/LaTeX/三角/`π^0` 折叠/复数/`1/0`/`∞−∞`/M61/`2^64`/组合数/`20!`/`21!` |
| verify 简明档 | 10 | 汇总行、真值自产、中文标点、容器人话、三角、solve、∞、E-INF、台账、退出码 |
| 详细档 / JSON / 语言 | 6 | B4 四行、`^` 指位、码表中文解释、`--lang=en`、JSON 结构 |
| zf/zfc 保留 | 2 | 11 条登记、不进三态统计 |
| 入口防护 | 7 | CRLF / BOM / 全角三份 ×2 + 全角行 `^` 指位 |
| 护栏 | 2 | 裸单字母 `E-UNK-LINE`、硬错 exit 1 |
| 反例 | 2 | `=True` 不产判定记号、词库外判定词报实算真值 |
| 版本 | 1 | `ltir006 0.0.6 强化修复版` |
| **【强化】L06 修复** | 6 | 截图整卷 L06→True、6 True/0 UNDECIDED、行数不变、`设/令/取 x=5` 三种绑定 |
| **【强化】乙 标签并回** | 10 | `<calc>` / 容器内 `V:` / `S:Q:` 不产行 / 装饰登记 / `<answer>` / `<过程>` 归一 / `<C_and_E>` 三字段 / 不产三态 / `tag:answer` / 中文保留 |
| **【强化】domain 扩增** | 7 | 24 领域、`<subject domain>` 进 Ctx、属性记录、`domain: ?`、顶层设定、未知放行、函数定义域未回归 |
| **【强化】clang 复核** | 2 | `den,t` 死变量已清、产物存在 |

```
-----------------------------------------------
69 / 69 项满足
-----------------------------------------------
```

**回归声明**：原 44 条中 **43 条逐字未动全绿**；第 44 条因本令要求版本后缀，
期望值由 `ltir006 0.0.6` 改为 `ltir006 0.0.6 强化修复版`，**断言本身保留**。
demo 整卷仍是 `24 lines: 20 True, 3 False, 1 UNDECIDED`，逐行输出未变。

## 三、L06 修复归因（逐字复现立法者截图）

```
V: √(16) + π^0
<a>
前情提要：这题非常难，但 3^3 == 27 -> True 那么
结论成立。
顺便计算一下 V: 1/3+1/6 = 1/2。

</a>
A: 2+2=4 -> True。
A: 3^2=9 -> True，所以这题选C。
<subject domain="math">
解：设 x=5，代入得 A: x^2 = 25 -> True。
</subject>
```

| | 首版 | 强化修复版 |
|---|---|---|
| L06 | `UNDECIDED  E-SCOPE-TRANSCENDENTAL` | **`True`** |
| 汇总 | `5 True, 0 False, 1 UNDECIDED` | **`6 True, 0 False, 0 UNDECIDED`** |
| 行数 | 6 | **6（不变）** |

根因四步（首版）：① 容器内中文当分隔符；② `bodyClean()` **只返回含 `->` 的一段**，
`设 x=5` 那段被丢；③ 全项目只有 `at:` 写环境，「设」从未翻译成绑定；
④ 未绑定 `x` → 诚实弃权 → UNDECIDED。

修法：容器内「一段」→「多段状态机」`splitSegments()`，新增 `tryBindEnv()` 只认
「单标识符 = 可求值数值」的段（`x^2=25` / `2+2=4` / `1/3=1/2` 左侧非单标识符，不误绑）；
容器开合用 `envStack` 做快照恢复，**「设 x=5」不跨容器泄漏**。
反证：只补 `at: x=5` 时首版也判 True，说明求值器没坏，差的就是绑定。

## 四、简明档样例（`./ltir006 verify fixtures/demo.ltir`）

```
L01 True  R
L02 True  2*x - 3
L03 True
L04 False  E-VAL-MISMATCH
...
L20 True  ∞
L21 UNDECIDED  E-INF
...
── 24 lines: 20 True, 3 False, 1 UNDECIDED ──
registered: 9 (zf/zfc/given/case/check/const/define/seq/set, ledger only)
tags: 5 answer, 0 C_and_E, 7 decoration, 3 preserved
```

`tags:` 行是本版新增，**只在显式用过标签时打印**；顶层 24 行逐字不变。

## 五、详细档样例（`--explain`，B4 四行同形）

```
L04 False  E-VAL-MISMATCH
  A: f'(3) = 4 -> True
           ^
  左边实算为 False，不是 True
```

`--lang=en` 下为 `LHS evaluates to False, not True`。`^` 按显示宽度算列。

## 六、JSON 样例（新字段）

```
$ ./ltir006 verify 卷.ltir --json
{"ok":true,"lines":[{"n":1,"state":"True","code":"","tag":""},
{"n":2,"state":"True","code":"","tag":"answer"},...],
"counts":{"True":6,"False":0,"UNDECIDED":0},"registered":0,
"domain":"math","domains_registered":24,"domains_seen":1,
"decorations":2,"preserved":4,"inferences":0,"answers":4}
```

`registered` 保留原字段位（旧断言仍命中），新字段全部追加在后。

## 七、Termux 真机取证（销案）

立法者两张截图坐实：

- `== 编译（Termux：clang++ 自适应） ==` → `OK: ./ltir006`，**clang++ 编译成功**
- `== SMOKE ==` → `44 / 44 项满足`，noexec 搬家后提示符 `~/ltir_lab $`
- 真机抓到 `src/value.hpp:187` `Rational c2,d2,den,t` 的 `unused variable 't'`
  → **1 warning**。首版报「0 warning」只在 g++ 下成立，这条认；本版删 `,t`，
  g++ 复核 0 warning，并加 SMOKE 第 68 条静态断言防回归。
- 上轮「clang++ 没实跑」报备**销案**，双编译器取证成立。
- 未做：强化修复版的 69 条在 Termux 真机复跑（真机跑的是首版 44 条），
  待立法者回填，不谎称已验。

## 八、改动文件清单（相对 `ltir006/`）

```
改  src/ltir006.cpp       容器栈 / 多段状态机 / 装饰简写 / C_and_E / domain / 版本 / JSON 字段
改  src/value.hpp         删 clang 死变量 t
改  tests/smoke006.sh     44 → 69 条（第 44 条期望值随版本后缀更新）
改  README.md  SYNTAX.md  速览.md  RUNRESULT.md
改  ltir006.sh            搬家清单补 domains.tsv
新  domains.tsv           24 个领域注册表（语法层资产，非码表）
不动 core/ 13 件 / RUN_core_sha.txt / lab_codes.tsv / fixtures/ / 速览-0.0.5-原件.md
排除 ltir006 二进制 / *.o / *.log / __pycache__
```

**码表 `lab_codes.tsv` 一字未动** —— 领域扩增走 `domains.tsv`，不碰 append 权在立法者的码表。

## 九、诚实报备（不遮）

1. **真机 1 warning 已修但只在 g++ 复核**：clang++ 侧无本地环境，靠静态断言 + 删死变量，
   不敢写「clang++ 0 warning」。
2. **强化版 69 条未在 Termux 真机跑**：真机截图是首版 44 条，待回填。
3. **首版越界条款撤销记档**：`SYNTAX.md` 首版写过「明确不用 `S: E: P: Q:`」，
   属施工侧自行否决旧标签体系，立法者从未授权。本版按乙裁决整套并回，
   第二节留撤销记档，不悄悄翻案。
4. **装饰简写整行跳过**：首跑 67/69，根因是把 `S: 解：2x+6=14` 按中文切段后，
   `2x+6=14` 被误当断言判了 UNDECIDED。改为「简写整行机器跳过」后全绿 ——
   这本身就是规范「简写行机器跳过」的要求，首版做错了，不是放松断言。
5. **领域只登记可查，不改判定逻辑**：不同 domain 不切换验真规则，域能差不靠转换硬抹平。
6. **`<C_and_E>` 三字段不产三态行**：只登记进 `inferences`，三态锁死不许加第四态；
   `by:` 凭据不替引擎判真假（铁律 1）。
7. **`A:` 在同一层里既是断言又是 `<answer>` 归一目标**：靠 `tag` 字段标注，
   不是两个 `<answer>` —— 与历史「两个 `<answer>` 不在同一房间」记档不冲突。
8. **`V:` 双身份**：0.0.6 里是 `value:` 求值短写，解题文本层里是「验」装饰。
   本版取「仍求值 + 额外登记装饰身份」，兑现回归零破坏；若立法者要改成纯跳过，
   打回即可（会动 demo 的 L08–L11 等行）。
9. **输出语言口径分歧**照旧：auto 默认 + `--lang=en|zh|auto` 钩子。
10. **0.0.5 原型源码仍未落盘**：`/data/inputs` 23 件里只有 13 件 core + 脚本 + 速览，
    原型 `src/` 从未收到（按 .tools 教训附文件数，不写「疑似」）。有就发来合并，不阻塞。
11. **`localhost:8080`** 是本机回环，服务端够不着，不是程序坏了。

## 十、未尽事项（只列，不做）

1. **P2 主体**：分段函数、参数方程 / 极坐标、解三角形、立体几何、线性回归、
   分布列、复合 LaTeX 的 AST 级折叠
2. **P3 边界**：渲染层标黄放行（`∪∩⊆`）、中文装饰词渲染加粗、浏览器宿主
   （WASI shim + index.html）、中文数学术语词典、`--lang` 全量解释
3. Termux 真机复跑强化版 69 条 → 立法者回填
4. 0.0.5 原型源码若到位后的合并联调
5. `lab_codes.tsv` 的 append 权在立法者，引擎不改码表
6. i18n 多语言报错（1.x 议）、C++ 性能核（预留位，无令不动工）
7. Windows 分支（v0.0.2 / Win7 legacy / `.bat`）**冻结，不维护**
8. v0.0.1 验收仍 1/2 平台（Termux 那一格未回填），与本令无关

## 0.0.7 Linux + Termux（本轮）

- 源码：`src/ltir006.cpp` 增量落盘，core 11 件 SHA `aecc9726…` 零改动；
  `ltir.sh` 动两处（收容清单加 `./axioms`、SMOKE 切 `smoke007.sh`）。
- 编译：本地 g++ 11.4.0，**0 error / 0 warning**。
  Termux clang++ 真机本轮**未跑**（0.0.6 强化修复版的 69/69 真机取证
  不回退），待立法者回填。
- SMOKE：`tests/smoke007.sh`，69 条回归 + V07 组；三角囚徒冻结卷
  `fixtures/triangle_prisoner.ltir` 重跑验收。
- 资源：`lab_codes.tsv` 11 → 35 条码（append 24 行）、`domains.tsv`
  24 → 27 注册、`axioms/` 12 个模块文件。
- Windows 10+：`ltir006_setup.exe` 本轮不重打，真机七项仍待回填。
- 未做（只列）：区间证书 / 符号证明 / 无限集展开 / 嵌套集合精确判定 /
  短写开表 / 码表两套 state 统一 / i18n 全量 / Win 包重打。

## 0.0.7 补丁验收（版本号不变 · Linux / Termux）

**日期**：2026-11-25 · **版本**：`ltir006 0.0.7`（一字未动，无新后缀）

| 项 | 结果 |
|---|---|
| 内核 SHA 门禁 | `OK: 11 文件与取证一致`（core 11 件零改动） |
| 本地编译 | g++ 11.4 · **0 error / 0 warning** |
| SMOKE | **175 / 175**（136 条原样 + 39 条新增） |
| 干净解包复现 | 见交付包内复现记录 |
| 三角囚徒冻结卷 | `10 / 10 True / 0 UNDECIDED`（一位不漂） |
| demo | `24 lines: 20 True, 3 False, 1 UNDECIDED`（逐字不变） |
| 域表 | 27 → **55**（28 个仅台账域） |
| 码表 | 35 → 36 行（append `E-SCOPE-LEDGER-ONLY`） |

### 修复清单（逐字复现用户卷 `test_007_logic.ltir`）
修前 `7 lines: 2 True / 0 False / 5 UNDECIDED`，修后
**`7 lines: 4 True / 1 False / 2 UNDECIDED`**：

1. `V: 2 ∈ {1,2,3}`：`UNDECIDED E-SCOPE-TRANSCENDENTAL` → **True**
2. `V: 2 ∈ ∅`：同上 → **False E-VAL-MISMATCH**（能判就别弃权）
3. `card: {1,2,3}`：`E-REF-UNKNOWN` → **3**（`card: ∅` → 0）
4. `<C_and_E>` 内 `by:`：静默漏记 → **并入 `by_refs`**（`2 checked, 0 unknown`）
5. `A: card({{1},{2}})`：超越域兜底 → **`E-SCOPE-NESTED`**
6. `V: 2 ∈ ∅ -> False`：整串吞掉集合语义 → **先剥声明词**；声明词一致时不残留
   `E-VAL-MISMATCH`（此污染为 0.0.7 首版既有，一并收口）
7. `V:` 短写的 `∪ / ∩ / ⊆ / |A|` 旧输出伪 `1` → 输出精确集合 / 布尔

**归因更正**：上一轮把 L07 猜成「容器内 `by: calc`」是错的。本地逐字复现证实
L07 是 `card: {1, 2, 3}`；容器内 `by:` 的问题是**统计漏记**（`calc` 本就在白名单，
不会产生 `E-REF-UNKNOWN`）。两处都修了，不把误判包装成结论。

### 甲：一个判定核都没加
`domain: analysis` + `A: 2+2==4 -> True` → `UNDECIDED E-SCOPE-LEDGER-ONLY`，
不是 False、不是硬错、exit 0；老域 `trigonometry` 照常判 True，不串台。
老账本归编见 `BACKLOG.md`。

### 交付物
- `termux/termux-file-editor`：随包，标题读 `--version`，三锚点焊死
- `BACKLOG.md`：老挂账 / 新域台账 / 实验区半张图 / 工程项 / 销案记录
- `ltir.sh` 收容清单补 `./termux ./BACKLOG.md ./AXIOM_NOTES.md`

### 诚实报备（不遮）
- **Termux clang++ 真机本补丁未跑**：0.0.7 首版 136/136 真机取证不回退；等回填
- **Win10+ 包本轮不重打**：源码同源，`_WIN32` 分支仅纠正两处残留旧版本串
  （`ltir006.json` 的 version、文件关联描述），`--version` 输出未动；Win 包仍是
  0.0.6 强化修复版，不冒充已同步
- 原 136 条里 **3 条期望随域表更新**（`27 → 55`），其余 133 条逐字未动
- `win/_pack_win.py` 的 payload 清单未加 `BACKLOG.md` / `termux/`，将来重打 Win 包时需补
- 甲只登记，28 个新域**零判定核**；`localhost:8080` 仍是本机回环，够不着


## 0.0.7 补丁二 · 取证（Linux / Termux 侧）

- **225 / 225 项满足**（原 175 条 + 新增 50 条），g++ **0 error / 0 warning**
- 内核 SHA 门禁 `OK: 11 文件与取证一致`；core 11 件一字未动
- 干净解包独立复现：解包 → `bash ltir.sh` → 225/225 → `ltir006 0.0.7`
- 逐字复现两 bug 的修前/修后：
  - `V: 2 ∈ ∅`：修前 `False E-VAL-MISMATCH` → 修后 `False`（无码）
  - `forall: x in {1,2,3}: x^2 > 0`：修前 `UNDECIDED E-SCOPE-INFINITE` → 修后 `True`
  - Naive 卷（`load: Naive.ltir` + `{x | x ∉ x}` + `V: R ∈ R`）：
    修前 `True {UNDECIDED}` / `False E-VAL-MISMATCH` → 修后两行均
    `UNDECIDED E-PARADOX-RUSSELL`，零 True/False
- 回归锁定：三角囚徒冻结卷 `10/10`、demo `24 / 20 True 3 False 1 UNDECIDED`、
  用户卷 `7 / 4 True 1 False 2 UNDECIDED` 全部不漂
- 版本锚点：`--version` / JSON / 二进制串均 `0.0.7`，未推进 0.0.8，未回滚 0.0.6

### 报备
- **Termux clang++ 真机本补丁未跑**（0.0.7 首版 136/136 与补丁版 175/175 真机取证
  不回退），等你回填；本地用 g++ 编译 + 干净解包复现
- **Win10+ 包本轮不重打**；`_WIN32` 分支未改，版本号不动
- 四私货**只落有限集**：连续域 / 无限集 / 嵌套集合 / 区间证书 / 符号证明仍挂在 P2
- `localhost:8080` 还是本机回环，够不着，不是程序坏了

## 0.0.8 交付取证（Linux / Termux 侧）

- 版本推进 `ltir006 0.0.8`；`VERSION` / `LTIR_STAMP` 新增。
- 启动脚本 `ltir.sh`，底片不一致自动重搬；`tests/smoke008.sh` 为入口。
- core 11 件 SHA 门禁仍 `OK: 11 文件与取证一致`，core 零改动。
- 三角囚徒冻结卷 `10 / 10 True / 0 False / 0 UNDECIDED` 不漂；
  demo `24 lines: 20 True, 3 False, 1 UNDECIDED` 逐字未动。
- Win10+ 包未重打；Termux clang++ 真机待回填。
- 0.0.8 一键 `bash ltir.sh`：276 / 276 项满足，g++ -Wall -Wextra 0 warning / 0 error。
- 0.0.8 干净解包独立复现：276 / 276（交付 zip 顶层 ltir008/）。
- 0.0.8 外存收容等价路径模拟：底片缺失 → 自动重搬；底片一致 → 直接入驻；底片改坏 → 自动重搬恢复，三种均 exit 0。

## 0.0.9 交付取证（Linux / Termux 侧）

- 版本推进 `ltir006 0.0.9`；`VERSION` / `LTIR_STAMP` 同步。
- 入口 `tests/smoke009.sh`；core 11 件 SHA 门禁仍 `OK: 11 文件与取证一致`。
- 三角囚徒冻结卷 `10 / 10 True / 0 False / 0 UNDECIDED` 不漂；
  demo `24 lines: 20 True, 3 False, 1 UNDECIDED` 逐字未动。
- 四种输出模式（normal / tri / no-answer / check）与 env 覆盖均有断言。
- Win10+ 包未重打；Termux clang++ 真机待回填。
- 0.0.9 一键 `bash ltir.sh`：347 / 347 项满足，g++ -Wall -Wextra 0 warning / 0 error。
- 0.0.9 主动探针两批共 58 例：隐式乘法 / 受限概括 / 溢出归类 / π/12 家族 / 四种输出模式，未发现新伪判。

---

# English summary

Run records: per-version build evidence and SMOKE results, oldest first. Current
state: full `ltir.sh` pipeline green — SHA gate → build → **SMOKE 347/347** → demo
(0.0.9 lineage: 44 → 69 → 136 → 225 → 347 items, each generation zero breakage of the
previous). Historical lines above may reference `速览-0.0.5-原件.md` (archived, removed
from this repo) and `速览.md` (renamed to `QUICK_TOUR.md` on 2026-11-25) — they were
true when written and are kept verbatim as records.
