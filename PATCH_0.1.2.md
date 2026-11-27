# PATCH 0.1.2 — 修 bug（第一份替换包）

> 顺序：**先应用本包，再应用 `ltir_0.2.0_元编程_需替换文件.zip`**。
> 两份共用 `src/ltir006.cpp`，第二份是**叠加版**，单独用会缺 B8/B9/B10。
> `VERSION` / `LTIR_STAMP` **原地 0.1.1**，版本推进待你一句话；不发第二份发行版。

## 需替换文件（相对仓库根，同名覆盖）

| 文件 | 内容 |
|---|---|
| `src/ltir006.cpp` | B8 Δ<0 恒号分支；B10 大写 E 整体进 NUM |
| `src/value.hpp` | B9 RAT/RAT 直除快路径（不在 core 十一件内，无需重签底片） |
| `tests/regress_bugfix.sh` | 追加 B8/B9/B10 三组共 20 条断言 |
| `fixtures/history_gaps.ltir` | 新增历史卷 v1（0 三态） |
| `PATCH_0.1.2.md` | 本教程 |

## 三条内置规则（正是 0.2.0 分派表的默认行）

- `ltir_quad_sign`：`Δ<0` 无需开方，A>0 恒正 / A<0 恒负 → `R` / `∅`
- `ltir_rat_div`：不再绕 `y.a*y.a` 有理化，`1/1.1E17` 精确出分数（大写 E 科学计数整体进 NUM；小写 e 是欧拉数，`1/1.1e17` 弃权——原稿笔误）
- `ltir_sci_number`：只认大写 `E`；小写 `e` 仍是欧拉数 `SYM`，不冒充科学计数

## 应用

```bash
cd /path/to/ltir-push
unzip -o ltir_0.1.2_修bug_需替换文件.zip -d /tmp/p012
cp -r /tmp/p012/files/. .
rm -rf "$HOME/ltir_lab"          # 守卫不盖 src/ltir006.cpp，必清
bash ./ltir.sh
```

## 回退

把 `src/ltir006.cpp` / `src/value.hpp` / `tests/regress_bugfix.sh` 从上一版快照取回，
删 `fixtures/history_gaps.ltir`；三条修复互相独立，可单条撤。

## 如实报

- `A.p==0` 一次不等式仍 `E-SCOPE-SYMBOLIC`（`x+1>=0` 未修），留 BACKLOG，未夹带
- `Δ>0` 非完全平方（`x^2-2>=0`）仍弃权，端点要 √2 展示，诚实合理
- 中间量真溢出（如 `1.1E17 * 1.1E17`）仍 `E-OVF` 弃权，护栏不撤
- 码表未 append、新错误码未立、`core/` 十一件与 `RUN_core_sha.txt` 一字未动

## 验收回执（干净快照覆盖重放，实测）

```
编译  g++ -std=c++17 -O2 -Wall -Wextra -I core/include  → OK
门禁  RUN_core_sha.txt 逐文件核对 11/11 · bad=[]
SMOKE 445 / 445   regress 104 / 104（B8/B9/B10 三组共 23 条）
demo  24 lines: 20 True, 3 False, 1 UNDECIDED（冻结基线一致）
selftest 6/6    CI=1 → stdout 0 字节、零 \b    bash -n 全 sh OK
--version ltir006 0.1.1   VERSION / LTIR_STAMP 原地
fixtures/history_gaps.ltir → 0 lines: 0 True, 0 False, 0 UNDECIDED
```
