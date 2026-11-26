# LTIR 0.1.0 · 正式版

> **最新版本为 0.1.1**（bugfix 增量：隐式乘截断 / 0÷0 未定形 / 台账域纯算术豁免 / 进制短闭合 / 输出层收尾）。本文为 0.1.0 发布存档。

**这是正式发布版**：不带 beta / RC / experimental / 试验 标记，不打预发布 tag。
版本 `0.1.0`，`LTIR_STAMP = ltir-0.1.0`，`core/` 十一件 SHA 门禁原样通过。

## 本次收编

- **试卷库 `exams/`**：六档难度目录、六道题、`keys/` 参考答案证书卷、`exam.json`、
  `submit:` 交卷、考前声明、延迟反馈、结构指标打分、seed 可复现抽题。
  `ltir_exam.sh` 全在 shell 层，判定核 `./ltir006` 与 `core/` 一字不动。
- **进制眼镜**：`radix:` / `<2_nary>` / `<3_nary>` / `<二进制>` / `<base=16>`，
  只换数字字面量读法、不换世界；`0.1` 在 base3 是 1/3，全程精确有理数、零浮点。
  非法数字 `E-LITERAL-BASE`、忘闭合 `E-SCOPE-UNCLOSED`。
- **赋值环境表**：顶层裸 `a=1+1` / `b=2+2` 跨行绑定；`=` 只绑定、`==` 只判定；
  重名 `E-BIND-REDEFINED`，不静默覆盖。纯函数式单卷，不引入可变状态。
- **一致性强度 / 势 / 真类 / 范畴论（L0 台账）**：`con:` `equiconsistent:`
  `strength_le:` `interprets:` `translation:` `theory_sig:` `card_le:` `card_eq:`
  `class:` `proper_class:` `cat:` `object:` `arrow:` `compose:` `identity:`
  `commute:` `functor:` `nat:` `preserves:` `limit:` ... 只登记关系与弃权理由。
  手写 `-> True` 只触发校验、不提供真值；无证书 → `UNDECIDED`，**绝不并入 False**；
  `con: T -> T` 由 Gödel II 结构护栏拦下；真类谈势 `E-SCOPE-PROPER-CLASS`。
- **0.0.9 工程四件**：`\b` 动态进度条（默认本地可见、非 TTY 静默）、
  demo 纯检查 / 只吐失败块、`verbose` 后门、时区识别中英文 UI、
  满屏历史分组标签去版本号（推版不改 SMOKE 标题）。
- **收容目录统一为 `~/ltir_lab`**：与版本解绑，项目名 / 可执行名不变。

## 不取（明确边界）

给 `Con(T)` 判 True · 把强度排成全序 / 宣称良序 · 给 `|V|` 比大小 ·
一般极限构造与判定核 · 判证明的数学正确性 · 把 0 分并进三态 · 交卷前回显任何判定 ·
`/tmp` 临时文件（Termux 没有 `/tmp`，统一 `$TMPDIR:-$HOME`）。

## 验收

`bash ltir.sh` → core 11/11 → 编译 → SMOKE 全绿 → demo 24 行冻结基线一致。
`bash ./ltir.sh exam --list` 看题库；`bash ./ltir.sh exam --json --no-clock --human=n` 给 Agent 喂。
