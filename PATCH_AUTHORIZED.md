# PATCH 授权收尾 — 待批总表与裁决

> 放权后一次性结清。**叠加在前两份包之上**：
> `ltir_0.1.2_修bug` → `ltir_0.2.0_元编程` → 本包。
> 前两份不重打；单主干、`VERSION` / `LTIR_STAMP` 仍 `0.1.1`。

## 一、完整待批总表（之前每轮只顺手提一句，从没给过全表）

| # | 待批项 | 我的推荐 | 裁决 |
|---|---|---|---|
| 1 | `E-INDETERMINATE` 独立立码 | 准 | ✅ 已在 0.1.1，销案 |
| 2 | Gödel II 独立错误码 | **准**，判得对但蹭错码 | ✅ `E-GODEL-II` |
| 3 | 强度「缺证书」码 | **准** | ✅ `E-STRENGTH-NOCERT` |
| 4 | 强度「不可比」码 | **准**，不可比 ≠ False | ✅ 立码，本期预留 |
| 5 | 强度「成环」码 | **准** | ✅ 立码，本期预留 |
| 6 | `fixtures/godel_ii.ltir` 定理卷 | **准**，只登记不判 | ✅ 本包 |
| 7 | 元编程三候选码 | **不准**，并回 `E-DEF-SIG` | ❌ 改裁决版文件 |
| 8 | 强度草案转正（剔公理名、DAG） | 不取，另开一轮 | ❌ |
| 9 | `reformulate:` 自动接管 UNDECIDED | 不取，卷面能改判风险最高 | ❌ |
| 10 | 弃权原因码 → gap id 自动暴露 | 不取，留 0.2.1 | ❌ |
| 11 | `__int128` 通分 / 重签 core 底片 | 不取，「算不动就空着」是招牌 | ❌ |
| 12 | Lean 桥接读外部文件 | 不取，不联网不调外部进程 | ❌ |
| 13 | 推进 `VERSION` / `LTIR_STAMP` | 不取，发行事件不夹审批包 | ❌ |
| 14 | 彩蛋整族（A/B 抽奖等） | 已撤，不复活 | ❌ |
| 15 | `E-DEF-SIG` 复用结论 | 维持 | ✅ |

**准做理由**：#2/#3 是「判得对、码蹭错」——和当初 `0/0` 蹭
`E-SCOPE-TRANSCENDENTAL` 一模一样，你亲手立的规矩就该对称执行。
#4/#5 提前立法是因为码表 append-only，等 DAG 落地再补会打乱编号语义。

**不准理由**：#7 复用现成码更省，三条新码会把 68 行膨胀成 71 行却只覆盖
同一类「声明/签名冲突」；#9 一旦让卷面规则自动改判，`2+2==5 -> True`
的护栏就要重写；#11 动 `core/` 十一件收益远小于风险；#13 两份包刚交、
SMOKE / help / JSON / STAMP 全锁 `0.1.1`，改了连锁炸，推版跟推仓库同批做。

## 二、需替换文件

| 文件 | 动作 |
|---|---|
| `lab_codes.tsv` | **append 4 行**（68 → 72 个 `E-` 条目），历史行不动 |
| `src/ltir006.cpp` | 两处换码（叠加版，含前两份全部改动） |
| `tests/smoke009.sh` | 两条断言期望码同步，总数 445 不变 |
| `tests/regress_bugfix.sh` | 追加授权收尾组 12 条 |
| `fixtures/godel_ii.ltir` | 新增定理卷 |
| `META_CODES_CANDIDATE.tsv` | 覆盖成裁决版 |
| `SYNTAX.md` / `BACKLOG.md` | 仅 append 新节 |
| `PATCH_AUTHORIZED.md` | 本表 |

## 三、应用

```bash
cd /path/to/ltir-push
# 先按 PATCH_0.1.2.md 与 PATCH_0.2.0.md 依次覆盖两份
unzip -o ltir_授权收尾_需替换文件.zip -d /tmp/pa
cp -r /tmp/pa/files/. .
rm -rf "$HOME/ltir_lab"
bash ./ltir.sh && bash tests/regress_bugfix.sh
```

## 四、如实报

- `E-STRENGTH-INCOMPARABLE` / `E-STRENGTH-CYCLE` **本期只立码、不挂逻辑**：
  偏序 DAG 与证书边未实现，条文明写「预留」，不假装判得了不可比。
- SMOKE 两条断言的**期望码被改**（`E-SCOPE-CONSISTENCY` → 两个新码），
  所以是「445/445 且断言同步」，不是宣称逐字零回归。
- 定理卷内**不写** `con: T -> T`，否则会产一行弃权；只用 `hint:` 登记。
- `core/` 十一件与 `RUN_core_sha.txt` 逐文件核对无差；无新判定核、
  无浮点、无随机、无交互、无计时；`--tri` / `--json` 契约未动。

## 五、回退

把 `lab_codes.tsv` 尾部四行删掉、`src/ltir006.cpp` 两处换回
`E-SCOPE-CONSISTENCY`、SMOKE 两条断言换回、删 `fixtures/godel_ii.ltir`
与两处文档新节即可。前两份包的修复完全不受影响。

## 六、验收回执（前两份 + 本包，同一份干净快照三重叠加实测）

```
编译  g++ -std=c++17 -O2 -Wall -Wextra -I core/include  → OK
门禁  RUN_core_sha.txt 逐文件核对 11/11 · bad=[]
SMOKE 445 / 445（两条断言期望码同步，总数不变）
regress 117 / 117（授权收尾组 13 条）· trial_meta 24 / 24
selftest 6/6 · bash -n 全 sh OK
CI=1 → stdout 0 字节、零 \b
--tri：indeterminate.ltir stdout 0 字节
--json：meta_gaps/meta_rules/meta_quotes/meta_depth 齐；
        indeterminate.ltir 与 godel_ii.ltir counts 全 0
demo 24 lines: 20 True, 3 False, 1 UNDECIDED（冻结基线一致）
码表 ^E- 条目 59 → 63，历史行未动
--version ltir006 0.1.1 · VERSION / LTIR_STAMP 原地

con: ZFC -> ZFC            → UNDECIDED  E-GODEL-II
strength_le: Q <= PA -> True → UNDECIDED  E-STRENGTH-NOCERT
interprets: ZF in ZFC      → UNDECIDED  E-STRENGTH-NOCERT
V: 0/0                     → UNDECIDED  E-INDETERMINATE（未串台）
V: 1.1E17                  → 110000000000000000（第一份修复仍在）
solve: x^2 + 1 >= 0        → x ∈ R（第一份修复仍在）
```

⚠️ 一格环境说明：本轮回执里 `bash ltir.sh` 全流程在非交互沙盒中未跑完
（停在演示段等待），改用 `./ltir006 verify fixtures/demo.ltir` 直接核对
冻结基线；前两份包的 `ltir.sh` 全流程已在上一轮 `OK, 0 failed · 445 / 445`
跑通过。真机上按第三节命令重跑即可。
