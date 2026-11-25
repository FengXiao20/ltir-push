# LTIR

**A math verifier that refuses to bluff.**

You feed it mathematical assertions; it answers line by line with one of three verdicts:

- **`True`** — the engine computed it and it actually holds
- **`False`** — the engine computed it and it doesn't (with a reason code)
- **`UNDECIDED`** — out of scope, overflow, or undetermined form: the engine *honestly abstains*

The key rule: `-> True` in your file is only a **declaration**. The engine washes it out
and computes the value itself. Write `A: 2+2=5 -> True` and it answers `False` — your
claim is not evidence. It never folds "I don't know" into `False`, never passes sampled
points off as a universal proof, and never rules on CH or Goldbach for mathematics.
Everything runs on exact rationals / algebraic numbers / complex numbers — zero floating point.

```
$ ./ltir006 verify paper.ltir
L01  True
L02  False  E-VAL-MISMATCH        (declared True, engine got 4 ≠ 5)
L03  UNDECIDED  E-SCOPE-INFINITE  (quantifier over an infinite set — abstained)
```

## Quick start

```bash
bash ltir.sh                    # gate check -> build -> SMOKE (0.1.0) -> demo
./ltir006 eval "1/3+1/6"        # 1/2
./ltir006 verify paper.ltir     # verify a scroll
./ltir006 check paper.ltir      # verdicts only, suppress answer values
./ltir006 verify paper.ltir --tri      # bare True/False/UNDECIDED per line
./ltir006 verify paper.ltir --explain  # verbose: caret pointer + code-table reason
./ltir006 verify paper.ltir --json     # structured output
./ltir006 --version            # ltir006 0.1.0
```

C++17, standard library only. Linux + Termux (`pkg install clang make`; keep the
project off `/storage/emulated/0` — noexec — the script relocates it to `~/ltir_lab`
and switches to `clang++` automatically).

## The language in one page

A scroll (`.ltir`) is a list of lines. Every line starts with a **keyword** in the
first column; everything after `#` is a comment. Keywords are English, abbreviated
where possible, but **bare single letters are forbidden as keywords** (`A:` is fine;
a lone `A` at line start is a hard error `E-UNK-LINE`). Single letters like `i e π x f R`
are identifiers/constants inside expressions, not keywords — the ban is on keyword
spellings, not on mathematical notation.

### Assertion & evaluation keywords

| Keyword | Short | Example | Result |
|---|---|---|---|
| `assert:` | `A:` | `A: 2+2=5 -> True` | `False E-VAL-MISMATCH` |
| `value:` | `V:` | `V: 1/3+1/6` | `1/2` |
| `ident:` | `id:` | `id: (x+1)^2 == x^2+2x+1` | `True` |
| `numth:` | `N:` | `N: factors(360)` | `2^3 × 3^2 × 5` |
| `at:` | `at:` | `at: x=3, y=2/3` | binds a substitution scope |
| `define:` | — | `define: f(x)=x^2-3x+2` | registers a function |
| `derive:` | — | `derive: f'(x)` | `2*x - 3` |
| `solve:` | — | `solve: x^2-5x+6 < 0` | `x ∈ (2, 3)` |
| `sum:` | — | `sum: n=1..10 of a_n` | `120` |
| `seq:` | — | `seq: a_n = 2n+1` | registers a sequence |
| `limit:` | — | `limit: n->inf of 1/n` | `0` |
| `vector:` | — | `vector: cross([1,0,0],[0,2,0])` | `(0, 0, 2)` |
| `prob:` | — | `prob: C(5,2)*(1/2)^5` | `5/16` |
| `stat:` | — | `stat: mean([1,2,3,4])` | `5/2` |
| `set:` | — | `set: A = {1,2,3}` | `{1, 2, 3}` |
| `domain:` | — | `domain: trigonometry` / `domain: ?` | sets scope / lists domains |
| `const:` | — | `const: phi = (1+sqrt(5))/2` | registers a new symbol |
| `given:` / `case:` / `check:` / `note:`(`#`) / `subject:`(`<s>`) / `zf:` | — | bookkeeping lines | registered, never verdicts |

Registration lines never enter the verdict statistics — they only show up in the
`registered:` count. The verdict set is **locked at three states**; there is no fourth.

### Expressions

```
+ - * /        ^ (right-assoc)      ! (factorial, ≤ 20!)    2x, x(x+1) implicit product
1/2, 0.25      = == != < > <= >=    LaTeX \frac{A}{B}       √ π ∞ × ÷ − ² ³ ≤ ≥ ≠ ⇒
sin/cos/tan with exact special angles (incl. the π/12 family)      complex i (i²=-1)
algebraic numbers a+b√r        ∞ as a sentinel (1/0 → ∞; ∞−∞ → UNDECIDED)
number theory: isprime nextprime factors gcd lcm phi modpow C P
```

Non-special angles (`sin(π/7)`) hit a **Richardson bound**: the engine abstains
instead of guessing — no float interpolation, no fake `True`.

### Structured scrolls (problem/work/answer tags)

```xml
<subject domain="algebra"> … </subject>   tags the problem, domain is queryable
<calc> A: 2+2=4 -> True </calc>           computation block
<C_and_E> cause: … effect: … by: calc </C_and_E>   reasoning block (no verdict lines)
<answer> A: 3^3 == 27 -> True </answer>   archived answer, tag:"answer" in --json
```

Inside a container, plain Chinese prose is allowed and kept verbatim; every ASCII
math segment is fed to the parser, and "设 x=5"-style segments bind the substitution
scope. Decorative shorthands `S:`(solution) `E:`(let) `P:`(proof) `Q:`(QED) are
kept verbatim, machine-skipped.

### Set theory as a map (0.0.7+ ledger layer)

```
system: ZFC                 declare which map you stand on
choice: on|off|dc|cc|bpi    which Choice layer is loaded
axiom: AC, Separation       axioms active on this map
preset: ZFC / load: axioms/X.ltir     load an axiom module
A: ZFC ⊢ CH -> True         → UNDECIDED (independence is a fact, not a gap)
forall: x in {1,2,3}: x > 0 → True     exists:, cex:, inj:/surj:/bij:, card:, set: R = {x ∈ A | P(x)}
```

`⊢` means *relatively derivable on the loaded map* — never an absolute truth stamp.
Large cardinals, multiverse atlases (`world:` `force:` `ground:` `reachable:`),
formalized definitions (`sig:` `rule:` `expand:` ≤ 8 δ-steps) are registered as maps,
never judged as truth.

### Three states, exit codes, and the code table

| Verdict | Meaning | Exit |
|---|---|---|
| `True` | computed and consistent | 0 |
| `False` | computed and inconsistent (reason code attached) | 0 |
| `UNDECIDED` | out of scope / overflow / open — honest abstention, **never merged into False** | 0 |
| `ERROR` | hard syntax violation (`E-UNK-LINE`, `E-SYN`, …) | **1** |

Reason codes live in `lab_codes.tsv` — an append-only table owned by the legislator,
read-only for the engine. Input hygiene runs first: CRLF→LF, BOM strip, full-width→
half-width, math Unicode→ASCII. Ellipsis spam (`......`) collapses to a single `...`
with one tsundere remark ("N-not like I did it for you or anything!") — that's an
easter egg, listed separately in `--json`.

## What LTIR will never do

No fake verdicts on transcendental identities, no universal claims over infinite
domains, no theorem prover posing as an arithmetic unit, no folding `UNDECIDED`
into `False`. Abstention is a feature, not a bug.

## Repository map

| Path | Content |
|---|---|
| `src/` | 0.0.6+ language layer (C++17, single binary `ltir006`) |
| `core/` | the eleven frozen ℚ-domain kernel files (SHA-gated, untouched) |
| `axioms/` | loadable axiom modules (`.ltir`, registration-only) |
| `fixtures/` | demo & regression scrolls |
| `tests/` | SMOKE suite — 387/387 green |
| `SYNTAX.md` | full legislation text (Chinese, with English edition appended) |
| `QUICK_TOUR.md` | one-page quick tour (bilingual 中英) |
| `PATCH_009.md` | the implicit-power binding patch |

## Version

**0.1.0** — see `VERSION` (plain text) and `LTIR_STAMP` (build stamp).
Lineage: frozen ℚ kernel (0.0.1–2) → capability extension (0.0.4-lab) → shorthand,
containers, LaTeX, trig, ZFC ledger (0.0.5) → self-contained C++ layer (0.0.6,
reinforced) → set-theory map & quantifiers (0.0.7) → formal defs & multiverse (0.0.8)
→ implicit product, π/12 exact angles, output modes (0.0.9) → exam library,
radix glasses, binding environment, consistency-strength ledger (0.1.0, 387 SMOKE). MIT licensed.

## 0.1.0 at a glance

```bash
bash ltir.sh                       # gate -> build -> SMOKE 387/387 -> demo
bash ltir.sh exam --list           # six difficulty folders
bash ltir.sh exam --json --no-clock --human=n --seed=42   # agent-friendly, one JSON receipt
```

- **Exam library** `exams/`: six difficulty folders, `exam.json`, `submit:` hand-in,
  pre-exam notice on stderr, delayed feedback, structure-based scoring,
  deterministic seeded draw. Runner `ltir_exam.sh` is shell-only — the kernel is untouched.
- **Radix glasses**: `radix: 2`, `<2_nary>`, `<3_nary>`, `<二进制>`, `<base=16>`.
  Only the *reading* of numeric literals changes (`0.1` in base 3 is `1/3`);
  everything stays exact rationals, zero floating point. Invalid digit → `E-LITERAL-BASE`,
  unclosed container → `E-SCOPE-UNCLOSED`.
- **Binding environment**: top-level `a=1+1` binds, `==` still judges;
  redefinition → `E-BIND-REDEFINED`, never silently overwritten.
- **L0 ledgers**: consistency strength, cardinality, proper classes, category theory.
  `con: ZFC -> ZFC` is blocked by a structural Gödel-II guard; a handwritten `-> True`
  never supplies truth; missing certificate → `UNDECIDED`, never folded into `False`.
- Relocation directory is now `~/ltir_lab` (version-decoupled). Project name,
  executable name and `core/` SHA gate are unchanged.
