# PATCH 009b · 幂对隐乘组的绑定修正（只动 src/ltir006.cpp，4 处）

**病灶**：0.0.9 的隐乘补丁把 `xy` 拆成 x·y 返回后，`post()` 的 `^` 对
整个拆分结果取幂 → `xy^2` 解析成 `(xy)^2`。
后果：`id: (x+y)^3 == x^3+3x^2y+3xy^2+y^3` 真·恒等式被判 **False**
（违反"不伪判 False"硬令）。`x^2y` 方向（先幂后连写）不受影响。

**修法**：PolyParser 记录"本次 prim 是否来自拆分组及因子表"，
`^` 命中时只对**最后一个变量**取幂，前缀因子原样保留。

改动清单（共 4 处，全部在 `struct PolyParser` 内）：
1. 成员区 +2：`lastSplit` / `splitVars`（行 500）
2. `prim()` NUM 分支：置 `lastSplit=false`（行 503）
3. `prim()` 括号分支：置 `lastSplit=false`（行 504）
4. `prim()` 拆分分支：登记 `splitVars`；单名/KEEP/失败路径清旗（行 514-523）
5. `post()` `^` 分支：拆分组时重构成 `前缀·末变量^e`，应用后清旗（行 524-533）

（注：1-3 合并为 4 处主结构改动，实际 hunk 5 个。）

**验收**：
- 347/347 SMOKE 回归零破坏
- `xy^2 == x*y^2` → True（原 False）
- `(x+y)^3 == x^3 + 3x^2y + 3xy^2 + y^3` → True（原 False）
- `id: 2xy^2 == 2*x*y^2` → True
- 数值 lane 未动（`V: xy^2` 弃权是诚实的，与立身之本一致）
- 未触 core/、lab_codes.tsv、三锚点

**仍挂账（供 BACKLOG，不是本补丁范围）**：
- `limit: n->inf of sin(n)` → `True sin(n)`：非有理式实参静默回声，宜弃权
- 数值 lane 多字母连写（`V: 2xy`）不拆 → UNDECIDED（诚实但与多项式 lane 口径不一）
- `xy^2^3` 链式幂：首 ^ 修末变量、次 ^ 作用在整体（罕见写法，行为未立法）
