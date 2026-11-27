# LTIR 0.1.1 · 不定式专码增量（需替换文件包）

> 不铸版本号：`VERSION` / `LTIR_STAMP` / `--version` 仍 `0.1.1`，不打 tag、
> 不带 beta / RC / experimental 字样。`core/` 十一件与 `RUN_core_sha.txt` 一字不动。
> 发上去的永远是正式版。

## 这一包做了什么

1. **`E-INDETERMINATE` 独立立码**（`lab_codes.tsv` append 一行，state 3）：
   `0/0`、`(1-1)/(1-1)`、`0/0+1`、`0/0 == 0/0` 不再蹭
   `E-SCOPE-TRANSCENDENTAL`；`1/0 → ∞`、`∞−∞ / 0·∞ → E-INF` 三家互不串台。
2. **∵∴ 因果链**：默认档补一行 `∴`；`--explain` 摊开
   `∵ N / R1 / R2 / R3 / R4 → ∴` 完整链，中英同构。
   `--tri` 只吐三态词、`--json` 加 `indeterminate_form` + `rules` 结构字段，
   形式化展示不污染数据通道。`NaN` 仅为人话兜底词，**不是引擎真值**。
3. **`fixtures/indeterminate.ltir` 定理卷**：只登记四条公理本体 + `theory_sig:` +
   ∵∴ 链，`verify` 输出 `0 lines: 0 True, 0 False, 0 UNDECIDED`。
   **不含交互、不含抽奖、不含计时、不含随机数**。
4. **`define:` 参数名接线**：三处写死 `"x"` 改读真实参数名（缺省回退 `x`）。
   `V: g(3) → 10`、`V: 2g(3) → 20`、`derive: g'(t) → 2*t`；旧 `f(x)` 零回归。
5. **顶层 `by:` 登记 + `--tri` 保护**：keys 卷裸 `verify` 不再 `E-UNK-LINE`；
   `--tri` 遇 `state=3` 的 ERROR 跳过，不把系统错冒充 `UNDECIDED`。
6. **文案归位**：README / GITHUB_INTRO 首屏改成「精确计算器 + 不装懂判定器」双身份；
   `card |A|` 连写抹成 `card A`；`SYNTAX.md` 立条文并 `ref:` 定理卷；
   `BACKLOG.md` 记收编。UI 默认总数 420 → 445。

**没做**（按裁决）：彩蛋 A/B 抽奖整族、`sqrt(-1)` / 复数比较、
Gödel II 独立码与定理卷、局部理论块、δ 展开松绑、公理依赖图、
试卷计分与 keys 卷对齐、结构化测试事件重写、`win/` 重打、0.1.2。

## 文件清单（11 个，相对仓库根目录同名覆盖）

| 文件 | 说明 |
|---|---|
| `src/ltir006.cpp` | 立码触发 / ∵∴ 链 / `by:` / `define` 参数名 / tri+json |
| `src/value.hpp` | `Num.indet` 原因标记 + 传播，0/0 打标 |
| `lab_codes.tsv` | append `E-INDETERMINATE` |
| `tests/smoke009.sh` | +25 条回归，420 → 445 |
| `ltir_ui.sh` | 默认总数 420 → 445 |
| `fixtures/indeterminate.ltir` | **新增**定理卷 |
| `README.md` | 双身份 + eval 计算器示例 |
| `GITHUB_INTRO.md` | 中英各版双身份 + `card` 口径 |
| `SYNTAX.md` | 0.1.1 不定式专码条文 + `ref:` |
| `BACKLOG.md` | 收编记录 |
| `PATCH_INDETERMINATE.md` | 本文 |

不在包内：`VERSION` / `LTIR_STAMP` / `core/` 十一件 / `RUN_core_sha.txt` /
`exam.json` / `exams/` / `ltir.sh` / `ltir_exam.sh` / `termux/` / `uninstall.sh`。

## git 三步（仓库根目录）

```bash
unzip -o ltir_0.1.1_indeterminate_需替换文件.zip
git add src/ltir006.cpp src/value.hpp lab_codes.tsv tests/smoke009.sh ltir_ui.sh \
        fixtures/indeterminate.ltir README.md GITHUB_INTRO.md SYNTAX.md BACKLOG.md \
        PATCH_INDETERMINATE.md
git commit -m "0.1.1: E-INDETERMINATE with causal chain, def params, by: registration"
```

不打 tag、不推进 `VERSION`。

## MT 三步

1. 解压到 `ltir_0.1.1/ltir-push/` **根目录**，`src/` `tests/` `fixtures/` 是自带路径，别再套一层
2. 同名全选「覆盖」，不用改权限、不用改底片
3. **清一次收容室** —— 重搬守卫只比 STAMP + `ltir_ui.sh`，不盖
   `src/ltir006.cpp` / `src/value.hpp`，不清会跑旧二进制：

```bash
cd "$HOME" && rm -rf "$HOME/ltir_lab"
cd /storage/emulated/0/LTIR/ltir_0.1.1/ltir-push/ && bash ./ltir.sh
```

验收：`OK, 0 failed · 445 / 445`、demo `24 lines: 20 True, 3 False, 1 UNDECIDED`、
`./ltir006 --version` → `ltir006 0.1.1`。
中文链：另写一行 `V: 0/0`，加 `--explain --lang=zh` 看完整 ∵∴；
`fixtures/indeterminate.ltir` 裸 verify 应是零三态。

## 回退

整包回退即把 11 个文件还原到上一版（`git checkout -- <文件>` + 删
`fixtures/indeterminate.ltir`）。各改动互不耦合：
只想撤 `by:` 登记，删 `isKeyword` 里的 `"by:"` 与 handle 那三行即可；
只想撤 ∵∴ 链，删输出层两处 `E-INDETERMINATE` 分支即可，立码与判定不受影响。

## 验收（干净快照覆盖重放）

```
门禁 11/11 · 编译 4/4 · OK, 0 failed · 445 / 445 · rc=0
demo 24 lines: 20 True, 3 False, 1 UNDECIDED（冻结基线一致）
selftest 6/6 · CI=1 stdout 0 字节
PTY：帧宽恒 25 列、纯 \b 重绘、孤立 \r=0、\033[K=0、含 100%
./ltir006 --version → ltir006 0.1.1
core 11 件 SHA 与 RUN_core_sha.txt 一致 · 11 个包内文件哈希自洽
```

以上为**干净快照覆盖 11 个文件后、隔离 HOME 的实测**，不是纸面推演。
Termux 真机与 noexec 收容路径待覆盖后回填。
中文链用 `--lang=zh`（`LTIR_LANG` 是 shell 层变量，不直接传给 `ltir006`）。
