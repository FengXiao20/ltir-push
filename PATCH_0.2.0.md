# PATCH 0.2.0 — 元编程能力（第二份替换包）

> **必须先应用第一份 `ltir_0.1.2_修bug_需替换文件.zip`**。本包的
> `src/ltir006.cpp` 是叠加版，含 B8/B9/B10 + 六词 + depth/trace + json 字段。
> `VERSION` / `LTIR_STAMP` 仍 `0.1.1`，版本推进待你一句话；不发第二份发行版。

## 需替换文件

| 文件 | 内容 |
|---|---|
| `src/ltir006.cpp` | 六词入关键词表 / Ctx 容器 / handle 分支 / expand 开关 / json 四字段 |
| `fixtures/history_gaps.ltir` | **覆盖**第一份的 v1 → v2 正式标本卷 |
| `fixtures/metaprogram_trial.ltir` | 新增试验卷（0 三态） |
| `tests/trial_meta.sh` | 新增自检（六词 / 冲突 / depth / trace / 叠加回归） |
| `META_KEYWORDS.md` | 正式关键词表（含落包补记） |
| `NEXT_VERSION_DRAFT.md` | 正式下版本草案（含历史卷与单主干） |
| `META_CODES_CANDIDATE.tsv` | **候选码行，不覆盖 `lab_codes.tsv`** |
| `SYNTAX.md` / `BACKLOG.md` | 仅 append 新节，已有条文一字不改 |
| `PATCH_0.2.0.md` | 本教程 |

## 应用

```bash
cd /path/to/ltir-push
unzip -o ltir_0.2.0_元编程_需替换文件.zip -d /tmp/p020
cp -r /tmp/p020/files/. .
rm -rf "$HOME/ltir_lab"
bash ./ltir.sh
bash tests/trial_meta.sh
```

## 边界（如实报，不假装完成）

- **六词只登记 + 签名/冲突校验**，`reformulate:` 本期**不自动接管**原本 UNDECIDED
  的分支；0.1.2 三条手修仍是内置默认规则。自动接管留 0.2.1。
- 「弃权原因码 → gap id」自动暴露**未做**，`reflect: gaps` 只列已登记 id。
- `E-RULE-SIG` 并回现成 `E-DEF-SIG`；三个候选码只在 `.tsv` 里列着，
  **码表未 append**（立法权在你）。
- `quote:` 不 eval、`grammar:` 仍只收 BNF、`core/` 十一件与
  `RUN_core_sha.txt` 一字未动、无新判定核、无浮点、无随机、无交互、无计时。
- 一次式 `x+1>=0` 仍弃权，未夹带；彩蛋整族不复活。

## 回退

删六词分支 + `expand:` 两行改动即退回 0.1.2；历史卷换回 v1；
`SYNTAX.md` / `BACKLOG.md` 删尾部新节。三条 bug 修复完全不受影响。

## 验收回执（先包1、再包2，同一份快照叠加实测）

```
编译 OK · 门禁 11/11 · SMOKE 445 / 445 · regress 104 / 104
trial_meta 24 / 24 · selftest 6/6 · bash -n 全 sh OK
CI=1 → stdout 0 字节、零 \b
meta_trial --tri stdout 0 字节
--json meta_gaps=1 meta_rules=1 meta_quotes=1 meta_depth=12
--json counts 全 0；indeterminate.ltir counts 全 0（未污染）
hist v2 → 0 lines: 0 True, 0 False, 0 UNDECIDED
ltir.sh → OK, 0 failed · 445 / 445 · demo 24 lines 20/3/1
--version ltir006 0.1.1 · VERSION / LTIR_STAMP 原地
```
