# LTIR 0.1.1 · bugfix 增量补丁（版本号不变）

本补丁只修缺陷、补回归、对齐文档。**`VERSION` / `LTIR_STAMP` 仍为 `0.1.1`**，
不推 0.1.2、不打 tag、不带 beta/RC/experimental 字样。
发给用户的永远是正式版；本包是正式版内的差量替换文件。

## 一、包内文件（相对仓库根目录同名覆盖）

| 文件 | 改动 |
|---|---|
| `src/ltir006.cpp` | 隐式乘函数名截断、台账域纯算术豁免、非布尔断言语、进制短闭合 |
| `src/value.hpp` | `0/0` 未定形弃权 |
| `ltir_exam.sh` | 计分认 `inj:` / `surj:` / `bij:` 为结构证书 |
| `ltir_ui.sh` | SMOKE 默认总数 387 → 420 |
| `tests/smoke009.sh` | 台账域断言同步 + 新增 15 条 bugfix 回归 |
| `README.md` / `QUICK_TOUR.md` / `SYNTAX.md` | 版本文案、`card:` 记号、进制闭合、台账域条文 |
| `PATCH_BUGFIX.md` | 本教程 |

`core/` 十一件、`RUN_core_sha.txt`、`lab_codes.tsv`、`domains.tsv`、
`exam.json`、`exams/`、`fixtures/` **一字不动**；不新增错误码。

## 二、修了什么（逐条实测）

### P0 · 假真值级（最要命的一个）

`Parser::mul()` 早先写着「遇 `FUNCS` 里的函数名就 `break`」，本意是别把
`sin` 当变量乘，结果把剩余 token 全留在流里 → 静默截断成前导因子：

```
修复前  2sin(pi/6) -> 2        2sqrt(2) -> 2        2sqrt(2)+1 -> 2
修复后  2sin(pi/6) -> 1        2sqrt(2) -> (2)√2    2sqrt(2)+1 -> 1 + (2)√2
```

同一处同时造假 True（根式被洗成有理数，绕过「RAD2 不参与四则」）
和假 False（`A: 2sin(pi/6)==1 -> True` 被判错）。修法是把 `break` 去掉，
让函数名由 `primary()` 正常吃括号并 `call()`。显式 `*` 路径不变。

`divv()` 里 `0/0`、`(1-1)/(1-1)` 原按 `1/0` 处理成 `∞`；现与
`∞−∞`、`0*∞` 同档 → `UNDECIDED`。`1/0 → ∞` 保持。

`A: 5 -> True` 的报错原写「LHS evaluates to 5, not True」（拿数值跟
声明词比），改为「LHS evaluated to 5 (non-boolean); cannot compare
against a verdict token」。仍判 False，不挂新码。

### P1 · 行为不一致

- **台账域作用域外溢（⚠️ 行为变更，见下节）**：`domain: topology` 下
  `A: 1+1==2 -> True` 原变 `UNDECIDED E-SCOPE-LEDGER-ONLY`。现对 `A:`/`V:`
  剥离声明词后**不含任何字母**的纯有理算术照常精确判；含标识符/函数名
  （如 `V: sin(pi/6)`）仍弃权，门禁没撤。
- **试卷计分**：`ltir_exam.sh` 的 `CERTS` 只认 `by:/rule:/cex:/...`，
  标准答案卷 `exams/keys/injection-check.ltir` 全对只给 70/100。现把
  `inj:/surj:/bij:` 计入结构证书 → **100/100**。

### P2 · 文档对不上

- `SYNTAX.md` 原写 ``card(A)` / `|A|` 精确判有限集势``，实测只有
  `card: A` 有效、`card: |A|` → `E-REF-UNKNOWN`。改文档，不改实现
  （`|` 在 `{x | ...}` 已有语义，不拿来当势标签）。
- `<base=16>` 配 `</base>` 原报 `E-SCOPE-UNCLOSED`。现兼容短闭合
  `</base>` / `</nary>` / `</radix>` / `</进制>`；同名闭合仍有效。
  `SYNTAX.md` 补了写法。
- README / QUICK_TOUR 首屏 `ltir006 0.1.0`、`387 SMOKE` 已对齐
  `0.1.1`，示例里的写死 SMOKE 数字去掉（以后加断言不再过时）。
  **历史条文与 `RELEASE_0.1.0.md` 等归档一字未动**，没有全局替换。

## 三、⚠️ 唯一的行为变更与回退

「仅台账域整段弃权」原本是 `SYNTAX.md`「0.0.7 补丁 · 甲条文」的明文立法
（「能算 `2+2==4` 不等于懂 analysis」），405 条里有断言锁着。本补丁把
它**细化**为「纯算术豁免，域专有表达式仍弃权」，并同步改了条文与那条
SMOKE 断言。

若要保留原设计，两步即可回退：

1. `src/ltir006.cpp` 的 `JUDGE` 门禁里删掉
   `&& !((key=="assert:"||key=="value:") && pureArith(body))`
   （连同上面的 `pureArith` lambda）
2. `tests/smoke009.sh` 把 `台账域纯算术不外溢` 改回
   `ckc "仅台账域：有码弃权" "$LE" 'E-SCOPE-LEDGER-ONLY'`，
   并删掉 `台账域域专有式仍弃权` 一条

其余修复互不影响，可单独保留。

## 四、怎么装

### git

```bash
cd ltir-push
unzip -o ltir_0.1.1_bugfix_需替换文件.zip
git add src/ltir006.cpp src/value.hpp ltir_exam.sh ltir_ui.sh \
        tests/smoke009.sh README.md QUICK_TOUR.md SYNTAX.md PATCH_BUGFIX.md
git commit -m "0.1.1 bugfix: implicit-product truncation, 0/0 abstain, ledger scope, radix closer, exam scoring"
git push
```

### MT（Android）

1. 解压到仓库根目录，**同名全选覆盖**；`src/`、`tests/` 是包内自带路径，别再套一层
2. 不用改权限、不用动 `VERSION` / `LTIR_STAMP`
3. **必须清一次收容室**：重搬守卫只比 STAMP + `ltir_ui.sh`，不覆盖
   `src/ltir006.cpp`，不清会继续跑旧二进制：

```bash
cd "$HOME" && rm -rf "$HOME/ltir_lab"
cd /storage/emulated/0/LTIR/ltir_0.1.0/ltir-push/ && bash ./ltir.sh
```

## 五、验收（干净快照重放结果）

```
门禁 11/11 · 编译 4/4 · OK, 0 failed · 420 / 420 · rc=0
demo 24 lines: 20 True, 3 False, 1 UNDECIDED（冻结基线一致）
bash -n 全部 sh 通过 · ltir_ui.sh selftest 全绿
CI=1 bash ./ltir.sh  → stdout 0 字节、零 \b
PTY 下 \b 帧恒 25 列、孤立 \r=0、\033[K=0
./ltir006 --version   → ltir006 0.1.1
计分回归：bash ltir_exam.sh exams/keys/injection-check.ltir \
          --json --no-clock --human=n --seed=42  →  "score":100
```

一格如实说明：标准答案卷的 `verdicts` 仍含 `UNDECIDED` / `False`
（证书行不产理想三态），打分器按**证书结构**给分、不按 `verdicts` 累加，
这是两层的口径，本补丁不合并它们。

## 六、没夹带（仍记 BACKLOG）

`sqrt(-1) → i√|x|`、复数比较放开、`--json --stdin | jq` 题面混 stdout、
结构化测试事件 / 模式优先级表 / 完整日志保留、局部理论块、δ 展开松绑、
公理依赖图、函子性证书、塌图兜底、证书接收档、Win 重打。
