#!/usr/bin/env bash
# ltir006/tests/smoke006.sh —— 0.0.6 SMOKE（独立于 JS/Python 历史套件）
# Linux: g++  / Termux: clang++（由 ltir006.sh 传入）
set -u
cd "$(cd "$(dirname "$0")/.." && pwd)"
BIN=./ltir006
PASS=0; TOTAL=0
ck() { TOTAL=$((TOTAL+1)); if [ "$2" = "$3" ]; then PASS=$((PASS+1));
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; else
  printf '  [%02d] ✗ %s   got=<%s> want=<%s>\n' "$TOTAL" "$1" "$2" "$3"; fi; }
ckc() { TOTAL=$((TOTAL+1)); if printf '%s' "$2" | grep -qF -- "$3"; then PASS=$((PASS+1));
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; else
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

echo "== 版本（0.0.6 强化修复版：主版本号不变，后缀随本令） =="
ck  "0.0.6 强化修复版" "$($BIN --version)" 'ltir006 0.0.6 强化修复版'

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
ckc "领域表 24 个（出些领域）" "$($BIN verify "$TMP/test_006.ltir" --json)" '"domains_registered":24'
ckc "<subject domain=> 真的进 Ctx" "$($BIN verify "$TMP/bind.ltir" --json)" '"domain":"algebra"'
ckc "domain 属性被记录" "$($BIN verify "$TMP/bind.ltir")" 'domain: algebra (24 registered)'
printf 'domain: ?\n' > "$TMP/dlist.ltir"
ckc "domain: ? 列出已注册领域" "$($BIN verify "$TMP/dlist.ltir")" '24 domains registered'
printf 'domain: trigonometry\nA: sin(pi/6) = 1/2 -> True\n' > "$TMP/dset.ltir"
ckc "顶层 domain: 显式设定领域" "$($BIN verify "$TMP/dset.ltir" --json)" '"domain":"trigonometry"'
printf 'domain: 冷门新领域\n' > "$TMP/dnew.ltir"
ckc "未知领域标黄放行不报错" "$($BIN verify "$TMP/dnew.ltir"; echo "RC=$?")" 'RC=0'
ckc "函数定义域查询未回归" "$($BIN verify fixtures/demo.ltir)" 'L01 True  R'

echo "== 【强化修复版】clang 死变量复核（Termux 真机抓到的那条） =="
ck  "value.hpp 无 den,t 残留" "$(grep -c 'den,t;' src/value.hpp)" '0'
ck  "编译产物存在" "$([ -x ./ltir006 ] && echo yes)" 'yes'

echo "-----------------------------------------------"
echo "$PASS / $TOTAL 项满足"
echo "-----------------------------------------------"
[ "$PASS" = "$TOTAL" ] || exit 1
