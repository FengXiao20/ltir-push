# PATCH · 纯实践收编（0.2.1 r3）

> 立法者口径：「我让你全实现了……我说的是纯实践，还包括你那四项专码」。
> **只落实践，不动发行**：不推 `VERSION` / `LTIR_STAMP`、不改四份发行文案版本字样、
> 不打 tag、不另发第二份发行版、不重出上传提示词。

---

## 1. 补回来了什么

| # | 项 | 之前状态 | 现在 |
|---|---|---|---|
| 1 | 四项专码 `E-SHAPE-UNKNOWN` / `E-REWRITE-ARITY` / `E-REWRITE-CONFLICT` / `E-REWRITE-REDEEM` | 「不批准就删回 65」待批 | **已放权定稿**，65 → 69，regress 硬断言锁住 |
| 2 | 词库外判定词（`-> Maybe` / 乱写尾巴） | r2 因 smoke009 第 43 条收窄成「只禁 UNDECIDED」 | **补回全禁版**：写了 `->` 但认不出合法 True/False → `E-SYN` |
| 3 | 代数复杂性两卷 | 只在沙盒、挂「下次大改动」 | 入仓 `fixtures/` |
| 4 | `BACKLOG` 一行 + `SYNTAX` 两节 + `META_KEYWORDS` 五词 | 挂账 | 全部落盘 |
| 5 | 四行能力 gap + 历史卷 v3 负面 witness | 挂账 | `fixtures/metacapability_gaps.ltir` / `history_gaps.ltir` |

**没碰**：`VERSION`（`0.2.0`）、`LTIR_STAMP`（`ltir-0.2.0`）、README / QUICK_TOUR /
GITHUB_INTRO / EXAM_SCORING 的版本字样、`ltir.sh` / `ltir_ui.sh` / `ltir_exam.sh`、
`core/` 11 件与 `RUN_core_sha.txt`、tag。

---

## 2. 立法同步（一处，必须报）

`tests/smoke009.sh` **第 43 条**期望由
「词库外不判定不报错，报实算真值 → `L01 True`」
改成「词库外判定词一律给码 → `E-SYN`」。
这是改既有立法，不是零回归；条数仍 445，故 `445/445` 是**期望同步后的绿**。
`tests/archive/smoke00{6,7,8}.sh` 是历史归档，按项目口径**不动、不跑**。
合法声明零回归另加三条断言：中文标点尾巴、行尾 `by:`、量词声明。

---

## 3. 两卷实测

```
领域卷  fixtures/algebraic_complexity.ltir
        0 True / 0 False / 0 UNDECIDED · registered 29 · --tri 0 字节
        theory_sig e8734cc43c501f2f · mismatch=0
        只改公理名 → 指纹不变；改公式本体 → 3a7d81a8dceadc6e（变）
        reflect: gaps → symbolic_family, asymptotic_bound, all_fields

实例卷  fixtures/algebraic_complexity_instances.ltir
        5 True：10 / 1 / 5/2i / 6 / 2
        （PERM_2 [[1,2],[3,4]]、单位阵、分数+复数矩阵元、PERM_3 全1 六项、HC_3 全1 两项）
        零浮点；`e` 是欧拉数保留符号，单位阵那组特意改用 u,v,w,z 避撞名

能力 gap 卷 0 三态，reflect 列四个边界 id
历史卷 v3   0 三态，reflect 列六个 id（三段负面 witness 也有出口）
```

`quote:` 内层写纯公式串 `VP != VNP`，不写 `A:`（L3 先验拒收判定词）。
不 eval、不核论文真伪、不查 `sorryAx`、不判 `VP≠VNP`。

---

## 4. 干净基底复验

```
g++ -std=c++17 -O2 -Wall -Wextra -I core/include   → 编译 OK，无警告
门禁 RUN_core_sha.txt                              → 11/11，bad=[]
./ltir006 --version                                → ltir006 0.2.0
lab_codes.tsv ^E- 条目                             → 69（末尾 4 行俱在，历史行未动）
smoke009.sh                                        → 445 / 445（第 43 条期望同步）
regress_bugfix.sh                                  → 127 / 127
trial_meta.sh                                      → 31 / 31
trial_strength.sh                                  → 13 / 13
trial_rewrite.sh                                   → 98 / 98
demo.ltir                                          → 24 lines: 20/3/1
bash -n 全 sh                                      → OK
0 三态卷 --tri                                     → stdout 0 字节
0/0 → E-INDETERMINATE · con: ZFC->ZFC → E-GODEL-II · strength_le → E-STRENGTH-NOCERT
1.1E17 / x ∈ R / 1/110000000000000000               → 三件旧修复全在
```

---

## 5. 应用 / 回退

**应用**（干净 `0.2.0` main 上**单包**覆盖，不必先盖 r2/r1）：

```bash
unzip -o ltir_0.2.1_practice_finalize.zip -d /tmp/rw
cp -r /tmp/rw/files/. .
make
bash tests/trial_rewrite.sh && bash tests/smoke009.sh && bash tests/regress_bugfix.sh
```

真机：`rm -rf "$HOME/ltir_lab"` 后 `bash ./ltir.sh`（守卫不盖 `src/ltir006.cpp`）。

**回退**：删新增卷与回执，用覆盖前的 `src/ltir006.cpp` / `lab_codes.tsv` /
`tests/{smoke009,regress_bugfix,trial_rewrite}.sh` / 三份词表文档还原，`make clean && make`。
⚠️ 四项专码**已放权定稿**，正式工作树不得再把断言改回 `65`；
本地回退仅是试验还原手段。

---

## 6. 边界（如实报，不粉）

- `witness:` 仍只登记，自动比对在脚本层；`reflect: mode` 只自报、不作硬门禁。
- 形状族只认 `NAME([[n,n],[n,n]])`；验的是**合法**不是**正确**，
  `a*d-b*c` 写反时形状全合法、counts 全等，靠 `witness:` 回归兜。
- 宏体被 L3 拒后会连锁 `E-SHAPE-UNKNOWN`（多 1 行 UNDECIDED），是行为不是 bug。
- 上传提示词仍过期（写的是三包覆盖 / `0.1.1`），发 agent 前单独重出。
- 连「哪天转正 / 推版本」也不拍 True，四行 `gap:` 安静躺着。

---

## 7. 文件清单

| 文件 | 性质 |
|---|---|
| `src/ltir006.cpp` | 覆盖：全禁版 guard（2 处） |
| `lab_codes.tsv` | 覆盖：69 行定稿 |
| `META_CODES_CANDIDATE.tsv` | 覆盖：候选 → 已放权定稿 |
| `tests/smoke009.sh` | 覆盖：第 43 条期望同步 |
| `tests/regress_bugfix.sh` | 覆盖：加 2 条专码定稿断言 |
| `tests/trial_rewrite.sh` | 覆盖：a9/a10 期望同步 + 3 条零回归 |
| `fixtures/algebraic_complexity.ltir` | 新增 |
| `fixtures/algebraic_complexity_instances.ltir` | 新增 |
| `fixtures/metacapability_gaps.ltir` | 新增 |
| `fixtures/history_gaps.ltir` | 覆盖：v3 |
| `fixtures/rewrite_*.ltir` | 沿用 r2 四卷 |
| `META_KEYWORDS.md` / `SYNTAX.md` / `BACKLOG.md` | 覆盖：追加节 |
| `PATCH_0.2.1_REMAKE.md` / `PATCH_PRACTICE_FINALIZE.md` | 文档 |

单主干、不推 main、不打 tag、不带 beta；`core/` 11 件一字未动。
