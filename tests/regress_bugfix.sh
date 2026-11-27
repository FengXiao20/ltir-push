#!/usr/bin/env bash
# ltir006/tests/regress_bugfix.sh —— 0.1.1 外部实测 bug 回归组（B1–B6）
# 来源：2026-11-27 上手测试报告（limit 死代码 / 进制自转换 / solve 重根 /
#       深嵌套崩溃 / inj 声明洗涤 / exam --json 契约）
# 纪律：先 RED 后 GREEN；每条断言对应一个已复现 bug 的最小行为。
set -u
cd "$(cd "$(dirname "$0")/.." && pwd)"
BIN=./ltir006
PASS=0; TOTAL=0
TMP="$(mktemp -d "${TMPDIR:-/tmp}/ltir_regress.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT
ck() { TOTAL=$((TOTAL+1)); if [ "$2" = "$3" ]; then PASS=$((PASS+1));
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; else
  printf '  [%02d] ✗ %s   got=<%s> want=<%s>\n' "$TOTAL" "$1" "$2" "$3"; fi; }
ckc() { TOTAL=$((TOTAL+1)); if printf '%s' "$2" | grep -qF -- "$3"; then PASS=$((PASS+1));
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; else
  printf '  [%02d] ✗ %s   missing=<%s>\n' "$TOTAL" "$1" "$3"; fi; }
ckn() { TOTAL=$((TOTAL+1)); if printf '%s' "$2" | grep -qF -- "$3"; then
  printf '  [%02d] ✗ %s   unexpected=<%s>\n' "$TOTAL" "$1" "$3"; else PASS=$((PASS+1));
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; fi; }

echo "== B2 进制眼镜：radix 指令参数不得被当前进制自转换 =="
printf 'V: 12\nradix: 3\nV: 12\nradix: 10\nV: 12\nV: 0.1\n' > "$TMP/b2a.ltir"
O="$($BIN verify "$TMP/b2a.ltir")"
ckc "3 进制下 12 读 5" "$O" 'L02 True  5'
ckc "radix: 10 切回十进制：12 读 12" "$O" 'L03 True  12'
ckc "切回后 0.1 读 1/10" "$O" 'L04 True  1/10'
printf 'radix: 3\nradix: 16\nV: 10\n' > "$TMP/b2b.ltir"
O="$($BIN verify "$TMP/b2b.ltir"; echo "RC=$?")"
ckc "3 进制下 radix: 16 不报 E-LITERAL-BASE" "$O" 'RC=0'
ckc "16 进制下 10 读 16" "$O" 'L01 True  16'
printf 'radix: 2\nV: 11\nradix: 10\nV: 11\n' > "$TMP/b2c.ltir"
O="$($BIN verify "$TMP/b2c.ltir")"
ckc "2 进制 11 读 3" "$O" 'L01 True  3'
ckc "切回十进制 11 读 11" "$O" 'L02 True  11'

echo "== B3 solve：重根（判别式=0）不得输出空开区间冒充解集 =="
printf 'solve: x^2 < 0\n' > "$TMP/b3a.ltir"
O="$($BIN verify "$TMP/b3a.ltir")"
ckc "x^2<0 解集为空：x ∈ ∅" "$O" 'x ∈ ∅'
ckn "x^2<0 不得输出 (0, 0)" "$O" '(0, 0)'
printf 'solve: x^2 <= 0\n' > "$TMP/b3b.ltir"
O="$($BIN verify "$TMP/b3b.ltir")"
ckc "x^2<=0 解为单点 x = 0" "$O" 'x = 0'
ckn "x^2<=0 不得输出开区间 (0, 0)" "$O" '(0, 0)'
printf 'solve: x^2 >= 0\n' > "$TMP/b3c.ltir"
ckc "x^2>=0 解为全体实数" "$($BIN verify "$TMP/b3c.ltir")" 'x ∈ R'
printf 'solve: 0-x^2 > 0\n' > "$TMP/b3d.ltir"
ckc "-x^2>0 解集为空" "$($BIN verify "$TMP/b3d.ltir")" 'x ∈ ∅'
printf 'solve: 0-x^2 <= 0\n' > "$TMP/b3e.ltir"
ckc "-x^2<=0 解为全体实数" "$($BIN verify "$TMP/b3e.ltir")" 'x ∈ R'
printf 'solve: x^2 = 0\n' > "$TMP/b3f.ltir"
O="$($BIN verify "$TMP/b3f.ltir")"
ckc "x^2=0 单根不重复罗列" "$O" 'x = 0'
ckn "x^2=0 不输出 0 or 0" "$O" 'or'
printf 'solve: (x-1)^2 < 0\n' > "$TMP/b3g.ltir"
ckc "(x-1)^2<0 解集为空" "$($BIN verify "$TMP/b3g.ltir")" 'x ∈ ∅'
printf 'solve: x^2 > 0\nsolve: x^2-5x+6 < 0\nsolve: x^2 < 1\n' > "$TMP/b3h.ltir"
O="$($BIN verify "$TMP/b3h.ltir")"
ckc "回归：x^2>0 仍为去心双区间" "$O" '(-∞, 0) ∪ (0, ∞)'
ckc "回归：x^2-5x+6<0 仍为 (2, 3)" "$O" 'x ∈ (2, 3)'
ckc "回归：x^2<1 仍为 (-1, 1)" "$O" 'x ∈ (-1, 1)'

echo "== B1 limit：分析极限不被范畴论登记表拦截，且数学正确 =="
printf 'limit: n->inf of 1/n\n' > "$TMP/b1a.ltir"
O="$($BIN verify "$TMP/b1a.ltir")"
ckc "limit 1/n 出判定（不再被登记吞掉）" "$O" '1 lines: 1 True'
ckc "limit 1/n = 0" "$O" 'L01 True  0'
printf 'limit: n->inf of (3n+1)/(2n+5)\n' > "$TMP/b1b.ltir"
ckc "同次有理式取首项系数比 3/2" "$($BIN verify "$TMP/b1b.ltir")" 'L01 True  3/2'
printf 'limit: n->inf of (n^2+1)/(n+1)\n' > "$TMP/b1c.ltir"
O="$($BIN verify "$TMP/b1c.ltir")"
ckc "分子高次 → ∞（不是 0）" "$O" '∞'
ckn "分子高次不得判 0" "$O" 'L01 True  0'
printf 'limit: n->inf of (n+1)/(n^2+1)\n' > "$TMP/b1d.ltir"
ckc "分母高次 → 0" "$($BIN verify "$TMP/b1d.ltir")" 'L01 True  0'
printf 'limit: x->0 of (x+1)/(x+2)\n' > "$TMP/b1e.ltir"
ckc "有限趋向点直接代入 1/2" "$($BIN verify "$TMP/b1e.ltir")" 'L01 True  1/2'
printf 'limit: x->1 of (x^2-1)/(x-1)\n' > "$TMP/b1f.ltir"
ckc "0/0 型诚实弃权 E-INDETERMINATE" "$($BIN verify "$TMP/b1f.ltir")" 'E-INDETERMINATE'
printf 'limit: n->inf of 5\nlimit: n->inf of n\n' > "$TMP/b1g.ltir"
O="$($BIN verify "$TMP/b1g.ltir")"
ckc "常数列极限为自身" "$O" 'L01 True  5'
ckc "n→∞ 发散记 ∞" "$O" 'L02 True  ∞'
printf 'limit: F\n' > "$TMP/b1h.ltir"
O="$($BIN verify "$TMP/b1h.ltir")"
ckc "范畴论 limit: F 仍只登记" "$O" '0 lines:'
ckc "范畴论 limit 计入 registered" "$O" 'registered: 1'

echo "== B4 深嵌套：递归深度护栏，弃权不崩溃 =="
python3 -c "print('V: ' + '('*5000 + '1' + ')'*5000)" > "$TMP/b4a.ltir" 2>/dev/null || \
  awk 'BEGIN{printf "V: ";for(i=0;i<5000;i++)printf "(";printf "1";for(i=0;i<5000;i++)printf ")";print ""}' > "$TMP/b4a.ltir"
O="$(timeout 10 $BIN verify "$TMP/b4a.ltir" 2>&1; echo "RC=$?")"
ckc "5000 层括号不崩溃（RC=0）" "$O" 'RC=0'
ckc "5000 层括号诚实弃权" "$O" 'UNDECIDED'
O="$(timeout 10 $BIN eval "$(awk 'BEGIN{for(i=0;i<5000;i++)printf "(";printf "1";for(i=0;i<5000;i++)printf ")"}')" 2>&1; echo "RC=$?")"
ckc "eval 深嵌套不崩溃" "$O" 'RC=0'
ckc "eval 深嵌套弃权" "$O" 'UNDECIDED'
printf 'V: %s1%s\n' "$(awk 'BEGIN{for(i=0;i<300;i++)printf "("}')" "$(awk 'BEGIN{for(i=0;i<300;i++)printf ")"}')" > "$TMP/b4b.ltir"
ckc "300 层正常算出 1" "$($BIN verify "$TMP/b4b.ltir")" 'L01 True  1'

echo "== B5 inj/surj/bij：有限映射的手写声明洗涤后照算 =="
MP='mapping: f = {1->2, 2->3, 3->2} on {2,3}'
printf '%s\ninj: f -> True\n' "$MP" > "$TMP/b5a.ltir"
O="$($BIN verify "$TMP/b5a.ltir")"
ckc "inj: f -> True 洗声明实算 False" "$O" 'L01 False'
ckc "声明冲突挂 E-VAL-MISMATCH" "$O" 'E-VAL-MISMATCH'
ckn "有限映射不得误挂 E-SCOPE-INFINITE" "$O" 'E-SCOPE-INFINITE'
printf '%s\ninj: f -> False\nsurj: f -> True\nbij: f -> False\n' "$MP" > "$TMP/b5b.ltir"
O="$($BIN verify "$TMP/b5b.ltir")"
ckc "声明与实算一致判 True（inj）" "$O" 'L01 True'
ckc "声明与实算一致判 True（surj）" "$O" 'L02 True'
ckc "声明与实算一致判 True（bij）" "$O" 'L03 True'
printf '%s\ninj: f\n' "$MP" > "$TMP/b5c.ltir"
ckc "回归：裸 inj: f 仍实算 False" "$($BIN verify "$TMP/b5c.ltir")" 'L01 False'
printf 'inj: R -> True\n' > "$TMP/b5d.ltir"
ckc "无限域 inj: R 仍弃权 E-SCOPE-INFINITE" "$($BIN verify "$TMP/b5d.ltir")" 'E-SCOPE-INFINITE'

echo "== B6 exam --json：stdout 只给一条 JSON 回执，题面走 stderr =="
EO="$(printf 'A: 1+1=2 -> True\nsubmit:\n' | bash ltir_exam.sh --json --no-clock --human=n --seed=42 2>"$TMP/b6.err")"
ckn "json 模式 stdout 不含题面文本" "$EO" 'problem:'
ckc "json 模式 stdout 含唯一回执" "$EO" '"exam_id"'
ck "json 模式 stdout 恰好一行" "$(printf '%s\n' "$EO" | grep -c '[^[:space:]]')" "1"
ckc "题面改走 stderr" "$(cat "$TMP/b6.err")" 'problem:'
bash ltir_exam.sh --json --no-clock --human=n --seed=42 </dev/null >"$TMP/b6n.out" 2>/dev/null
RC=$?
ck "未交卷仍 exit 3" "$RC" "3"
ckc "未交卷给 JSON 错误回执" "$(cat "$TMP/b6n.out")" '"submit":false'

echo "-----------------------------------------------"
echo "$PASS / $TOTAL 项满足"
echo "-----------------------------------------------"
[ "$PASS" = "$TOTAL" ]
