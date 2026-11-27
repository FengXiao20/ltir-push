# 强度草案 · 本地试验版（不是发布版）

基底：**2026-11-27 最新 main**（含 B1–B7 / S 批修复、`regress_bugfix.sh` 81 条、
SMOKE 445/445）。本包只做试验，**不推仓库、不打 tag、不改 VERSION**，
`core/` 十一件与 `RUN_core_sha.txt` 一字未动，码表 `lab_codes.tsv` 未 append。

`base:` 已被进制眼镜占用，故相对基理论写作 `con_base:`（试验名，可改）。

## 一、包内 5 个文件（相对仓库根目录同名覆盖）

| 文件 | 性质 |
|---|---|
| `src/ltir006.cpp` | 指纹 v2 口径 + 6 个试验词条 + JSON/人话两处摘要 |
| `ltir_exam.sh` | CERTS 正则加 5 个展示标签（核心操作打分） |
| `fixtures/strength_trial.ltir` | 试验卷：PA / ε₀ / Gentzen 证书链，0 三态 |
| `tests/trial_strength.sh` | 12 条试验自检（不进 `ltir.sh`） |
| `PATCH_STRENGTH_TRIAL.md` | 本教程 |

## 二、这版试什么

**① 剔公理名（你那句金句落地）**
`sig_mode: formula` 下 `computeTheorySig` 只喂公式本体：
`Sep = φ` 与 `Q = φ` 同指纹；默认 `named` 逐字保留 0.1.1 输出，零回归。
v2 下改一字公式仍变，语言签名与推导规则仍入 hash。

**② 展示型值标签（跟风学术，不自算值）**
`ordinal:` / `benchmark:` / `metatheory:` / `notation:` / `con_base:`
只进 ledger，**不进指纹、不产三态、不产标量**。`ε₀` 待遇照抄 `(2)√2`：
能显示、不参与比较。`strength_le:` 不进分（写一行比较不白拿分）。

**③ 核心操作打分**
计分器只数操作行、不解析内容：链齐（≥4 条）给分，
旧答案卷 `exams/keys/injection-check.ltir` 仍 100/100，分数不漂。
分数与三态分道：满分不代表判了 True。

**明确不做**：给 `Con(T)` 判真、自算强度标量、自动线性化、
τ 保真校验、跨图搬真值、码表新增 `E-STRENGTH-*` 三行（立法权仍在你）。

## 三、实测（干净快照真编译真跑）

```
core 11/11 · 编译 4/4 · SMOKE 445/445 · regress_bugfix 81/81
demo 24 lines 20 True / 3 False / 1 UNDECIDED（冻结基线一致）
trial_strength 12/12 · bash -n 6 件全过
CI=1 stdout 0 字节 · ./ltir006 --version → ltir006 0.1.1
试验卷 --tri stdout 无三态词 · --json 带 sig_mode / strength_labels
ltir_ui.sh 未动（\b 帧宽 25、零 \r、零 \033[K 口径不变）
```

## 四、怎么装

### git（本地分支，别推 main）
```bash
cd ltir-push
unzip -o ltir_0.1.1_强度草案试验_需替换文件.zip -d .
bash ./ltir.sh          # 门禁 + 编译 + SMOKE + demo
bash tests/trial_strength.sh
```

### MT
解压到 `ltir_0.1.1/ltir-push/` 根目录同名覆盖，然后**清一次收容室**
（重搬守卫只比 STAMP + `ltir_ui.sh`，不盖 `src/ltir006.cpp`）：

```bash
cd "$HOME" && rm -rf "$HOME/ltir_lab"
cd /storage/emulated/0/LTIR/ltir_0.1.1/ltir-push/ && bash ./ltir.sh
```

## 五、回退

删 `fixtures/strength_trial.ltir`、`tests/trial_strength.sh`、本教程，
用最新 main 的 `src/ltir006.cpp` / `ltir_exam.sh` 覆盖回去即可。
其余文件互不影响。

## 六、等点头才动的

- 码表 append：`E-STRENGTH-NOCERT` / `E-STRENGTH-INCOMPARABLE` / `E-STRENGTH-CYCLE`
- 证书边 + 有限 DAG 传递闭包（不可比显式留空，绝不并 False）
- `fixtures/godel_ii.ltir` 定理卷与 Gödel II 独立码
- `translation:` / `preserves:` 升 L2 函子性证书校验
- 正式条文写入 `SYNTAX.md`（本试验词条**尚未立法**）
