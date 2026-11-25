# 0.1.0 A 增量 · 公理本体化（需替换文件包）

**口径**：正式版补丁，不带 beta/RC/experimental 字样。**不铸版本号** ——
`VERSION` / `LTIR_STAMP` / `--version` / JSON `"version"` 仍 `0.1.0`；
`core/` 十一件与 `RUN_core_sha.txt` 一字未动；短写表冻结；码表零 append。

## 一句话

`theory_sig:` 从「只登记的空壳」升级为按 **lang + 公理公式本体 + rules** 算的
FNV-1a 64 内容指纹。同名 `Sep` 把 `and` 改成 `or`，指纹就变 ——
「一致性强度不是符号给的」在机器侧第一次落地。

## 包内 6 个文件（相对仓库根目录，同名覆盖）

| 文件 | 变化 |
|---|---|
| `src/ltir006.cpp` | `lang:` 新长名；`axiom:` 支持 `Name = <公式>`；`theory_sig:` 算指纹并比对；JSON 四字段；人话档一行摘要 |
| `tests/smoke009.sh` | 新增 18 条，387 → **405 / 405** |
| `SYNTAX.md` | 追加「0.1.0 A 增量 · 公理本体化」条文 |
| `BACKLOG.md` | 记 B/C/D 草图与已知欠账 |
| `fixtures/theory_sig.ltir` | 新增示范卷 |
| `PATCH_A.md` | 本说明 |

`ltir_ui.sh` **未改**：进度条按 SMOKE 收尾行自适应真实总数，
`LTIR_SMOKE_TOTAL=387` 只是初始估算，最终显示 `405 / 405`。

## 新写法

```
lang: L_N = {0, S, +, *}
axiom: Sep = forall x (x in A and P(x))
rule: ModusPonens
theory_sig: ?                          # 只算不比
theory_sig: e34a93f0257ea42d          # 给期望 hex → mismatch=0
```

- `./ltir006 verify fixtures/theory_sig.ltir` → 人话档给指纹摘要
- `./ltir006 verify fixtures/theory_sig.ltir --json | jq .theory_sig`
- 旧 `axiom: AC, Separation` / `preset:` / `load:` 全部逐字不回退；
  模块内 `lang:` 与 `axiom: Name = ...` 同样入册
- 期望值不符 → 只记 `sig_mismatch=1`，**0 True / 0 False / 0 UNDECIDED**，
  不报错、不并 False、exit 0；人话档仍显示实算指纹，不照抄声明值
- `tri` / `no-answer` / `json` 均不被指纹行污染
- 公式内避免半角逗号（逗号是列表分隔符），用中文逗号或分号

## 验证（干净副本真编译真跑）

- core SHA `11 / 11` · `g++ -std=c++17 -O2 -Wall -Wextra` 零警告
- SMOKE `405 / 405`，旧 387 条零回归
- demo `24 lines: 20 True, 3 False, 1 UNDECIDED` 逐字不漂
- 三角囚徒冻结卷 `10 True` 不漂
- `--tri` 零输出、`--json` 结构恒定、`--version ltir006 0.1.0`
- `bash -n` 全部脚本过；6 个文件哈希逐一自洽

## 更新仓库（git 三步）

```bash
unzip -o ltir_0.1.0_A_需替换文件.zip
git add src/ltir006.cpp tests/smoke009.sh SYNTAX.md BACKLOG.md \
        fixtures/theory_sig.ltir PATCH_A.md
git commit -m "Theory content fingerprint: axioms are formulas, not names"
```

## MT 三步（Termux）

1. 解压到 `ltir_0.1.0/ltir-push/` **根目录**，`src/` `tests/` `fixtures/` 是自带路径，别再套一层
2. 同名全选「覆盖」；不用改权限、不用改底片
3. **收容室必须先清一次**（重搬守卫不盖 `src/ltir006.cpp`，不清会跑旧二进制）：

```bash
cd "$HOME" && rm -rf "$HOME/ltir_lab"
cd /storage/emulated/0/LTIR/ltir_0.1.0/ltir-push/ && bash ./ltir.sh
```

预期：`OK, 0 failed · 405 / 405`。

## 回退

```bash
git revert HEAD          # 或把 6 个文件换回上一版
cd "$HOME" && rm -rf "$HOME/ltir_lab"
```

## 不夹带

B（`depth:`/`trace:`）、C（局部理论块）、D（公理依赖图）、L2 函子性证书、
塌图兜底、证书接收档、题库新增题目、Win 重打 —— 全在 `BACKLOG.md` 留档等令。
`0.1.1` 只记账，本包不打 tag。
