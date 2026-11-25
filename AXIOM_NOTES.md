# 0.0.7 集合论全图 · 学术口径核对留档

本轮只用公开权威来源（CMI / AMS 官方页、SEP、论文书目与机构库），
部分卷期页码未取全文，标「存疑」不编造；学界本就无答案的条目
（Berkeley 一致性、NF 一致性、Ultimate-L 猜想、AD 是否蕴涵 DC_R）
恰好印证「只登记不判真值」站得住。

## 必须修正的三处（已回填进稿）

1. **Woodin 位置**：单个 Woodin 基数弱于超紧；只有「真类多个 Woodin」
   才更强。旧稿把 Woodin 插在强紧之前、两头不靠，已改。
2. **I0 与 Reinhardt / Berkeley 不是同一条线性台阶**：I0 在 ZFC 内，
   躲开 Kunen 上界；Reinhardt 是 `j: V→V`、与 AC 不相容，只在 ZF 无
   选择公理线讨论，Berkeley 更强。旧稿画成一条梯子，已改为分叉。
3. **Levy–Solovay 表述**：「大基数不决定 CH」是该定理的**推论**而非
   定理本体，且它只封杀「真值可被小 forcing 改变」的命题。
   旧稿写成定理断言，已精确化。
   SCH / Silver / Shelah pcf 上界三条均带「强极限」「不可数共尾度」
   限定词，漏掉即成假命题，已补。

## 确认的硬点

- CH 不在 CMI 千禧年七问内；它是**希尔伯特第一题**。七问里只有
  Riemann 与 Hilbert 清单重合，Poincaré 已由 Perelman 证明。
  本版七问保持七问，CH 另开独立命题专档（`independent-of: ZFC`）。
- Gödel 1940（L 中 CH 成立）与 Cohen 1963（forcing 得 ¬CH）是
  **独立于 ZFC** 的两条外部证书；`ZFC ⊢ CH → UNDECIDED` 是数学史实，
  不是引擎能力缺口，正好给三态纪律背书。
- Ultimate-L 仍是开放研究纲领且公理改过版；「V=Ultimate L 蕴涵 CH」
  是条件定理。只登记地图名 + `by:` 证书链，不写伪判。
- 绝对无限 Ω ≠ 真类 V：V 是所有集合的真类，Ω 连势比较都取消。
- ZF 下「任意两基数可比」等价于 AC —— 所以连「能不能直接用大小符号
  比势」都由 `choice:` 图层决定。
- AC 等价族：Zorn、良序定理、Tychonoff、任意两基数可比、满射有右逆、
  每向量空间有基。有限族选择在 ZF 内可证，不算公理。
- `AC ⟹ DC ⟹ AC_ω`，两处严格；BPI（≍UFT，Halpern–Lévy 1971）
  严格弱于 AC；BPI 与 DC/AC_ω **互不可比**；Kinna–Wagner 严格卡在
  AC 与「每集合可线性序」之间。弱选择原则是**偏序**，稿里只给已
  确认的蕴涵边，未定边标 `incomparable`，不画总排序表。
- AD 与 AC 冲突、与 DC 相容；full DC 独立于 ZF+AD；Kechris 1984 只在
  加 `V=L(R)` 时证出 DC，「AD 是否蕴涵 DC_R」至今开放（2025 年仍有
  论文证明各投影层级的决定性都不蕴涵 full DC_R）。登记语只能写
  「AD + DC 相容、AD 蕴涵 DC 未定」。
- MC（多重选择）在 ZF 里等价于 AC、在 ZFA 里不等价 —— 同一句前提换
  底层就变，正是「地图」该管的事。
- Kunen 1971 的证明关键用 AC：`choice: on` 下 Reinhardt 直接
  `E-AXIOM-CONFLICT`（登记元定理，**严禁并入 False**）。

## 本版只登记不判（P2 起实验 C）

无限集自动展开 · 区间/符号证明证书 · 公理一致性判定 · 大基数存在性
判 True · CH/RH/千禧年真假 · 奇异基数共尾性 · NF 一致性 ·
AD ⟹ DC_R · 全局 V 的势 · 省略号猜通项。

**三态锁死 / 真值自产 / Richardson 不伪判 / 码表只读消费不变。**

## 0.0.8 学术口径补记

- **集合论多宇宙观**：Hamkins 2012 主张多个同等真实的集合论宇宙，
  不承认唯一绝对 V；CH 在部分宇宙真、部分假，答案是对其在多宇宙中
  成立／失效方式的知识。Maddy 的 universe view 与 Woodin/Steel 的一元论
  仍在，2024 年访谈研究判「多宇宙观只贴合多元主义实践」——
  故台账记 `status: contested`，不写成学界已接受。
- **图间关系**：forcing extension `V[G]`、ground、所有 ground 之交为 mantle
  （Fuchs–Hamkins–Reitz）。Hamkins–Löwe 把 forcing 扩张当 Kripke 可达关系，
  □=所有扩张、◇=某个扩张，ZFC 可证的有效原则恰为 **S4.2**，`.2` 靠乘积力迫。
- **CH 是 switch**：相对温和的 forcing 即可来回翻且保大基数，
  这是上一稿「大基数不决定 CH」的精确定理本体，不是含糊推论。
- **哥德巴赫**：强（二元）猜想至今未证；弱（三元）由 Helfgott 2013–2014
  无条件证出，方向为强 ⟹ 弱。偶数已验到 4×10¹⁸ 无反例，但有限验证
  不证明全称（与三角囚徒卷「采样不蕴涵全称」同构）。
  它是 `status: open`，**不是** `independent-of: ZFC`。
- 同理，千禧年七问与 P vs NP 都是 open，0.0.8 起不再误挂
  `E-SCOPE-INDEPENDENT`。

## 0.0.9 补记（精确值，不是数值近似）

- π/12 家族进表用的是**双根式精确值**：
  `sin(π/12)=(√6−√2)/4`，`cos(π/12)=(√6+√2)/4`，`tan(π/12)=2−√3`，
  及其 5π/12、7π/12、11π/12、13π/12、17π/12、19π/12、23π/12 的对称角。
  全部由半角/和差公式导出，零浮点、零查表近似。
- `RAD2` 只作精确展示，**不参与四则与比较**（返回 UND 有码弃权），
  避免把两个不同根式的运算伪造成有理数 —— 与 Richardson 弃权同源。
- 非特殊角（如 `sin(π/7)`）仍弃权，不插值、不近似。

---

# English summary

Academic calibration archive for the 0.0.7 set-theory map, sourced from public
authorities (CMI/AMS pages, SEP, paper bibliographies). Items with no scholarly
consensus (Berkeley consistency, NF consistency, Ultimate-L conjecture, AD ⟹ DC_R)
are registered as open — which is itself evidence that "register, never judge" holds.

**Three corrections (backfilled into the text)**: ① a single Woodin cardinal is weaker
than supercompact — only a proper class of Woodins is stronger; ② I0 lives inside ZFC
(dodging Kunen) while Reinhardt/Berkeley belong to the ZF-without-AC line — not one
linear ladder; ③ "large cardinals don't settle CH" is the **corollary** of Levy–Solovay,
not its statement; SCH/Silver/pcf bounds all carry "strong limit / uncountable cofinality"
qualifiers.

**Confirmed hard points**: CH is Hilbert's 1st problem, not among the CMI seven
(Gödel 1940 + Cohen 1963 are the two external certificates of independence —
`ZFC ⊢ CH → UNDECIDED` is a fact of mathematics, not an engine gap). AC ⟹ DC ⟹ AC_ω
(strict); BPI strictly weaker than AC and incomparable with DC; the weak-choice
principles form a partial order — only confirmed implication edges are drawn. AD
conflicts with AC, is consistent with DC; whether AD ⟹ DC_R is open. Under
`choice: on`, Reinhardt hits `E-AXIOM-CONFLICT` (Kunen 1971 uses AC) — registered as
a metatheorem, **never merged into False**.

**0.0.8 addendum**: Hamkins' multiverse view is registered `status: contested` (Maddy's
universe view and Woodin/Steel monism persist); forcing extensions as Kripke
accessibility gives exactly **S4.2** (Hamkins–Löwe); CH is a *switch* (mild forcing
flips it while preserving large cardinals); strong Goldbach is unproven, weak Goldbach
was proved by Helfgott 2013–2014, checked to 4×10¹⁸ — finite checks don't prove
universals, so Goldbach is `status: open`, **not** `independent-of: ZFC` (same for the
millennium problems since 0.0.8).

**0.0.9 addendum**: the π/12 family uses exact dual-radical values
(`sin(π/12)=(√6−√2)/4`, `cos(π/12)=(√6+√2)/4`, `tan(π/12)=2−√3`, symmetric angles),
derived by half-angle/sum-difference formulas — zero floats, zero table lookups.
`RAD2` is display-exact only: it never participates in arithmetic or comparison
(returns a coded UND) so distinct radicals can't be laundered into rationals — same
discipline as the Richardson abstention. Non-special angles (`sin(π/7)`) still abstain.
