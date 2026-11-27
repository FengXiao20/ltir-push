# PATCH_REVIEW_FIX —— 外部实测六 bug 修复（0.1.1 增量）

来源：2026-11-27 上手测试（约 80 组黑盒用例）。修复纪律：TDD，先写 `tests/regress_bugfix.sh`
51 条断言看其失败（RED 14/51），再最小修复至全绿（GREEN 51/51），官方 SMOKE 445/445 与
demo 冻结基线零漂移，旧套件（006/007/008）失败项与修复前基线逐条一致（均为过时断言，无新增破坏）。

## B1 limit: 死代码复活 + 处理器重写（src/ltir006.cpp）

- 范畴论 REG 登记集合曾把**所有** `limit:` 行吞成登记，分析极限分支永不可达。
  现按形态分流：`limit: var->target of expr`（同时含 ` of ` 与 `->`）走分析判定；
  其余 `limit:`（如范畴论 `limit: F`）仍只登记。关键词表去除重复的 `"limit:"`。
- 旧处理器三处数学错误一并重写：
  1. 只取 n^1 系数 → 现按极限变量真实最高次比较（分子高次 → ±∞，同次 → 首项系数比，分母高次 → 0）；
  2. 忽略趋向点 → 现解析 `var->target`；有限趋向点精确代入，分母为 0 且分子为 0 归
     `E-INDETERMINATE`（0/0 不定式），分母 0 分子非 0（单侧符号分析超纲）诚实弃权；
  3. 无除号时把表达式本身当极限值 → 现按多项式正常处理（常数 → 自身，n → ∞）。
- 含其他自由变量（如 `(n+k)/(n+1)`）→ `E-SCOPE-SYMBOLIC` 弃权，不猜。

## B2 radix:/base: 指令参数恒按十进制读（src/ltir006.cpp）

指令行自身曾先过 radixNorm：b 进制下 "10" 恒等于 b，`radix: 10` 永远切不回十进制，
3 进制下 `radix: 16` 还会误报 E-LITERAL-BASE。现指令行绕过进制归一（指令是元数据不是数据）。

## B3 solve: 重根符号分析（src/ltir006.cpp）

判别式=0 时按 A(x-r)² 符号分析给解集，不再拿空开区间 (r,r) 冒充解集：
- `x^2<0` → `x ∈ ∅`（曾伪判 True + (0,0)）；`x^2<=0` → `x = 0`；`x^2>=0` → `x ∈ R`；
- 负首项系数对称处理；`x^2=0` 单根不重复罗列；
- 非重根情形逐字不变（回归锁定）。非重根 `<=`/`>=` 端点闭合见 B7（追加修复）。

## B4 递归深度护栏（src/ltir006.cpp）

Parser::expr()/postfix() 与 PolyParser::prim() 共用 RAII 深度计数，上限 1024 层，
超限置 ok=false → UNDECIDED 诚实弃权。5000 层嵌套括号曾段错误（exit 139），现正常弃权；
300 层正常算式不受影响。

## B5 inj:/surj:/bij: 声明洗涤（src/ltir006.cpp）

有限映射带手写 `-> True/False` 曾一律误挂 E-SCOPE-INFINITE。现与 A: 同权：
findVerdict 洗涤声明后照算，冲突挂 E-VAL-MISMATCH；E-SCOPE-INFINITE 只留给
R/N/Z/Q 与 `R -> R` 等真无限域形态。考试计分（声明扣分）不受影响。

## B6 exam --json 契约（ltir_exam.sh）

- json/ndjson 模式题面改走 stderr，stdout 只留唯一一条 JSON 回执（README「one JSON receipt」成立）；
- 未交卷（EOF）时机模式给 JSON 错误回执（`"submit":false,"error":"E-EXAM-UNSUBMITTED"`），exit 3 不变；
- default/tri 模式行为逐字不变。

## B7 solve: 非重根 <=/>= 端点闭合（src/ltir006.cpp，追加修复）

非严格不等号的解集含端点：`x^2<=1` → `x ∈ [-1, 1]`（曾输出开区间 (-1,1) 丢端点），
`x^2>=1` → `x ∈ (-∞, -1] ∪ [1, ∞)`；负首项系数对称。严格 `</>` 开区间逐字不变，
重根分支（B3）不受影响。回归断言扩至 59 条。

## 附带

- README 仓库地图失效引用 `PATCH_009.md` → `PATCH_A.md`。
- 新增 `tests/regress_bugfix.sh`（51 条，B1–B6 全覆盖，含回归锁定断言）。
