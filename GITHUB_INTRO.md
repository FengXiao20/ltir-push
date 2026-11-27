# LTIR · 对外介绍文案（多版任选）

> 用法：挑一版贴到 GitHub README / Release note / 朋友圈；A 版可直接当 README 正文。
> 许可证由仓库所有者决定（建议 MIT），本文案不代为指定。

---

## A 版 · README 正文（中文，推荐）

**LTIR — 一台不会装懂的精确计算器 + 判定器**

它既是计算器（`./ltir006 eval "1/3+1/6"` → `1/2`，精确有理、零浮点），
也是判定器：只判三种状态 `True` / `False` / `UNDECIDED`。

你写 `2+2==5 -> True`，它照样回 `False` —— **声明不算证据，真值得自己算**。
算不了就老实弃权：不把「不知道」凑成否，不拿一堆采样点冒充全称证明，
不替数学界给 CH 或哥德巴赫拍板。全程精确有理数与代数数，零浮点。

再往上，集合论被做成一张地图：ZFC、朴素集合论、大基数、多宇宙图册，
用 `⊢` 标「我在哪张图」，用 `preset:` / `load:` 加载公理模块。
`0.0.9` 起支持形式化定义（`sig:` / `rule:` / 有限步 δ 展开）、
受限概括、`∀/∃`、反例登记册 `cex:` 与有限集精确运算。

最可爱的是多打了省略号，它会默默补全，然后甩一句「才不是帮你呢。」

C++17 单机、纯标准库，Linux / Termux 可跑；`bash ltir.sh` 一键
门禁 → 编译 → 自测 → 演示，当前自测 **全绿**。

```bash
bash ltir.sh                 # 一键全流程
./ltir006 verify 卷.ltir     # 判卷
./ltir006 check 卷.ltir      # 只验算，不输出答案
./ltir006 verify 卷.ltir --tri   # 只输出三态词
```

---

## B 版 · README（English）

**LTIR — an exact calculator and a verifier that refuses to bluff**

Calculator: `./ltir006 eval "1/3+1/6"` → `1/2` (exact rationals, zero floating point).
Verifier: three verdicts only — `True`, `False`, `UNDECIDED`.
`2+2==5 -> True` still returns `False`: **a claim is not evidence**;
truth must be computed. When it cannot compute, it abstains honestly —
never collapsing "unknown" into `False`, never promoting finite samples
into a universal proof, never adjudicating CH or Goldbach on behalf of
mathematics.

Exact rationals and algebraic numbers throughout, **zero floating point**.
Set theory is a map: ZFC, Naive, large cardinals, the set-theoretic
multiverse — `⊢` records *which map you are on*, `preset:` / `load:`
bring in axiom modules. `0.0.9` adds formal definitions (`sig:`, `rule:`,
bounded δ-expansion), restricted comprehension, `∀/∃`, counter-example
ledgers and exact finite-set operations.

Type too many dots? It silently fixes them and mutters
*"N-not like I did it for you or anything!"*

Single-binary C++17, stdlib only. Linux / Termux: `bash ltir.sh`.

---

## C 版 · 一句话 / 电梯版

> 一台宁可说「不知道」也不肯装懂的精确计算器 + 判定器：三态判定、集合论地图、
> 形式化定义，还会傲娇地帮你补省略号。

备选短句：

- 声明不算证据，真值得自己算；算不了就诚实弃权。
- 不把 UNDECIDED 凑成 False，是这台机器唯一的固执。
- 给集合论画地图，给证明留法庭，给省略号留一句嘴硬。

---

## D 版 · 极客硬核版

```
LTIR 0.2.0
  verdicts      True | False | UNDECIDED        (UNDECIDED never folds into False)
  truth         self-computed; "-> True" is a claim, not a certificate
  arithmetic    ℚ / RAD / RAD2 / CPX, exact; Richardson-style abstain
  sets          ∪ ∩ ⊆ ⊂ ∈ card A, restricted comprehension, ∀/∃, cex:, inj/surj/bij
  maps          ⊢ coordinates, system:/choice:/axiom:, preset:/load: modules
  foundations   CH independent · Goldbach open · Gödel incomplete · multiverse ledger-only
  definitions   sig: type: where: rule: grammar: expand: (≤8-step δ)
  I/O           --tri (three words only) · --no-answer · check · LTIR_OUTPUT=tri
  engineering   VERSION · LTIR_STAMP (auto re-stage) · ltir.sh · uninstall.sh
  core          11 files SHA-gated, untouched; self-test all green
```

---

## E 版 · 玩梗 / 社交版

> 写 `2+2==5 -> True` 它回我 `False`。
> 写 `∀ x ∈ R, x²>0` 它回我 `UNDECIDED`，还附一句「采样点不是全称证明」。
> 写罗素集它不判真假，只说「这张地图塌了」。
> 我多打了几个省略号，它给我补齐，然后：「才不是帮你呢。」
>
> 一个 C++ 写的、脾气很倔的精确计算器 + 验算机。开源了。

---

## 附：贴仓库时的小抄

- **Badges 建议**：`C++17` · `License (自定)` · `Linux / Termux` · `no dependencies`
- **目录速览**：`src/` 语言层 · `core/` 11 件 SHA 门禁内核 ·
  `axioms/` 公理模块 · `fixtures/` 示范卷 · `tests/` 自测 ·
  `termux/termux-file-editor` 文件管理器钩子 · `SYNTAX.md` 语法条文 ·
  `AXIOM_NOTES.md` 学术口径 · `BACKLOG.md` 欠账
- **Release 标题建议**：`LTIR 0.2.0 — 元编程 / 授权弃权码 / 公理本体化指纹 / 进制眼镜 / 赋值环境 / 试卷库 / 卸载`
- **`card:` 只认名字**：`card: A` 才出势，`card: |A|` → `E-REF-UNKNOWN`；
  `|A|` 只是展示记号，不是引擎语法。
- **别写进 README 的**：不要承诺「判定 CH」「证明哥德巴赫」；
  这些在台账里是 `independent` / `open`，只登记不判 —— 这正是项目的立场。
