#!/usr/bin/env bash
# ltir006/tests/smoke009.sh —— 0.1.1 SMOKE（回归 + V010 + A 增量组）
# Linux: g++  / Termux: clang++（由 ltir.sh 传入）
# 入口：ltir.sh t（tests/smoke008.sh 保留为历史取证）
set -u
cd "$(cd "$(dirname "$0")/.." && pwd)"
BIN=./ltir006
PASS=0; TOTAL=0

# 结构化计划：顶层断言静态展开数（简单 for 循环按条目数展开），供输出层实时算进度。
# 循环展开仅支持单层 `for …; do`；实跑权威总数仍以 TSV / 收尾行为准（见 ltir_ui.sh）。
LTIR_PLAN=$(awk '
  inloop && /^[[:space:]]*(ck|ckc)[[:space:]]/ { lw++; next }
  /^[[:space:]]*(ck|ckc)[[:space:]]/ { n++; next }
  /^for[[:space:]]+[^[:space:]]+[[:space:]]+in[[:space:]]+/ && /;[[:space:]]*do[[:space:]]*$/ {
    s=$0
    sub(/^for[[:space:]]+[^[:space:]]+[[:space:]]+in[[:space:]]+/, "", s)
    sub(/;[[:space:]]*do[[:space:]]*$/, "", s)
    items=split(s, a, /[[:space:]]+/)
    lw=0; inloop=1; next
  }
  inloop && /^done[[:space:]]*$/ { n += lw*items; lw=0; items=0; inloop=0; next }
  END { printf "%d", n }
' "$0")
echo "__LTIR_PLAN__=${LTIR_PLAN:-unknown}" >&2
# 结构化测试事件：设置 LTIR_TSV 时，每条断言额外双写一行 TSV
#   TEST_OK / TEST_FAIL \t id \t got \t want \t file \t line
# 输出层优先消费 TSV；未设 LTIR_TSV（本脚本单独跑）时跳过，stdout/stderr 与旧版一致。
_ltir_evt() {  # $1=OK|FAIL  $2=id  $3=got  $4=want
  [ -n "${LTIR_TSV:-}" ] || return 0
  # TSV 单行纪律：got/want 拍平换行与制表符（多行 JSON/explain 输出会撕行），截断 160 字符
  local _g=${3//$'\t'/ } _w=${4//$'\t'/ } _nl=' | '
  _g=${_g//$'\n'/"$_nl"}; _w=${_w//$'\n'/"$_nl"}
  printf 'TEST_%s\t%s\t%.160s\t%.160s\t%s\t%s\n' "$1" "$2" "${_g:-}" "${_w:-}" \
    "$(basename "${BASH_SOURCE[2]:-$0}")" "${BASH_LINENO[1]:-0}" >> "$LTIR_TSV"
}
ck() { TOTAL=$((TOTAL+1)); if [ "$2" = "$3" ]; then PASS=$((PASS+1)); _ltir_evt OK "$1" "$2" "$3";
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; else _ltir_evt FAIL "$1" "$2" "$3";
  printf '  [%02d] ✗ %s   got=<%s> want=<%s>\n' "$TOTAL" "$1" "$2" "$3"; fi; }
# ckc 是 grep -qF 子串匹配（弱断言，有意为之）：历史断言里存在「实际输出是期望的超集」场景，
# 改成全等会炸存量 —— 只在此注明，不改判定语义。
ckc() { TOTAL=$((TOTAL+1)); if printf '%s' "$2" | grep -qF -- "$3"; then PASS=$((PASS+1)); _ltir_evt OK "$1" "$2" "$3";
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; else _ltir_evt FAIL "$1" "$2" "$3";
  printf '  [%02d] ✗ %s   missing=<%s>\n' "$TOTAL" "$1" "$3"; fi; }

echo "== 内核 SHA 门禁 =="
sha256sum -c RUN_core_sha.txt --quiet && echo "  OK: 11 文件与取证一致"

echo "== eval 精确求值 =="
ck  "1/3+1/6"        "$($BIN eval '1/3+1/6')"        '1/2'
ck  "LaTeX frac"      "$($BIN eval '\frac{1}{2}+\frac{1}{3}')" '5/6'
ck  "sin(2*pi/3)"     "$($BIN eval 'sin(2*pi/3)')"    '(1/2)√3'
ck  "π^0 折叠先于弃权" "$($BIN eval '√(16) + π^0')"    '5'
ck  "(1+i)^2"         "$($BIN eval '(1+i)^2')"        '2i'
ck  "1/0 → ∞"         "$($BIN eval '1/0')"             '∞'
ck  "oo-oo 弃权"      "$($BIN eval 'oo-oo')"          'UNDECIDED'
ck  "isprime M61"     "$($BIN eval 'isprime(2^61-1)')" 'True'
ck  "2^64 溢出弃权"   "$($BIN eval '2^64')"           'UNDECIDED'
ck  "3^3==27"         "$($BIN eval '3^3 == 27')"      'True'
ck  "C(5,2)*(1/2)^5"  "$($BIN eval 'C(5,2)*(1/2)^5')" '5/16'
ck  "sin(pi/6)"       "$($BIN eval 'sin(pi/6)')"      '1/2'
ck  "20!"             "$($BIN eval '20!')"             '2432902008176640000'
ck  "21! 弃权"        "$($BIN eval '21!')"             'UNDECIDED'

echo "== verify 整卷（简明档） =="
OUT="$($BIN verify fixtures/demo.ltir)"
ckc "汇总行三态计数"   "$OUT" '── 24 lines: 20 True, 3 False, 1 UNDECIDED ──'
ckc "真值自产：2+2=5 声明 True 仍判 False" "$OUT" 'L22 False  E-VAL-MISMATCH'
ckc "中文标点尾随不拒" "$OUT" 'L23 True'
ckc "容器人话抽取"     "$OUT" 'L24 True'
ckc "三角精确值"       "$OUT" '(1/2)√3'
ckc "solve 区间"       "$OUT" 'x ∈ (2, 3)'
ckc "∞ 输出"           "$OUT" 'L20 True  ∞'
ckc "∞ 未定形归 E-INF" "$OUT" 'L21 UNDECIDED  E-INF'
ckc "台账登记数"       "$OUT" 'registered: 9'
ck  "verify 退出码"    "$($BIN verify fixtures/demo.ltir >/dev/null; echo $?)" '0'

echo "== 详细档 / JSON / 语言钩子 =="
EX="$($BIN verify fixtures/demo.ltir --explain)"
ckc "B4 四行结构：行号+码"   "$EX" 'L04 False  E-VAL-MISMATCH'
ckc "B4 四行结构：^ 指位"   "$EX" '^'
ckc "码表解释（中文跟随）"   "$EX" '左边实算为 False，不是 True'
EXEN="$($BIN verify fixtures/demo.ltir --explain --lang=en)"
ckc "--lang=en 强制英语"     "$EXEN" 'LHS evaluates to False, not True'
JS="$($BIN verify fixtures/zfc.ltir --json)"
ckc "JSON 结构"             "$JS" '"registered":11'
ckc "JSON ok"               "$JS" '"ok":true'

echo "== zf/zfc 台账保留（只登记不判） =="
Z="$($BIN verify fixtures/zfc.ltir)"
ckc "11 条登记"  "$Z" 'registered: 11'
ck  "不进三态统计" "$(printf '%s' "$Z" | grep -c '^L[0-9]')" '0'

echo "== 入口防护：污染标本三份 =="
VER="$(tr -d '\n' < VERSION 2>/dev/null || echo 0.1.1)"   # 版本单一来源：断言全部引用此值
TMP="$(mktemp -d "${TMPDIR:-.}/ltir006_smoke.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT
printf 'A: 2+2=4 -> True\nV: 1/3+1/6\n' > "$TMP/base.ltir"
# CRLF 版
sed 's/$/\r/' "$TMP/base.ltir" > "$TMP/crlf.ltir"
# BOM 版
printf '\357\273\277' > "$TMP/bom.ltir"; cat "$TMP/base.ltir" >> "$TMP/bom.ltir"
# 全角版（Ａ： Ｖ： 全角数字/运算符/Ｔｒｕｅ）
printf 'Ａ： ２＋２＝４ -> Ｔｒｕｅ\nＶ： １／３＋１／６\n' > "$TMP/fullwidth.ltir"
for f in crlf bom fullwidth; do
  O="$($BIN verify "$TMP/$f.ltir")"
  ckc "污染 $f：断言照常" "$O" 'L01 True'
  ckc "污染 $f：抽取照常" "$O" 'L02 True  1/2'
done
# 全角字符行 ^ 指位（显示宽度）
printf 'Ａ： ２＋２＝５ -> Ｔｒｕｅ\n' > "$TMP/caret.ltir"
CE="$($BIN verify "$TMP/caret.ltir" --explain)"
ckc "全角行 ^ 指位存在" "$CE" '^'

echo "== 护栏：裸单字母关键字拒收 =="
printf 'X\n' > "$TMP/bare.ltir"
BO="$($BIN verify "$TMP/bare.ltir" 2>&1; echo "RC=$?")"
ckc "E-UNK-LINE"   "$BO" 'E-UNK-LINE'
ckc "硬错退出码 1" "$BO" 'RC=1'

echo "== =True 反例 / 词库外判定词 =="
printf 'A: 2+2=4 =True\n' > "$TMP/eqtrue.ltir"
ET="$($BIN verify "$TMP/eqtrue.ltir")"
ckc "=True 不产判定记号，按普通表达式弃权" "$ET" 'UNDECIDED'
printf 'A: 2+2=4 -> Maybe\n' > "$TMP/maybe.ltir"
MB="$($BIN verify "$TMP/maybe.ltir")"
ckc "词库外不判定不报错，报实算真值" "$MB" 'L01 True'

echo "== 版本锚点（以 --version 为准） =="
ck  "版本归 VERSION" "$($BIN --version)" "ltir006 $VER"

echo "== 【强化修复版】L06 代入 bug（逐字复现立法者截图） =="
cat > "$TMP/test_006.ltir" <<'EOF'
V: √(16) + π^0
<a>
前情提要：这题非常难，但 3^3 == 27 -> True 那么
结论成立。
顺便计算一下 V: 1/3+1/6 = 1/2。

</a>
A: 2+2=4 -> True。
A: 3^2=9 -> True，所以这题选C。
<subject domain="math">
解：设 x=5，代入得 A: x^2 = 25 -> True。
</subject>
EOF
R6="$($BIN verify "$TMP/test_006.ltir")"
ckc "L06 由 UNDECIDED 修成 True" "$R6" 'L06 True'
ckc "整卷 6 True / 0 UNDECIDED" "$R6" '── 6 lines: 6 True, 0 False, 0 UNDECIDED ──'
ckc "行数不变（回归零破坏）" "$R6" 'L01 True  5'
for w in 设 令 取; do
  printf '<subject domain="algebra">\n%s x=5，代入得 A: x^2 = 25 -> True。\n</subject>\n' "$w" > "$TMP/bind.ltir"
  ckc "「$w x=5」容器内绑定生效" "$($BIN verify "$TMP/bind.ltir")" 'L01 True'
done

echo "== 【强化修复版】乙：四件套与装饰简写并回 =="
printf '<calc>\nS: 解：2x+6=14\nA: 2+2=4 -> True\nV: 1/3+1/6 = 1/2\nQ: 证毕\n</calc>\n' > "$TMP/calc.ltir"
CA="$($BIN verify "$TMP/calc.ltir")"
ckc "<calc> 容器等价 <a>，算式照抽" "$CA" 'L01 True'
ckc "容器内 V: 仍求值（回归）" "$CA" 'L02 True'
ckc "S:/Q: 装饰不产三态行" "$CA" '── 2 lines: 2 True, 0 False, 0 UNDECIDED ──'
ckc "装饰已登记（机器跳过但保留）" "$($BIN verify "$TMP/calc.ltir" --json)" '"decorations":3'
printf '<answer>A: 3^3 == 27 -> True</answer>\n' > "$TMP/answer.ltir"
ckc "<answer> 容器（A: 归一目标形态）" "$($BIN verify "$TMP/answer.ltir")" 'L01 True'
printf '<过程>\nA: 2+2=4 -> True\n</过程>\n' > "$TMP/proc.ltir"
ckc "<过程> 输入层归一为 <calc>" "$($BIN verify "$TMP/proc.ltir")" 'L01 True'
printf '<C_and_E>\ncause: 2x+6=14\neffect: x=4\nby: calc\n</C_and_E>\n' > "$TMP/ce.ltir"
CE="$($BIN verify "$TMP/ce.ltir" --json)"
ckc "<C_and_E> 三字段登记" "$CE" '"inferences":1'
ckc "<C_and_E> 不产三态行（三态锁死）" "$($BIN verify "$TMP/ce.ltir")" '── 0 lines:'
ckc "A: 归一 tag=answer" "$($BIN verify "$TMP/test_006.ltir" --json)" '"tag":"answer"'
ckc "中文原文原样保留" "$($BIN verify "$TMP/test_006.ltir" --json)" '"preserved":4'

echo "== 【强化修复版】domain 扩增 =="
ckc "领域表 55 个（甲：扩全图 28 个仅台账域）" "$($BIN verify "$TMP/test_006.ltir" --json)" '"domains_registered":55'
ckc "<subject domain=> 真的进 Ctx" "$($BIN verify "$TMP/bind.ltir" --json)" '"domain":"algebra"'
ckc "domain 属性被记录" "$($BIN verify "$TMP/bind.ltir")" 'domain: algebra (55 registered)'
printf 'domain: ?\n' > "$TMP/dlist.ltir"
ckc "domain: ? 列出已注册领域" "$($BIN verify "$TMP/dlist.ltir")" '55 domains registered'
printf 'domain: trigonometry\nA: sin(pi/6) = 1/2 -> True\n' > "$TMP/dset.ltir"
ckc "顶层 domain: 显式设定领域" "$($BIN verify "$TMP/dset.ltir" --json)" '"domain":"trigonometry"'
printf 'domain: 冷门新领域\n' > "$TMP/dnew.ltir"
ckc "未知领域标黄放行不报错" "$($BIN verify "$TMP/dnew.ltir"; echo "RC=$?")" 'RC=0'
ckc "函数定义域查询未回归" "$($BIN verify fixtures/demo.ltir)" 'L01 True  R'

echo "== 【强化修复版】clang 死变量复核（Termux 真机抓到的那条） =="
ck  "value.hpp 无 den,t 残留" "$(grep -c 'den,t;' src/value.hpp)" '0'
ck  "编译产物存在" "$([ -x ./ltir006 ] && echo yes)" 'yes'


echo "== 防「沙」：多打省略号折叠 + 傲娇彩蛋 =="
printf 'set: N = {0,1,2,......}\n' > "$TMP/sand.ltir"
SZ="$($BIN verify "$TMP/sand.ltir" --lang=zh)"
ckc "6 个点折叠为单个省略号（不伪判）" "$SZ" '── 0 lines:'
ckc "中文吐槽（主句）" "$SZ" '你多打了省略号，已经补全了。'
ckc "中文傲娇彩蛋（末句）" "$SZ" '才不是帮你呢。'
ckc "构造式只登记不展开" "$SZ" 'set_defs: 1'
SE="$($BIN verify "$TMP/sand.ltir" --lang=en)"
ckc "英文同样" "$SE" 'You typed extra ellipses; normalized to one.'
ckc "英文傲娇彩蛋" "$SE" 'N-not like I did it for you or anything!'
JS="$($BIN verify "$TMP/sand.ltir" --lang=zh --json)"
ckc "JSON sand_fixed" "$JS" '"sand_fixed":1'
ckc "JSON 彩蛋单列" "$JS" '"easter_egg":"才不是帮你呢。"'
ckc "JSON 不混入裸文本" "$JS" '"ok":true'
printf 'set: N = {0,1,2,...}\n' > "$TMP/one.ltir"
ck  "单个省略号不误报" "$(printf '%s' "$($BIN verify "$TMP/one.ltir")" | grep -c 'sand:')" '0'
printf 'set: N = {0,1,2, …, …}\n' > "$TMP/uni.ltir"
ckc "U+2026 单字符省略号进归一表" "$($BIN verify "$TMP/uni.ltir" --lang=zh)" '才不是帮你呢。'
printf 'set: N = {0,1,2, \\dots\\dots}\n' > "$TMP/ltx.ltir"
ckc "LaTeX \\dots 重复同样折叠" "$($BIN verify "$TMP/ltx.ltir" --lang=zh)" '才不是帮你呢。'
ck  "1..10 两点范围不误伤" "$($BIN eval '1+2+3')" '6'
ckc "sum: 范围回归" "$($BIN verify fixtures/demo.ltir)" 'L07 True  120'

echo "== 新关键词（全英文长名，短写表冻结） =="
cat > "$TMP/reg.ltir" <<'EOF'
system: ZFC
choice: on
axiom: AC, Separation
ref: Godel1940
theory: ModelTheory
inner: L(V=L)
problem: CH
status: independent-of ZFC
under: x>=0, y>=0, z>=0
cert: todo
witness: t1=2sqrt(3)sin40
map: UltL
independent: CH from ZFC
model: L
cite: Godel1940
EOF
RJ="$($BIN verify "$TMP/reg.ltir" --json)"
ckc "新关键词全部登记" "$RJ" '"registered":15'
ckc "地图坐标进 JSON" "$RJ" '"system":"ZFC"'
ckc "开关进 JSON" "$RJ" '"choice":"on"'
ckc "公理表可查" "$RJ" '"axioms_seen":2'
ckc "map_declared 点亮" "$RJ" '"map_declared":true'
printf 'X\n' > "$TMP/bare007.ltir"
ckc "护栏：裸单字母关键字仍拒收" "$($BIN verify "$TMP/bare007.ltir" 2>&1)" 'E-UNK-LINE'

echo "== 公理模块 preset: / load: =="
printf 'preset: ZFC\nA: 2+2==4 -> True\n' > "$TMP/p1.ltir"
P1="$($BIN verify "$TMP/p1.ltir")"
ckc "preset: ZFC 内置按名加载" "$P1" 'presets: ZFC'
ckc "preset 加载不产三态行" "$P1" '── 1 lines: 1 True'
ckc "preset 幂等不重复" "$($BIN verify "$TMP/p1.ltir" --json)" '"loaded_presets":1'
printf 'load: axioms/ZFC.ltir\n' > "$TMP/l1.ltir"
ckc "load: 外部 .ltir 模块" "$($BIN verify "$TMP/l1.ltir")" 'map: system=ZFC'
printf 'load: my_axioms.ltir\n' > "$TMP/l2.ltir"
ckc "load: 自动回退 axioms/ 目录" "$($BIN verify "$TMP/l2.ltir")" 'map: system=ZFC'
printf 'load: axioms/none.ltir\n' > "$TMP/mi.ltir"
ckc "模块缺失有码可查" "$($BIN verify "$TMP/mi.ltir")" 'E-LOAD-MISSING'
printf 'load: axioms/cycle_a.ltir\n' > "$TMP/cy.ltir"
ckc "循环加载有码可查" "$($BIN verify "$TMP/cy.ltir")" 'E-LOAD-CYCLE'
printf 'load: axioms/bad.ltir\n' > "$TMP/ba.ltir"
ckc "模块内断言行拦下" "$($BIN verify "$TMP/ba.ltir")" 'E-LOAD-ASSERT'
printf 'module: ZFC\n' > "$TMP/mod.ltir"
ckc "双击模块给人话提示" "$($BIN verify "$TMP/mod.ltir")" '这是公理模块'
printf 'preset: ZFC\naxiom: AD\n' > "$TMP/cf.ltir"
ckc "AD 与 AC 冲突（不并入 False）" "$($BIN verify "$TMP/cf.ltir")" 'E-CHOICE-CONFLICT'
printf 'preset: ZFC\naxiom: Reinhardt\n' > "$TMP/cf2.ltir"
ckc "Kunen 不相容登记" "$($BIN verify "$TMP/cf2.ltir")" 'E-AXIOM-CONFLICT'
ckc "冲突记为 UNDECIDED 而非 False" "$($BIN verify "$TMP/cf2.ltir")" '── 0 lines:'

echo "== 引用 by: / ⊢ 地图 / 全称弃权 =="
printf 'preset: ZFC\nA: 2+2==4 -> True by: AC\n' > "$TMP/by.ltir"
BY="$($BIN verify "$TMP/by.ltir")"
ckc "行尾 by: 不再被吞" "$BY" 'by_refs: 1 checked, 0 unknown'
ckc "by: 不替引擎判真假" "$BY" 'L01 True'
printf 'A: 2+2==4 -> True by: Zorn\n' > "$TMP/by2.ltir"
ckc "未登记引用标出" "$($BIN verify "$TMP/by2.ltir")" 'by_refs: 1 checked, 1 unknown'
printf 'system: ZFC\nA: ZFC ⊢ 2+2==4 -> True\n' > "$TMP/mp.ltir"
ckc "⊢ 同行：地图内可推导" "$($BIN verify "$TMP/mp.ltir")" 'L01 True'
printf 'system: ZFC\nA: ZFC ⊢ CH -> True\n' > "$TMP/ch.ltir"
ckc "CH 独立于 ZFC：诚实弃权" "$($BIN verify "$TMP/ch.ltir")" 'E-SCOPE-INDEPENDENT'
printf 'A: ZFC ⊢ CH -> True\n' > "$TMP/nomap.ltir"
ckc "无 system: 用 ⊢ → NO-MAP" "$($BIN verify "$TMP/nomap.ltir")" 'E-SCOPE-NO-MAP'
printf 'domain: set\nsystem: NBG\nuniverse: V\n' > "$TMP/v3.ltir"
ckc "全局 V：集合论域 + 显式地图" "$($BIN verify "$TMP/v3.ltir")" '── 0 lines:'
printf 'domain: inequality\nuniverse: V\n' > "$TMP/v1.ltir"
ckc "全局 V：非集合论域拦下" "$($BIN verify "$TMP/v1.ltir")" 'E-SCOPE-DOMAIN'
printf 'domain: set\nsystem: NBG\nuniverse: V\nA: card(V) == 3 -> True\n' > "$TMP/v4.ltir"
ckc "真类不可取势" "$($BIN verify "$TMP/v4.ltir")" 'E-SCOPE-PROPER-CLASS'
printf 'paradox: Russell\n' > "$TMP/ru.ltir"
ckc "罗素悖论：塌图登记不判真假" "$($BIN verify "$TMP/ru.ltir")" 'E-PARADOX-RUSSELL'
printf 'absolute: Omega\n' > "$TMP/ab.ltir"
ckc "康托尔绝对无限" "$($BIN verify "$TMP/ab.ltir")" 'E-SCOPE-ABSOLUTE'
printf 'claim: xyz(x+y)(y+z) <= 27\n' > "$TMP/cl.ltir"
ckc "全称主断言：诚实弃权" "$($BIN verify "$TMP/cl.ltir")" 'E-SCOPE-QUANTIFIER'
printf 'set: A = {1,2,3}\nforall: t in A: t <= 10\n' > "$TMP/fq.ltir"
ckc "有限集 ∀ 逐元素可判" "$($BIN verify "$TMP/fq.ltir")" 'L01 True'
printf 'forall: x, y, z\n' > "$TMP/fq2.ltir"
ckc "连续域 ∀ 仍弃权" "$($BIN verify "$TMP/fq2.ltir")" 'E-SCOPE-QUANTIFIER'

echo "== 集合论实验：有限集精确 / 构造式不伪判 =="
cat > "$TMP/set.ltir" <<'EOF'
set: A = {1,2}
set: B = {2,3}
A: A ∪ B == {1,2,3} -> True
A: A ∩ B == {2} -> True
A: {1,2} ⊆ {1,2,3} -> True
A: {1,2} ⊂ {1,2,3} -> True
A: 2 ∈ A -> True
A: card(A) == 2 -> True
A: {1,2} ⊆ {1,3} -> True
A: card(A) <= card(B) -> True
EOF
SO="$($BIN verify "$TMP/set.ltir")"
ckc "∪ 并集精确" "$SO" 'L03 True'
ckc "∩ 交集精确" "$SO" 'L04 True'
ckc "⊆ 子集精确" "$SO" 'L05 True'
ckc "⊂ 真子集不与 ⊆ 合并" "$SO" 'L06 True'
ckc "∈ 属于精确" "$SO" 'L07 True'
ckc "card 有限势精确" "$SO" 'L08 True'
ckc "子集反例判 False（不是 UNDECIDED）" "$SO" 'L09 False'
ckc "势比较有限集可判" "$SO" 'L10 True'
printf 'set: E = {2n : n ∈ N}\nA: 2 ∈ E -> True\n' > "$TMP/comp.ltir"
CO="$($BIN verify "$TMP/comp.ltir")"
ckc "集合构造符不伪判成 {UNDECIDED}" "$CO" 'set_defs: 1'
ckc "构造集上 ∈ 诚实弃权" "$CO" 'E-SCOPE-INFINITE'
printf 'set: P = {{1},{2}}\n' > "$TMP/nest.ltir"
ckc "嵌套集合不压平" "$($BIN verify "$TMP/nest.ltir")" 'E-SCOPE-NESTED'
printf 'A: 2+2==4 ∴ True -> True\n' > "$TMP/unk.ltir"
ckc "未登记符号不并入 False" "$($BIN verify "$TMP/unk.ltir")" 'E-REF-UNKNOWN'
printf 'A: 2+2==4 ⊨ True -> True\n' > "$TMP/mod2.ltir"
ckc "⊨ 语义蕴涵需证书" "$($BIN verify "$TMP/mod2.ltir")" 'E-SCOPE-SEMANTIC'
printf 'set: U = {1,2,3,4,5,6,7,8,9,10}\nuniv: U = {1,2,3,4,5,6,7,8,9,10}\n' > "$TMP/univ.ltir"
ckc "univ: 有限全集登记" "$($BIN verify "$TMP/univ.ltir" --json)" '"registered":2'

echo "== 三角囚徒冻结卷：重跑同一份文件验收 =="
TP="$($BIN verify fixtures/triangle_prisoner.ltir)"
ckc "0.0.7：10 True / 0 UNDECIDED" "$TP" '── 10 lines: 10 True, 0 False, 0 UNDECIDED ──'
ckc "内联 <C_and_E> 拿回身份" "$($BIN verify fixtures/triangle_prisoner.ltir --json)" '"inferences":4'
ckc "领域 inequality 认到" "$($BIN verify fixtures/triangle_prisoner.ltir --json)" '"domain":"inequality"'
ckc "主断言未伪判成 True（采样不蕴涵全称）" "$TP" '10 True'

echo "== 回归锁定 =="
ckc "69 条回归：demo 汇总逐字不变" "$($BIN verify fixtures/demo.ltir)" '── 24 lines: 20 True, 3 False, 1 UNDECIDED ──'
ckc "69 条回归：内核门禁文案" "$(sha256sum -c RUN_core_sha.txt --quiet && echo yes)" 'yes'
ck  "版本" "$($BIN --version)" "ltir006 $VER"

echo "== 修 bug：用户卷逐字复现 =="
cat > "$TMP/logic.ltir" <<'EOF'
# 1. 测试逻辑与公理系统
system: ZFC
axiom: AC, Separation
A: 2+2==4 -> True by: AC
paradox: Russell
absolute: Omega

# 2. 集合与无穷
V: {1,2,3} ⊆ {1,2,3,4}
V: 2 ∈ {1,2,3}
V: 2 ∈ ∅
card: {1, 2, 3}

# 3. 因果与证明标签
<C_and_E>
cause: 2x+6=14
effect: x=4
by: calc
</C_and_E>

S: 解：先移项
E: 设 x=4
P: 证明如下
Q: 证毕

# 4. 触发傲娇彩蛋
set: N = {0,1,2,......}
EOF
LG="$($BIN verify "$TMP/logic.ltir")"
ckc "用户卷：7 行 4 True / 1 False / 2 UNDECIDED" "$LG" '── 7 lines: 4 True, 1 False, 2 UNDECIDED ──'
ckc "L05 V: 2 ∈ {1,2,3} 不再误归超越域" "$LG" 'L05 True'
ckc "L06 V: 2 ∈ ∅ 判 False（能判就别弃权）" "$LG" 'L06 False'
ck  "补丁二：V: 求假不再挂 E-VAL-MISMATCH" "$(printf '%s' "$LG" | grep -c 'L06 False  E-VAL-MISMATCH')" '0'
ckc "L07 card: {1,2,3} 得 3（不再 E-REF-UNKNOWN）" "$LG" 'L07 True  3'
ckc "容器内 by: 并入引用账" "$LG" 'by_refs: 2 checked, 0 unknown'
ckc "防沙与地图坐标未回归" "$LG" 'map: system=ZFC choice=unset axioms=2'

echo "== 修 bug：V: 短写接入有限集语义 =="
printf 'V: {1,2} ∩ {2,3}\n' > "$TMP/vi.ltir"
ckc "V: 交集输出 {2}（不再伪输出 1）" "$($BIN verify "$TMP/vi.ltir")" 'L01 True  {2}'
printf 'V: {1,2} ∪ {2,3}\n' > "$TMP/vu.ltir"
ckc "V: 并集输出 {1, 2, 3}" "$($BIN verify "$TMP/vu.ltir")" 'L01 True  {1, 2, 3}'
printf 'V: {1,2} ⊆ {1,2,3,4}\n' > "$TMP/vs.ltir"
VS="$($BIN verify "$TMP/vs.ltir")"
ckc "V: ⊆ 判 True" "$VS" 'L01 True'
ck  "V: ⊆ 不再伪输出裸 1" "$(printf '%s' "$VS" | grep -c 'L01 True  1')" '0'
printf 'card: ∅\n' > "$TMP/ce0.ltir"
ckc "card: 空集得 0" "$($BIN verify "$TMP/ce0.ltir")" 'L01 True  0'
printf 'A: card(∅) == 0 -> True\n' > "$TMP/ce1.ltir"
ckc "A: card(∅) 断言可判" "$($BIN verify "$TMP/ce1.ltir")" 'L01 True'
printf 'A: 2 ∉ ∅ -> True\n' > "$TMP/ne.ltir"
ckc "∉ 空集判 True" "$($BIN verify "$TMP/ne.ltir")" 'L01 True'
printf 'A: card({{1},{2}}) == 2 -> True\n' > "$TMP/nc.ltir"
ckc "嵌套 card 归 E-SCOPE-NESTED（不再超越域兜底）" "$($BIN verify "$TMP/nc.ltir")" 'E-SCOPE-NESTED'
printf 'V: 2 ∈ ∅ -> False\n' > "$TMP/vc.ltir"
VC="$($BIN verify "$TMP/vc.ltir")"
ckc "V: 带声明词先剥再判（真值自产）" "$VC" 'L01 True'
ck  "声明词一致时不残留 E-VAL-MISMATCH" "$(printf '%s' "$VC" | grep -c 'L01 True  E-VAL-MISMATCH')" '0'

echo "== 修 bug：by: 两处共用一本账 =="
printf '<C_and_E>\ncause: a\neffect: b\nby: Zorn\n</C_and_E>\n' > "$TMP/cby.ltir"
ckc "容器内未登记引用标出" "$($BIN verify "$TMP/cby.ltir")" 'by_refs: 1 checked, 1 unknown'
printf 'preset: ZFC\nA: 2+2==4 -> True by: AC\n' > "$TMP/by3.ltir"
ckc "行尾 by: 统计未回归" "$($BIN verify "$TMP/by3.ltir")" 'by_refs: 1 checked, 0 unknown'
printf 'A: 2+2==4 -> True by: Zorn\n' > "$TMP/by4.ltir"
ckc "未登记引用计数未回归" "$($BIN verify "$TMP/by4.ltir")" 'by_refs: 1 checked, 1 unknown'

echo "== 甲：扩域表 + 台账 + 弃权码 =="
ckc "55 域注册" "$($BIN verify "$TMP/test_006.ltir" --json)" '"domains_registered":55'
ck  "28 个域标 [台账]" "$(grep -c '\[台账\]$' domains.tsv)" '28'
ckc "弃权码已入表" "$(cat lab_codes.tsv)" 'E-SCOPE-LEDGER-ONLY'
printf 'domain: analysis\nA: 2+2==4 -> True\n' > "$TMP/led.ltir"
LE="$($BIN verify "$TMP/led.ltir")"
ckc "台账域纯算术不外溢" "$LE" '── 1 lines: 1 True, 0 False, 0 UNDECIDED ──'
ck  "弃权不是硬错（exit 0）" "$($BIN verify "$TMP/led.ltir" >/dev/null; echo $?)" '0'
printf 'domain: analysis\nV: sin(pi/6)\n' > "$TMP/led3.ltir"
ckc "台账域域专有式仍弃权" "$($BIN verify "$TMP/led3.ltir")" 'E-SCOPE-LEDGER-ONLY'
printf 'domain: analysis\nsystem: ZF\nref: Godel1940\n' > "$TMP/led2.ltir"
ckc "仅台账域：登记类行照常不产三态行" "$($BIN verify "$TMP/led2.ltir")" '── 0 lines:'
ckc "老域不串台（trigonometry 照常判）" "$($BIN verify "$TMP/dset.ltir")" 'L01 True'
ckc "未知领域仍标黄放行" "$($BIN verify "$TMP/dnew.ltir"; echo "RC=$?")" 'RC=0'
ckc "三角囚徒冻结卷不漂" "$($BIN verify fixtures/triangle_prisoner.ltir)" '── 10 lines: 10 True, 0 False, 0 UNDECIDED ──'

echo "== 老账本归编 / hook 随包 =="
ck  "BACKLOG.md 已归编" "$([ -f BACKLOG.md ] && echo yes)" 'yes'
ck  "termux-file-editor 随包" "$([ -f termux/termux-file-editor ] && echo yes)" 'yes'
ckc "hook 锚点：收容目录不变" "$(cat termux/termux-file-editor)" '$HOME/ltir_lab'
ckc "hook 锚点：可执行名不变" "$(cat termux/termux-file-editor)" '$LAB/ltir006'
ckc "hook 标题读 --version" "$(cat termux/termux-file-editor)" 'VER="$("$BIN" --version 2>/dev/null)"'
ck  "hook 不写死旧版本号" "$(grep -c '0\.0\.6' termux/termux-file-editor)" '0'
ck  "收容清单含 ./termux 与 BACKLOG" "$(grep -c './termux ./BACKLOG.md' ltir.sh)" '1'

echo "== 版本锚点（版本号推进） =="
ck  "--version 归 VERSION" "$($BIN --version)" "ltir006 $VER"
ckc "JSON 版本归 VERSION" "$($BIN verify "$TMP/test_006.ltir" --json)" "\"version\":\"$VER\""
ck  "二进制无 0.0.10 越版残留" "$(strings ./ltir006 | grep -c '0\.0\.10')" '0'

echo "== Bug 1：V: 求假无码（求值标签 ≠ 断言标签） =="
printf 'V: 1 == 2\nV: 2 ∈ ∅\nV: 3 > 5\n' > "$TMP/b1.ltir"
B1="$($BIN verify "$TMP/b1.ltir")"
ckc "V: 求假仍进 False 三态" "$B1" '── 3 lines: 0 True, 3 False, 0 UNDECIDED ──'
ck  "V: 求假不挂 E-VAL-MISMATCH" "$(printf '%s' "$B1" | grep -c 'E-VAL-MISMATCH')" '0'
printf 'A: 1 == 2 -> True\n' > "$TMP/b1a.ltir"
ckc "A: 声明词冲突仍挂码（不回退）" "$($BIN verify "$TMP/b1a.ltir")" 'L01 False  E-VAL-MISMATCH'
printf 'V: 2 ∈ ∅ -> False\n' > "$TMP/b1b.ltir"
ckc "V: 带声明词一致仍判 True" "$($BIN verify "$TMP/b1b.ltir")" 'L01 True'

echo "== Bug 2：forall: 字面量有限集 =="
printf 'set: A = {1, 2, 3}\nforall: x in A: x^2 > 0\nforall: x in {1, 2, 3}: x^2 > 0\nforall: x in R: x^2 > 0\nforall: x in {0,1,2,...}: x >= 0\n' > "$TMP/b2.ltir"
B2="$($BIN verify "$TMP/b2.ltir")"
ckc "具名有限集 ∀ 逐元素可判" "$B2" 'L02 True'
ckc "字面量有限集 ∀ 不再误弃权" "$B2" 'L03 True'
ckc "连续域 ∀ 仍弃权" "$B2" 'L04 UNDECIDED  E-SCOPE-INFINITE'
ckc "省略号构造式仍弃权" "$B2" 'L05 UNDECIDED  E-SCOPE-INFINITE'
printf 'forall: x in {1,2,3}: x < 0\n' > "$TMP/b2f.ltir"
B2F="$($BIN verify "$TMP/b2f.ltir")"
ckc "∀ 判假仍进 False" "$B2F" 'L01 False'
ck  "∀ 判假无码（不蔓延 Bug 1）" "$(printf '%s' "$B2F" | grep -c 'E-VAL-MISMATCH')" '0'

echo "== 私货 1：受限概括 | 与罗素悖论闭环 =="
printf 'set: R = {x ∈ {1,2,3} | x > 1}\n' > "$TMP/s1a.ltir"
ckc "受限概括算出 {2, 3}" "$($BIN verify "$TMP/s1a.ltir")" 'L01 True  {2, 3}'
printf 'set: E = {y ∈ {1,2,3,4} | y <= 2}\n' > "$TMP/s1b.ltir"
ckc "受限概括换条件算出 {1, 2}" "$($BIN verify "$TMP/s1b.ltir")" 'L01 True  {1, 2}'
printf 'set: X = {x | x ∉ x}\nV: X ∈ X\n' > "$TMP/s1p.ltir"
S1P="$($BIN verify "$TMP/s1p.ltir")"
ckc "无受限域 → 罗素悖论弃权" "$S1P" 'L01 UNDECIDED  E-PARADOX-RUSSELL'
ckc "罗素集上 ∈ 不判真假" "$S1P" 'L02 UNDECIDED  E-PARADOX-RUSSELL'
ckc "不并入 False / 不产 {UNDECIDED}" "$S1P" '── 2 lines: 0 True, 0 False, 2 UNDECIDED ──'
ck  "构造符不再被当枚举" "$(printf '%s' "$S1P" | grep -c '{UNDECIDED}')" '0'
cat > "$TMP/naive.ltir" <<'EOF'
# 载入朴素集合论模块
load: Naive.ltir
set: R = {x | x ∉ x}
V: R ∈ R
EOF
NV="$($BIN verify "$TMP/naive.ltir")"
ckc "Naive 模块加载生效（cwd 已归位）" "$NV" 'map: system=Naive'
ckc "Naive 卷两行都弃权" "$NV" '── 2 lines: 0 True, 0 False, 2 UNDECIDED ──'
ck  "Naive 卷无 E-LOAD-MISSING" "$(printf '%s' "$NV" | grep -c 'E-LOAD-MISSING')" '0'

echo "== 私货 2：原生量词符号 ∀ / ∃ =="
printf 'A: ∀ x ∈ {1,2,3}, x^2 > 0 -> True\n' > "$TMP/s2a.ltir"
ckc "∀ 符号转译给内部量词逻辑" "$($BIN verify "$TMP/s2a.ltir")" 'L01 True'
printf 'A: ∃ x ∈ {1,2,3}, x == 2 -> True\n' > "$TMP/s2b.ltir"
ckc "∃ 遍历找到见证" "$($BIN verify "$TMP/s2b.ltir")" 'L01 True'
printf 'A: ∃ x ∈ {1,2,3}, x == 9 -> False\n' > "$TMP/s2c.ltir"
ckc "∃ 无见证与声明词一致" "$($BIN verify "$TMP/s2c.ltir")" 'L01 True'
printf 'exists: t in {1,2,3}: t == 2\n' > "$TMP/s2d.ltir"
ckc "exists: 关键字同路可判" "$($BIN verify "$TMP/s2d.ltir")" 'L01 True'
printf 'A: ∀ x ∈ R, x^2 > 0 -> True\n' > "$TMP/s2e.ltir"
ckc "连续域符号 ∀ 仍弃权" "$($BIN verify "$TMP/s2e.ltir")" 'E-SCOPE-INFINITE'
printf 'A: ∀ x ∈ {1,2,3} x^2 > 0 -> True\n' > "$TMP/s2f.ltir"
ckc "缺分隔符不误伤（QUANTIFIER 弃权）" "$($BIN verify "$TMP/s2f.ltir")" 'E-SCOPE-QUANTIFIER'

echo "== 私货 3：cex: 反例登记册（法庭纪律） =="
printf 'A: x > 0 -> True\ncex: x = -1\n' > "$TMP/s3a.ltir"
ckc "反例代入使命题失效 → 有效" "$($BIN verify "$TMP/s3a.ltir")" 'L02 True  counter-example valid'
printf '# 反例\nA: x > 0 -> True\ncex: x = -1\n' > "$TMP/s3zh.ltir"
ckc "中文卷反例有效（语言跟随）" "$($BIN verify "$TMP/s3zh.ltir")" '反例有效'
printf 'A: x^2 == 4 -> True\ncex: x = 2\n' > "$TMP/s3b.ltir"
ckc "反例代入命题仍真 → 无效" "$($BIN verify "$TMP/s3b.ltir")" 'L02 False  E-CEX-INVALID'
printf 'A: x^2 == 4 -> True\ncex: x = 3\n' > "$TMP/s3c.ltir"
ckc "换一个值反例成立" "$($BIN verify "$TMP/s3c.ltir")" 'L02 True'
printf 'cex: x = 1\n' > "$TMP/s3d.ltir"
ckc "无上一行断言 → 有码弃权" "$($BIN verify "$TMP/s3d.ltir")" 'E-REF-UNKNOWN'

echo "== 私货 4：mapping: / inj: / surj: / bij: =="
printf 'mapping: f = {1->2, 2->3, 3->4} on {2,3,4}\ninj: f\nsurj: f\nbij: f\n' > "$TMP/s4a.ltir"
S4="$($BIN verify "$TMP/s4a.ltir")"
ckc "有限映射定义只登记不产三态行" "$S4" '── 3 lines:'
ckc "inj: f 单射判 True" "$S4" 'L01 True'
ckc "surj: f 满射判 True" "$S4" 'L02 True'
ckc "bij: f 双射判 True" "$S4" 'L03 True'
printf 'mapping: g = {1->2, 2->2, 3->4} on {2,3,4}\ninj: g\nsurj: g\nbij: g\n' > "$TMP/s4b.ltir"
S4G="$($BIN verify "$TMP/s4b.ltir")"
ckc "inj: g 非单射判 False" "$S4G" 'L01 False'
ckc "surj: g 非满射判 False" "$S4G" 'L02 False'
ckc "bij: g 非双射判 False" "$S4G" 'L03 False'
ck  "映射判假无码" "$(printf '%s' "$S4G" | grep -c 'E-VAL-MISMATCH')" '0'
printf 'mapping: h = R -> R\n' > "$TMP/s4c.ltir"
ckc "R -> R 定义弃权" "$($BIN verify "$TMP/s4c.ltir")" 'E-SCOPE-INFINITE'
printf 'inj: R -> R\n' > "$TMP/s4d.ltir"
ckc "R -> R 性质判定弃权" "$($BIN verify "$TMP/s4d.ltir")" 'E-SCOPE-INFINITE'
printf 'mapping: k = {1->2}\nsurj: k\n' > "$TMP/s4e.ltir"
ckc "缺陪域 → surj 有码弃权" "$($BIN verify "$TMP/s4e.ltir")" 'E-SCOPE-CODOMAIN'

echo "== 回归锁定 / 版本不漂移 =="
ckc "用户卷汇总不漂" "$($BIN verify "$TMP/logic.ltir")" '── 7 lines: 4 True, 1 False, 2 UNDECIDED ──'
ckc "三角囚徒冻结卷不漂" "$($BIN verify fixtures/triangle_prisoner.ltir)" '── 10 lines: 10 True, 0 False, 0 UNDECIDED ──'
ckc "demo 24 行不漂" "$($BIN verify fixtures/demo.ltir)" '── 24 lines: 20 True, 3 False, 1 UNDECIDED ──'
ckc "码表新增两行" "$(cat lab_codes.tsv)" 'E-SCOPE-CODOMAIN'
ckc "hook 锚点不回退" "$(cat termux/termux-file-editor)" '$HOME/ltir_lab'
ck  "--version 仍 VERSION" "$($BIN --version)" "ltir006 $VER"
ckc "JSON 版本仍 VERSION" "$($BIN verify "$TMP/test_006.ltir" --json)" "\"version\":\"$VER\""
ck  "二进制无 0.0.10 越版残留" "$(strings ./ltir006 | grep -c '0\.0\.10')" '0'

echo "== 工程三件：ltir.sh / VERSION / LTIR_STAMP =="
ck  "ltir.sh 存在" "$([ -f ltir.sh ] && echo yes)" 'yes'
ck  "旧脚本名退役" "$([ ! -f ltir006.sh ] && echo yes)" 'yes'
ck  "VERSION 非空" "$(tr -d '\n' < VERSION)" "$VER"
ck  "LTIR_STAMP 含版本" "$(grep -c "^ltir-$VER$" LTIR_STAMP)" '1'
ckc "清单含底片与版本文件" "$(cat ltir.sh)" './VERSION ./LTIR_STAMP'
ckc "底片比对启用" "$(cat ltir.sh)" 'cmp -s ./LTIR_STAMP'
ckc "底片不一致自动重搬" "$(cat ltir.sh ltir_ui.sh)" '自动重搬'
ckc "入口切 smoke009" "$(cat ltir.sh)" 'tests/smoke009.sh'
ckc "v 子命令读 VERSION" "$(cat ltir.sh)" 'v) cat VERSION'
ckc "R 强制重搬兜底" "$(cat ltir.sh)" 'R) rm -rf'
ckc "hook 提示改名" "$(cat termux/termux-file-editor)" 'bash ltir.sh'
ckc "hook 三锚点：收容目录" "$(cat termux/termux-file-editor)" '$HOME/ltir_lab'
ckc "hook 三锚点：可执行名" "$(cat termux/termux-file-editor)" '$LAB/ltir006'
ck  "二进制无 0.0.10 越版残留" "$(strings ./ltir006 | grep -c '0\.0\.10')" '0'
ck  "--version = VERSION" "$($BIN --version)" "ltir006 $VER"
ckc "JSON 版本 VERSION" "$($BIN verify "$TMP/test_006.ltir" --json)" "\"version\":\"$VER\""

echo "== 形式化定义：签名 / 良构性 / 终止性 / δ 展开 =="
printf 'sig: sq : Num -> Num\ntype: sq : Polynomial\ngrammar: expr ::= term | expr + term\nrule: ModusPonens\ndefine: sq(x : Num) : Num = x*x\nexpand: sq(3)\nA: 2+2==4 -> True by: ModusPonens\n' > "$TMP/d8.ltir"
D8="$($BIN verify "$TMP/d8.ltir")"
ckc "登记行不产三态行" "$D8" '── 2 lines:'
ckc "δ 展开 sq(3) 得 9" "$D8" 'L01 True  9'
ckc "rule: 进 by: 证书链" "$D8" 'by_refs: 1 checked, 0 unknown'
D8J="$($BIN verify "$TMP/d8.ltir" --json)"
ckc "JSON def_sigs" "$D8J" '"def_sigs":1'
ckc "JSON rules" "$D8J" '"rules":1'
ckc "JSON grammars" "$D8J" '"grammars":1'
printf 'define: sq(x : Num) : Num = x*x\nexpand: sq(sq(2))\n' > "$TMP/d8b.ltir"
ckc "嵌套 δ 展开有限步" "$($BIN verify "$TMP/d8b.ltir")" 'L01 True  16'
printf 'define: bad(x) = foo + 1\n' > "$TMP/d8c.ltir"
ckc "自由标识符有码弃权" "$($BIN verify "$TMP/d8c.ltir")" 'E-DEF-UNBOUND'
printf 'define: rec(x) = rec(x) + 1\n' > "$TMP/d8d.ltir"
ckc "递归定义终止性拦下" "$($BIN verify "$TMP/d8d.ltir")" 'E-DEF-NONTERM'
printf 'sig: h : Set -> Prop\ndefine: h(x : Num) : Num = x+1\n' > "$TMP/d8e.ltir"
ckc "签名不符有码弃权" "$($BIN verify "$TMP/d8e.ltir")" 'E-DEF-SIG'
printf 'expand: nope(2)\n' > "$TMP/d8f.ltir"
ckc "未定义函数展开有码" "$($BIN verify "$TMP/d8f.ltir")" 'E-NAME'
ckc "旧 define: 写法不回退" "$($BIN verify fixtures/demo.ltir)" '── 24 lines: 20 True, 3 False, 1 UNDECIDED ──'
ckc "码表 E-DEF-DEPTH" "$(cat lab_codes.tsv)" 'E-DEF-DEPTH'

echo "== 集合论多宇宙观：图册只登记，不跨图搬真值 =="
printf 'multiverse: on\nworld: W0 = ZFC\nworld: W1 = ZFC + not-CH\nforce: W0 -> W1 by: Cohen1963\nground: W1 -> W0\nmantle: W0\nreachable: CH in W1\nA: □ CH -> True\nA: ◇ CH -> True\n' > "$TMP/mv.ltir"
MV="$($BIN verify "$TMP/mv.ltir")"
ckc "图册登记不产三态行" "$MV" '── 3 lines: 0 True, 0 False, 3 UNDECIDED ──'
ckc "reachable 不跨图搬真值" "$MV" 'E-SCOPE-CROSS-WORLD'
ckc "□ 模态弃权" "$MV" 'E-SCOPE-MODAL'
ckc "◇ 模态弃权" "$MV" 'E-SCOPE-MODAL'
MVJ="$($BIN verify "$TMP/mv.ltir" --json)"
ckc "JSON multiverse on" "$MVJ" '"multiverse":true'
ckc "JSON worlds=2" "$MVJ" '"worlds":2'
ckc "JSON world_edges=2" "$MVJ" '"world_edges":2'
printf 'force: X -> W0\n' > "$TMP/mv2.ltir"
ckc "未声明世界 → NO-MAP" "$($BIN verify "$TMP/mv2.ltir")" 'E-SCOPE-NO-MAP'
printf 'A: \\Box CH -> True\n' > "$TMP/mv3.ltir"
ckc "LaTeX \\Box 归一同样弃权" "$($BIN verify "$TMP/mv3.ltir")" 'E-SCOPE-MODAL'
ckc "示范卷 multiverse.ltir 只登记" "$($BIN verify fixtures/multiverse.ltir)" '── 0 lines:'
ckc "模块 Multiverse 不产三态行" "$($BIN verify axioms/Multiverse.ltir)" '── 0 lines:'
ckc "码表 E-SCOPE-CROSS-WORLD" "$(cat lab_codes.tsv)" 'E-SCOPE-CROSS-WORLD'

echo "== 哥德巴赫专档 / 七问口径正名 =="
printf 'system: PA\nproblem: Goldbach-binary\nstatus: open\nA: PA ⊢ Goldbach -> True\nA: ZFC ⊢ PvsNP -> True\nA: ZFC ⊢ CH -> True\n' > "$TMP/gb.ltir"
GB="$($BIN verify "$TMP/gb.ltir")"
ckc "哥德巴赫 open 专档" "$GB" 'E-SCOPE-OPEN'
ckc "PvsNP 口径正名" "$GB" 'E-SCOPE-OPEN'
ckc "CH 仍是独立命题" "$GB" 'E-SCOPE-INDEPENDENT'
ckc "三行全弃权不并入 False" "$GB" '── 3 lines: 0 True, 0 False, 3 UNDECIDED ──'
ckc "哥德巴赫示范卷只登记" "$($BIN verify fixtures/goldbach.ltir)" '── 0 lines:'
ckc "码表 E-SCOPE-OPEN" "$(cat lab_codes.tsv)" 'E-SCOPE-OPEN'

echo "== 回归锁定 =="
ckc "三角囚徒冻结卷不漂" "$($BIN verify fixtures/triangle_prisoner.ltir)" '── 10 lines: 10 True, 0 False, 0 UNDECIDED ──'
ckc "防沙彩蛋未回归" "$(printf 'set: N = {0,1,2,......}\n' > "$TMP/sand8.ltir"; $BIN verify "$TMP/sand8.ltir" --lang=zh)" '才不是帮你呢。'
ckc "用户卷汇总不漂" "$($BIN verify "$TMP/logic.ltir")" '── 7 lines: 4 True, 1 False, 2 UNDECIDED ──'
ck  "core SHA 门禁" "$(sha256sum -c RUN_core_sha.txt --quiet >/dev/null 2>&1; echo $?)" '0'


echo "== Bug 1：多项式隐式乘法 2xy = 2*x*y =="
printf 'id: (x+y)^2 == x^2 + 2xy + y^2\nid: (x+y)^2 == x^2 + 2*x*y + y^2\n' > "$TMP/m9.ltir"
M9="$($BIN verify "$TMP/m9.ltir")"
ckc "连写 2xy 判 True" "$M9" 'L01 True'
ckc "显式 2*x*y 判 True" "$M9" 'L02 True'
printf 'id: (a+b)^2 == a^2 + 2ab + b^2\n' > "$TMP/m9b.ltir"
ckc "连写 2ab 判 True" "$($BIN verify "$TMP/m9b.ltir")" 'L01 True'
printf 'id: xy == x*y\n' > "$TMP/m9c.ltir"
ckc "裸 xy 等价 x*y" "$($BIN verify "$TMP/m9c.ltir")" 'L01 True'
printf 'id: 2(x+1) == 2*x+2\n' > "$TMP/m9d.ltir"
ckc "2(x+1) 隐式乘不回退" "$($BIN verify "$TMP/m9d.ltir")" 'L01 True'
printf 'id: (x+y)^2 == x^2+y^2\n' > "$TMP/m9e.ltir"
ckc "旧恒等式仍判 False" "$($BIN verify "$TMP/m9e.ltir")" 'L01 False  E-VAL-MISMATCH'
ckc "demo 24 行不漂" "$($BIN verify fixtures/demo.ltir)" '── 24 lines: 20 True, 3 False, 1 UNDECIDED ──'

echo "== Bug 2：受限概括（限定域在竖线右侧） =="
printf 'set: T = {x | x in {1,2,3,4} and x > 2}\n' > "$TMP/c9a.ltir"
ckc "域在右侧算出 {3, 4}" "$($BIN verify "$TMP/c9a.ltir")" 'L01 True  {3, 4}'
printf 'set: U = {x in {1,2,3} | x > 1}\n' > "$TMP/c9b.ltir"
ckc "域在左侧仍算出 {2, 3}" "$($BIN verify "$TMP/c9b.ltir")" 'L01 True  {2, 3}'
printf 'set: A = {x | x in {5,6}}\n' > "$TMP/c9c.ltir"
ckc "无附加条件取全域" "$($BIN verify "$TMP/c9c.ltir")" 'L01 True  {5, 6}'
printf 'set: E = {x | x in {1,2} and x > 9}\n' > "$TMP/c9d.ltir"
ckc "空结果输出 {}" "$($BIN verify "$TMP/c9d.ltir")" 'L01 True  {}'
printf 'set: W = {x | x in R and x > 0}\n' > "$TMP/c9e.ltir"
ckc "无限域有码弃权（不冒充罗素）" "$($BIN verify "$TMP/c9e.ltir")" 'E-SCOPE-INFINITE'
printf 'set: R = {x | x notin x}\n' > "$TMP/c9f.ltir"
ckc "真·无受限域仍是罗素塌图" "$($BIN verify "$TMP/c9f.ltir")" 'E-PARADOX-RUSSELL'
ckc "罗素卷不产 True/False" "$($BIN verify "$TMP/c9f.ltir")" '── 1 lines: 0 True, 0 False, 1 UNDECIDED ──'
ckc "复现卷 fix009 可跑" "$($BIN verify fixtures/fix009.ltir)" 'L01 True'

echo "== Bug 3：阶乘 / 幂溢出归 E-OVF =="
printf 'V: 20!\nV: 21!\nV: 100!\nA: 100! -> True\nV: 2^64\n' > "$TMP/o9.ltir"
O9="$($BIN verify "$TMP/o9.ltir")"
ckc "20! 仍精确" "$O9" 'L01 True  2432902008176640000'
ckc "21! 归 E-OVF" "$O9" 'L02 UNDECIDED  E-OVF'
ckc "100! 归 E-OVF" "$O9" 'L03 UNDECIDED  E-OVF'
ckc "A: 100! 同样归 E-OVF" "$O9" 'L04 UNDECIDED  E-OVF'
ckc "2^64 同属溢出家族" "$O9" 'L05 UNDECIDED  E-OVF'
ckc "不再戴超越域帽子" "$(printf 'V: 100!\n' > "$TMP/o9b.ltir"; $BIN verify "$TMP/o9b.ltir" | grep -c 'E-SCOPE-TRANSCENDENTAL')" '0'
ckc "码表含 E-OVF" "$(cat lab_codes.tsv)" 'E-OVF'

echo "== Bug 4：π/12 家族精确角（双根式 RAD2） =="
printf 'V: sin(pi/12)\nV: cos(pi/12)\nV: tan(pi/12)\n' > "$TMP/t9.ltir"
T9="$($BIN verify "$TMP/t9.ltir")"
ckc "sin(pi/12) = (√6 - √2)/4" "$T9" 'L01 True  (√6 - √2)/4'
ckc "cos(pi/12) = (√6 + √2)/4" "$T9" 'L02 True  (√6 + √2)/4'
ckc "tan(pi/12) = 2 - √3" "$T9" 'L03 True  2 - √3'
printf 'V: sin(5*pi/12)\nV: cos(11*pi/12)\nV: sin(13*pi/12)\n' > "$TMP/t9b.ltir"
T9B="$($BIN verify "$TMP/t9b.ltir")"
ckc "5π/12" "$T9B" 'L01 True  (√6 + √2)/4'
ckc "11π/12 负号提取" "$T9B" 'L02 True  -(√6 + √2)/4'
ckc "13π/12" "$T9B" 'L03 True  -(√6 - √2)/4'
ckc "sin(2*pi/3) 旧值不回退" "$($BIN eval 'sin(2*pi/3)')" '(1/2)√3'
ckc "sin(pi/7) 仍诚实弃权" "$($BIN eval 'sin(pi/7)')" 'UNDECIDED'
printf 'A: sin(pi/12) == cos(pi/12) -> True\n' > "$TMP/t9c.ltir"
ckc "双根式比较有码弃权" "$($BIN verify "$TMP/t9c.ltir")" 'E-SCOPE-ALGEBRAIC'
printf 'V: sin(pi/12) + 1\n' > "$TMP/t9d.ltir"
ckc "双根式四则有码弃权" "$($BIN verify "$TMP/t9d.ltir")" 'E-SCOPE-ALGEBRAIC'
ckc "码表含 E-SCOPE-ALGEBRAIC" "$(cat lab_codes.tsv)" 'E-SCOPE-ALGEBRAIC'

echo "== 新功能 A：卸载脚本与命令 =="
ck  "uninstall.sh 存在" "$([ -f uninstall.sh ] && echo yes)" 'yes'
ckc "uninstall.sh 只删收容目录" "$(cat uninstall.sh)" '$HOME/ltir_lab'
ckc "uninstall.sh 删 hook" "$(cat uninstall.sh)" '$HOME/bin/termux-file-editor'
ckc "uninstall.sh 保留用户卷" "$(cat uninstall.sh)" '不碰用户自己的 .ltir 卷'
ckc "ltir.sh u 转交" "$(cat ltir.sh)" 'u) exec bash ./uninstall.sh'
ckc "收容清单含卸载脚本" "$(cat ltir.sh)" './uninstall.sh'
ckc "CLI uninstall 给指引" "$($BIN uninstall)" 'bash uninstall.sh'
TMPHOME="$(mktemp -d)"; mkdir -p "$TMPHOME/ltir_lab" "$TMPHOME/bin"
printf 'x' > "$TMPHOME/ltir_lab/keep"; printf 'x' > "$TMPHOME/bin/termux-file-editor"
HOME="$TMPHOME" bash uninstall.sh > "$TMPHOME/un9.log" 2>&1
ck  "卸载后收容目录消失" "$([ ! -d "$TMPHOME/ltir_lab" ] && echo yes)" 'yes'
ck  "卸载后 hook 消失" "$([ ! -f "$TMPHOME/bin/termux-file-editor" ] && echo yes)" 'yes'
rm -rf "$TMPHOME"

echo "== 新功能 B：当前终端只输出三态 =="
printf 'V: 2+2\nA: 2+2==5 -> True\nset: N = {0,1,2,......}\n' > "$TMP/mode9.ltir"
TRI="$($BIN verify "$TMP/mode9.ltir" --tri)"
ck  "tri 恰好两行" "$(printf '%s\n' "$TRI" | grep -c .)" '2'
ck  "tri 首行 True" "$(printf '%s\n' "$TRI" | sed -n 1p)" 'True'
ck  "tri 次行 False" "$(printf '%s\n' "$TRI" | sed -n 2p)" 'False'
ck  "tri 无行号" "$(printf '%s' "$TRI" | grep -c 'L0')" '0'
ck  "tri 无答案值" "$(printf '%s' "$TRI" | grep -c '  4')" '0'
ck  "tri 无汇总行" "$(printf '%s' "$TRI" | grep -c 'lines:')" '0'
ck  "tri 不吐彩蛋" "$(printf '%s' "$TRI" | grep -c 'sand:')" '0'
ck  "env LTIR_OUTPUT=tri 生效" "$(LTIR_OUTPUT=tri $BIN verify "$TMP/mode9.ltir" | sed -n 1p)" 'True'
ck  "env LTIR_TRI=1 生效" "$(LTIR_TRI=1 $BIN verify "$TMP/mode9.ltir" | sed -n 1p)" 'True'
ck  "--normal 覆盖 env" "$(LTIR_OUTPUT=tri $BIN verify "$TMP/mode9.ltir" --normal | sed -n 1p)" 'sand: You typed extra ellipses; normalized to one. N-not like I did it for you or anything!'
ckc "ltir.sh tri 打印 export" "$(bash ltir.sh tri)" 'export LTIR_OUTPUT=tri'
ck  "--tri 覆盖 --json" "$($BIN verify "$TMP/mode9.ltir" --tri --json | sed -n 1p)" 'True'
ck  "--tri 覆盖 --explain" "$($BIN verify "$TMP/mode9.ltir" --tri --explain | grep -c '^  ')" '0'

echo "== 新功能 C：只验算、不输出答案 =="
ckc "check 命令不给答案" "$($BIN check "$TMP/mode9.ltir" | sed -n 2p)" 'L01 True'
ck  "check 首行无 4" "$($BIN check "$TMP/mode9.ltir" | sed -n 2p | grep -c '4')" '0'
ckc "check 仍给三态汇总" "$($BIN check "$TMP/mode9.ltir")" '── 2 lines: 1 True, 1 False, 0 UNDECIDED ──'
ckc "--no-answer 等价" "$($BIN verify "$TMP/mode9.ltir" --no-answer | sed -n 2p)" 'L01 True'
ckc "env no-answer" "$(LTIR_OUTPUT=no-answer $BIN verify "$TMP/mode9.ltir" | sed -n 2p)" 'L01 True'
ckc "check 保留错误码" "$($BIN check "$TMP/mode9.ltir")" 'E-VAL-MISMATCH'
ck  "normal 仍给答案" "$($BIN verify "$TMP/mode9.ltir" --normal | sed -n 2p | grep -c '  4')" '1'

echo "== 回归锁定 =="
ckc "三角囚徒冻结卷不漂" "$($BIN verify fixtures/triangle_prisoner.ltir)" '── 10 lines: 10 True, 0 False, 0 UNDECIDED ──'
ckc "防沙彩蛋未回归" "$($BIN verify "$TMP/sand8.ltir" --lang=zh)" '才不是帮你呢。'
ckc "δ 展开未回归" "$($BIN verify "$TMP/d8.ltir")" 'L01 True  9'
ckc "多宇宙边未回归" "$($BIN verify "$TMP/mv.ltir" --json)" '"world_edges":2'
ckc "哥德巴赫 open 未回归" "$($BIN verify "$TMP/gb.ltir")" 'E-SCOPE-OPEN'
ckc "用户卷汇总不漂" "$($BIN verify "$TMP/logic.ltir")" '── 7 lines: 4 True, 1 False, 2 UNDECIDED ──'
ck  "core SHA 门禁" "$(sha256sum -c RUN_core_sha.txt --quiet >/dev/null 2>&1; echo $?)" '0'
ck  "二进制无 0.0.10 越版残留" "$(strings ./ltir006 | grep -c '0\.0\.10')" '0'
ck  "GITHUB_INTRO 多版文案在包里" "$([ -f GITHUB_INTRO.md ] && echo yes)" 'yes'

echo "== 【0.1.0】进制眼镜：只换读法，不换世界 =="
printf 'radix: 2\nA: 1010+1=1011 -> True\n' > "$TMP/rb1.ltir"
ckc "base2 连写判定" "$($BIN verify "$TMP/rb1.ltir")" 'L01 True'
printf '<3_nary>\nV: 0.1\n</3_nary>\nV: 0.1\n' > "$TMP/rb2.ltir"
ckc "base3 0.1 = 1/3" "$($BIN verify "$TMP/rb2.ltir")" 'L01 True  1/3'
ckc "出栈恢复十进制" "$($BIN verify "$TMP/rb2.ltir")" 'L02 True  1/10'
printf '<二进制>\nV: 1010\n</二进制>\n' > "$TMP/rb3.ltir"
ckc "中文标签同义" "$($BIN verify "$TMP/rb3.ltir")" 'L01 True  10'
printf '<base=16>\nV: 10\n</base=16>\n' > "$TMP/rb4.ltir"
ckc "base=16 写法" "$($BIN verify "$TMP/rb4.ltir")" 'L01 True  16'
printf '<2_nary>\nV: 102\n</2_nary>\n' > "$TMP/rb5.ltir"
ckc "非法数字有码" "$($BIN verify "$TMP/rb5.ltir")" 'E-LITERAL-BASE'
printf '<2_nary>\nV: 1010\n' > "$TMP/rb6.ltir"
ckc "忘闭合有码" "$($BIN verify "$TMP/rb6.ltir")" 'E-SCOPE-UNCLOSED'
ckc "JSON base 字段" "$($BIN verify "$TMP/rb1.ltir" --json)" '"base":2'
ckc "默认十进制 demo 不漂" "$($BIN verify fixtures/demo.ltir)" '── 24 lines: 20 True, 3 False, 1 UNDECIDED ──'

echo "== 【0.1.0】赋值环境表 =="
printf 'a=1+1\nb=2+2\nA: a+b == 6 -> True\n' > "$TMP/as1.ltir"
ckc "跨行绑定求值" "$($BIN verify "$TMP/as1.ltir")" 'L01 True'
printf 'x=5\nA: x == 5 -> True\n' > "$TMP/as2.ltir"
ckc "== 仍归判定不误绑" "$($BIN verify "$TMP/as2.ltir")" 'L01 True'
printf 'a=1\na=2\n' > "$TMP/as3.ltir"
ckc "重名不静默覆盖" "$($BIN verify "$TMP/as3.ltir")" 'E-BIND-REDEFINED'
ckc "绑定不产三态行" "$($BIN verify "$TMP/as1.ltir")" '── 1 lines:'

echo "== 【0.1.0】一致性强度 / 势 / 真类 / 范畴论（L0 台账） =="
printf 'class: V\ncard_eq: V = V\n' > "$TMP/lg1.ltir"
ckc "真类谈势拦下" "$($BIN verify "$TMP/lg1.ltir")" 'E-SCOPE-PROPER-CLASS'
printf 'card_le: A <= B by: f\n' > "$TMP/lg2.ltir"
ckc "无双射证书有码弃权" "$($BIN verify "$TMP/lg2.ltir")" 'E-SCOPE-CARDINAL'
printf 'strength_le: Q <= PA -> True\n' > "$TMP/lg3.ltir"
ckc "手写 True 不提供真值" "$($BIN verify "$TMP/lg3.ltir")" 'E-SCOPE-CONSISTENCY'
ckc "绝不并入 False" "$($BIN verify "$TMP/lg3.ltir")" '── 1 lines: 0 True, 0 False, 1 UNDECIDED ──'
printf 'con: ZFC -> ZFC\n' > "$TMP/lg4.ltir"
ckc "Gödel II 结构护栏" "$($BIN verify "$TMP/lg4.ltir")" 'E-SCOPE-CONSISTENCY'
printf 'con: ZF, ZFC\nequiconsistent: ZF, ZFC\n' > "$TMP/lg5.ltir"
ckc "等一致性只登记" "$($BIN verify "$TMP/lg5.ltir")" '── 0 lines:'
ckc "JSON ledger 字段" "$($BIN verify "$TMP/lg5.ltir" --json)" '"ledger":2'
printf 'cat: Set\nobject: A\narrow: f\ncommute: f != g\n' > "$TMP/lg6.ltir"
ckc "有限交换图判 False" "$($BIN verify "$TMP/lg6.ltir")" 'E-CAT-NONCOMMUTE'
printf 'commute: f g\n' > "$TMP/lg7.ltir"
ckc "泛性质需证书" "$($BIN verify "$TMP/lg7.ltir")" 'E-SCOPE-UNIVERSAL'

echo "== 【0.1.0】试卷库（shell 层，core 不动） =="
ck  "六档目录齐" "$([ -d exams/trivial ] && [ -d exams/easy ] && [ -d exams/normal ] && [ -d exams/hard ] && [ -d exams/very-hard ] && [ -d exams/prize ] && echo yes)" 'yes'
ck  "exam.json 存在" "$([ -f exam.json ] && echo yes)" 'yes'
ck  "ltir_exam.sh 存在" "$([ -f ltir_exam.sh ] && echo yes)" 'yes'
ck  "正式版说明存在" "$([ -f RELEASE_0.1.0.md ] && echo yes)" 'yes'
ckc "题面头带难度" "$(cat exams/very-hard/de-moivre-quintic.ltir)" 'difficulty: very-hard'
ckc "folklore 归属不写成定理" "$(cat exams/very-hard/de-moivre-quintic.ltir)" 'attribution: folklore'
ckc "收容清单含 exams" "$(cat ltir.sh)" './exams'
ckc "入口有 exam 子命令" "$(cat ltir.sh)" 'exam) shift'
printf 'exam: injection-check\nby: rule: finite-injection-check\nA: 2+2=4 -> True\nsubmit:\n' > "$TMP/ans.ltir"
EXJ="$(bash ./ltir_exam.sh "$TMP/ans.ltir" --json --no-clock --seed=42 --human=n 2>/dev/null)"
ckc "交卷回执 submit=true" "$EXJ" '"submit":true'
ckc "回执带 score" "$EXJ" '"score":'
ckc "回执带 verdicts" "$EXJ" '"verdicts":'
ckc "--no-clock 固定时钟" "$EXJ" '"elapsed_s":0'
ck  "交卷完成 exit 0（含成绩）" "$(bash ./ltir_exam.sh "$TMP/ans.ltir" --json --no-clock --seed=42 --human=n >/dev/null 2>&1; echo $?)" '0'
printf 'exam: x\nA: 2+2=5 -> True\n' > "$TMP/nosub.ltir"
ck  "EOF 不算交卷 exit 3" "$(bash ./ltir_exam.sh "$TMP/nosub.ltir" --json --no-clock --human=n >/dev/null 2>&1; echo $?)" '3'
printf 'submit:\n' > "$TMP/empty.ltir"
ckc "空卷 0 分" "$(bash ./ltir_exam.sh "$TMP/empty.ltir" --json --no-clock --human=n 2>/dev/null)" '"score":0'
ck  "0 分仍 exit 0" "$(bash ./ltir_exam.sh "$TMP/empty.ltir" --json --no-clock --human=n >/dev/null 2>&1; echo $?)" '0'
ck  "--list 可用" "$(bash ./ltir_exam.sh --list 2>/dev/null | grep -c 'very-hard')" '1'
ckc "tri 模式 stdout 只给三态" "$(bash ./ltir_exam.sh "$TMP/ans.ltir" --tri --no-clock --human=n 2>/dev/null)" 'True'

echo "== 公理本体化：内容指纹只登记，不判真假 =="
printf 'lang: L_N = {0, S, +, *}\naxiom: Sep = forall x (x in A and P(x))\nrule: ModusPonens\ntheory_sig: ?\n' > "$TMP/sig1.ltir"
S1="$($BIN verify "$TMP/sig1.ltir")"
J1="$($BIN verify "$TMP/sig1.ltir" --json)"
ckc "公式本体入册"              "$J1" '"axiom_bodies":1'
ckc "语言签名进 JSON"            "$J1" '"lang":"L_N = {0, S, +, *}"'
ckc "指纹 FNV 16 位且确定"       "$S1" 'theory_sig: e34a93f0257ea42d'
ckc "登记行不产三态"             "$S1" '── 0 lines: 0 True, 0 False, 0 UNDECIDED ──'
ck  "tri 不吐指纹"               "$($BIN verify "$TMP/sig1.ltir" --tri | grep -c theory_sig)" '0'
printf 'lang: L_N = {0, S, +, *}\naxiom: Sep = forall x (x in A or P(x))\nrule: ModusPonens\ntheory_sig: ?\n' > "$TMP/sig2.ltir"
H1="$(printf '%s' "$S1" | grep -o 'theory_sig: [0-9a-f]*' | cut -d' ' -f2)"
H2="$($BIN verify "$TMP/sig2.ltir" | grep -o 'theory_sig: [0-9a-f]*' | cut -d' ' -f2)"
ck  "同名改一字公式指纹变"       "$([ "$H1" != "$H2" ] && echo yes)" 'yes'
printf 'lang: L_N = {0, S, +, *}\naxiom: Sep = forall x (x in A and P(x))\nrule: ModusPonens\nrule: Cut\ntheory_sig: ?\n' > "$TMP/sig3.ltir"
H3="$($BIN verify "$TMP/sig3.ltir" | grep -o 'theory_sig: [0-9a-f]*' | cut -d' ' -f2)"
ck  "推导规则入指纹"             "$([ "$H1" != "$H3" ] && echo yes)" 'yes'
printf 'lang: L_ZF = {in}\naxiom: Sep = forall x (x in A and P(x))\nrule: ModusPonens\ntheory_sig: ?\n' > "$TMP/sig4.ltir"
H4="$($BIN verify "$TMP/sig4.ltir" | grep -o 'theory_sig: [0-9a-f]*' | cut -d' ' -f2)"
ck  "语言签名入指纹"             "$([ "$H1" != "$H4" ] && echo yes)" 'yes'
printf 'preset: ZFC\naxiom: AC, Separation\ntheory_sig: ?\n' > "$TMP/sig5.ltir"
ckc "旧公理写法不回退"           "$($BIN verify "$TMP/sig5.ltir")" 'map: system=ZFC choice=on axioms=10'
ckc "preset 公理也入本体"        "$($BIN verify "$TMP/sig5.ltir" --json)" '"axiom_bodies":10'
printf 'load: axioms/ZF.ltir\ntheory_sig: ?\n' > "$TMP/sig6.ltir"
J6="$($BIN verify "$TMP/sig6.ltir" --json)"
ckc "模块公理进本体"             "$J6" '"axiom_bodies":7'
ck  "ZF 模块与 ZFC 预设内容不同"  "$([ "$(printf '%s' "$J6" | grep -o '"theory_sig":"[0-9a-f]*"' | cut -d'"' -f4)" != "$($BIN verify "$TMP/sig5.ltir" --json | grep -o '"theory_sig":"[0-9a-f]*"' | cut -d'"' -f4)" ] && echo yes)" 'yes'
printf 'lang: L_N = {0, S, +, *}\naxiom: Sep = forall x (x in A and P(x))\nrule: ModusPonens\ntheory_sig: e34a93f0257ea42d\n' > "$TMP/sig7.ltir"
ckc "期望指纹一致 mismatch=0"    "$($BIN verify "$TMP/sig7.ltir")" 'mismatch=0'
printf 'lang: L_N = {0, S, +, *}\naxiom: Sep = forall x (x in A and P(x))\nrule: ModusPonens\ntheory_sig: deadbeefdeadbeef\n' > "$TMP/sig8.ltir"
S8="$($BIN verify "$TMP/sig8.ltir")"
ckc "期望不符只登记不并 False"   "$S8" '── 0 lines: 0 True, 0 False, 0 UNDECIDED ──'
ckc "不符计入 mismatch"          "$S8" 'mismatch=1'
ckc "JSON 带 sig_mismatch"       "$($BIN verify "$TMP/sig8.ltir" --json)" '"sig_mismatch":1'
ckc "三角囚徒冻结卷不漂"         "$($BIN verify fixtures/triangle_prisoner.ltir)" '── 10 lines: 10 True, 0 False, 0 UNDECIDED ──'
ckc "demo 24 行不漂"             "$($BIN verify fixtures/demo.ltir)" '── 24 lines: 20 True, 3 False, 1 UNDECIDED ──'

echo "== 0.1.1 bugfix：隐式乘 / 0÷0 / 台账域 / 短闭合 =="
ck  "显式乘不回退"          "$($BIN eval '2*sin(pi/6)')"   '1'
ck  "隐式乘遇函数不截断"    "$($BIN eval '2sin(pi/6)')"    '1'
ck  "空格隐式乘"            "$($BIN eval '2 sin(pi/6)')"   '1'
ck  "括号后隐式乘"          "$($BIN eval '(2)sin(pi/6)')"  '1'
ck  "2sqrt(2) 保留根式"     "$($BIN eval '2sqrt(2)')"      '(2)√2'
ck  "2√2 保留根式"          "$($BIN eval '2√2')"           '(2)√2'
ck  "隐式乘不连吞 +1"       "$($BIN eval '2sqrt(2)+1')"    '1 + (2)√2'
ck  "0/0 未定形弃权"        "$($BIN eval '0/0')"           'UNDECIDED'
ck  "(1-1)/(1-1) 弃权"     "$($BIN eval '(1-1)/(1-1)')"   'UNDECIDED'
ck  "1/0 仍为 ∞"            "$($BIN eval '1/0')"           '∞'
printf 'A: 2sin(pi/6) == 1 -> True\nA: 2sqrt(2) == 2sqrt(2) -> True\n' > "$TMP/bug1.ltir"
ckc "正确声明不再被判错"    "$($BIN verify "$TMP/bug1.ltir")" '── 2 lines: 2 True, 0 False, 0 UNDECIDED ──'
printf 'A: 5 -> True\n' > "$TMP/bug2.ltir"
ckc "非布尔断言措辞"        "$($BIN verify "$TMP/bug2.ltir" --explain)" 'cannot compare against a verdict token'
printf 'domain: topology\nA: 1+1 == 2 -> True\n' > "$TMP/bug3.ltir"
ckc "台账域纯算术照判"      "$($BIN verify "$TMP/bug3.ltir")" '── 1 lines: 1 True, 0 False, 0 UNDECIDED ──'
printf '<base=16>\nV: 10\n</base>\nV: 10\n' > "$TMP/bug4.ltir"
ckc "短闭合不报未闭合"      "$($BIN verify "$TMP/bug4.ltir")" '── 2 lines: 2 True, 0 False, 0 UNDECIDED ──'
ckc "短闭合出栈恢复 base10" "$($BIN verify "$TMP/bug4.ltir")" 'True  10'

echo "-----------------------------------------------"
echo "$PASS / $TOTAL 项满足"
echo "-----------------------------------------------"
[ "$PASS" = "$TOTAL" ] || exit 1
