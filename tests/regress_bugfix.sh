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

echo "== B7 solve：非重根 <=/>= 解集端点必须闭合 =="
printf 'solve: x^2 <= 1\n' > "$TMP/b7a.ltir"
O="$($BIN verify "$TMP/b7a.ltir")"
ckc "x^2<=1 闭区间 [-1, 1]" "$O" 'x ∈ [-1, 1]'
ckn "x^2<=1 不得丢端点输出 (-1, 1)" "$O" '(-1, 1)'
printf 'solve: x^2 >= 1\n' > "$TMP/b7b.ltir"
ckc "x^2>=1 外侧闭端点" "$($BIN verify "$TMP/b7b.ltir")" 'x ∈ (-∞, -1] ∪ [1, ∞)'
printf 'solve: x^2-5x+6 <= 0\nsolve: x^2-5x+6 >= 0\n' > "$TMP/b7c.ltir"
O="$($BIN verify "$TMP/b7c.ltir")"
ckc "x^2-5x+6<=0 → [2, 3]" "$O" 'x ∈ [2, 3]'
ckc "x^2-5x+6>=0 → (-∞, 2] ∪ [3, ∞)" "$O" 'x ∈ (-∞, 2] ∪ [3, ∞)'
printf 'solve: 0-x^2 >= -1\n' > "$TMP/b7d.ltir"
ckc "负首项 -x^2>=-1 → [-1, 1]" "$($BIN verify "$TMP/b7d.ltir")" 'x ∈ [-1, 1]'
printf 'solve: x^2 < 1\nsolve: x^2 > 1\n' > "$TMP/b7e.ltir"
O="$($BIN verify "$TMP/b7e.ltir")"
ckc "回归：严格 < 仍开区间 (-1, 1)" "$O" 'x ∈ (-1, 1)'
ckc "回归：严格 > 仍开区间外侧" "$O" 'x ∈ (-∞, -1) ∪ (1, ∞)'

echo "== S1 sqrt 负数：完全平方负数走复数，非完全平方诚实弃权 =="
ck "sqrt(-1) = i" "$($BIN eval 'sqrt(-1)')" "i"
ck "sqrt(-4) = 2i" "$($BIN eval 'sqrt(-4)')" "2i"
ck "sqrt(-2) 根式×i 混合超纲弃权" "$($BIN eval 'sqrt(-2)')" "UNDECIDED"
ck "sqrt(-1)^2 = -1" "$($BIN eval 'sqrt(-1)^2')" "-1"
ck "回归：sqrt(4) = 2" "$($BIN eval 'sqrt(4)')" "2"

echo "== S2 C/P：k>n 按组合学惯例归 0 =="
ck "C(5,6) = 0" "$($BIN eval 'C(5,6)')" "0"
ck "P(5,6) = 0" "$($BIN eval 'P(5,6)')" "0"
ck "回归：C(5,2) = 10" "$($BIN eval 'C(5,2)')" "10"
ck "回归：C(-1,2) 仍弃权" "$($BIN eval 'C(-1,2)')" "UNDECIDED"

echo "== S3 负整数指数 =="
ck "2^-1 = 1/2" "$($BIN eval '2^-1')" "1/2"
ck "2^-3 = 1/8" "$($BIN eval '2^-3')" "1/8"
ck "(1/2)^-2 = 4" "$($BIN eval '(1/2)^-2')" "4"
ck "回归：2^2^3 = 256（右结合）" "$($BIN eval '2^2^3')" "256"
ck "回归：-2^2 = -4" "$($BIN eval '-2^2')" "-4"
printf 'at: x=3\nV: 2x^2\n' > "$TMP/s3bind.ltir"
ckc "回归：PATCH009 幂绑定 2x^2=18" "$($BIN verify "$TMP/s3bind.ltir")" 'L01 True  18'

echo "== S6 eval factors 与 N: 口径一致 =="
ck "eval factors(360) 输出分解串" "$($BIN eval 'factors(360)')" '2^3 × 3^2 × 5'
ck "eval factors(97) = 97" "$($BIN eval 'factors(97)')" "97"
ck "eval factors(1) = 1（空分解，与 N: 一致）" "$($BIN eval 'factors(1)')" "1"
printf 'N: factors(360)\n' > "$TMP/s6n.ltir"
ckc "回归：N: factors(360) 不变" "$($BIN verify "$TMP/s6n.ltir")" '2^3'

echo "== D4 JSON 补文件行号 file_line =="
printf '# comment\nV: 1+1\ndefine: f(x)=x\nV: 2+2\n' > "$TMP/d4.ltir"
O="$($BIN verify "$TMP/d4.ltir" --json)"
ckc "第一条判定 file_line=2" "$O" '"file_line":2'
ckc "第二条判定 file_line=4" "$O" '"file_line":4'
ckc "判定序号 n 不变" "$O" '"n":1'

echo "== ST 强度草案补漏：v2 纯名字公理不得指纹碰撞 / 非法 sig_mode 给码 =="
printf 'axiom: AC, Separation, Replacement\nsig_mode: formula\ntheory_sig: ?\n' > "$TMP/st1.ltir"
printf 'axiom: Choice, Foundation, Infinity\nsig_mode: formula\ntheory_sig: ?\n' > "$TMP/st2.ltir"
S1="$($BIN verify "$TMP/st1.ltir" --json | grep -o '"theory_sig":"[0-9a-f]*"')"
S2="$($BIN verify "$TMP/st2.ltir" --json | grep -o '"theory_sig":"[0-9a-f]*"')"
ck "v2 纯名字公理：不同理论指纹必须不同" "$([ "$S1" != "$S2" ] && echo yes)" "yes"
printf 'lang: L = {0 ; S}\naxiom: Sep = forall n (n = n)\nsig_mode: formula\ntheory_sig: ?\n' > "$TMP/st3.ltir"
printf 'lang: L = {0 ; S}\naxiom: Q = forall n (n = n)\nsig_mode: formula\ntheory_sig: ?\n' > "$TMP/st4.ltir"
S3="$($BIN verify "$TMP/st3.ltir" --json | grep -o '"theory_sig":"[0-9a-f]*"')"
S4="$($BIN verify "$TMP/st4.ltir" --json | grep -o '"theory_sig":"[0-9a-f]*"')"
ck "回归：v2 同公式不同名仍同指纹" "$([ "$S3" = "$S4" ] && echo yes)" "yes"
printf 'sig_mode: banana\n' > "$TMP/st5.ltir"
O="$($BIN verify "$TMP/st5.ltir"; echo "RC=$?")"
ckc "非法 sig_mode 值给 E-SYN" "$O" 'E-SYN'
ckc "非法 sig_mode 硬错误 exit 1" "$O" 'RC=1'
printf 'sig_mode: formula\nsig_mode: named\nV: 1+1\n' > "$TMP/st6.ltir"
ckc "回归：合法开关切换仍只登记" "$($BIN verify "$TMP/st6.ltir")" 'L01 True  2'

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

echo "== B8 solve：判别式<0 恒号，不得弃权 =="
printf 'solve: x^2 + 1 >= 0\n' > "$TMP/b8a.ltir"
O="$($BIN verify "$TMP/b8a.ltir")"
ckc "x^2+1>=0 → 全体实数 R" "$O" 'x ∈ R'
ckn "B8a 不再 UNDECIDED" "$($BIN verify "$TMP/b8a.ltir" --tri)" 'UNDECIDED'
printf 'solve: x^2 + 1 <= 0\n' > "$TMP/b8b.ltir"
O="$($BIN verify "$TMP/b8b.ltir")"
ckc "x^2+1<=0 → 空集" "$O" 'x ∈ ∅'
printf 'solve: -x^2 - 1 <= 0\n' > "$TMP/b8c.ltir"
O="$($BIN verify "$TMP/b8c.ltir")"
ckc "-x^2-1<=0 → 全体实数 R" "$O" 'x ∈ R'
printf 'solve: x^2 + 1 = 0\n' > "$TMP/b8d.ltir"
ckc "x^2+1=0 → 空集（无实根）" "$($BIN verify "$TMP/b8d.ltir")" 'x ∈ ∅'
printf 'solve: -x^2 - 1 >= 0\n' > "$TMP/b8e.ltir"
ckc "-x^2-1>=0 → 空集" "$($BIN verify "$TMP/b8e.ltir")" 'x ∈ ∅'
printf 'solve: 2x^2 + 3 <= 0\n' > "$TMP/b8f.ltir"
ckc "2x^2+3<=0 → 空集" "$($BIN verify "$TMP/b8f.ltir")" 'x ∈ ∅'
printf 'solve: x^2 - 1 >= 0\n' > "$TMP/b8g.ltir"
ckc "B7 端点闭合零回归" "$($BIN verify "$TMP/b8g.ltir")" '(-∞, -1]'

echo "== B9 divv：RAT/RAT 不再被分母有理化撑爆 =="
printf 'V: 1/110000000000000000\n' > "$TMP/b9a.ltir"
ckc "1/1.1e17 精确出分数" "$($BIN verify "$TMP/b9a.ltir")" 'L01 True  1/110000000000000000'
printf 'V: 1/1000000000000 + 1/1000000000000\n' > "$TMP/b9b.ltir"
ckc "大分母加法仍精确" "$($BIN verify "$TMP/b9b.ltir")" 'L01 True  1/500000000000'
printf 'V: 1/(1+i)\n' > "$TMP/b9c.ltir"
ckc "复数分母有理化不回退" "$($BIN verify "$TMP/b9c.ltir")" 'L01 True'
printf 'V: 0/0\n' > "$TMP/b9d.ltir"
ckc "0/0 仍归 E-INDETERMINATE" "$($BIN verify "$TMP/b9d.ltir" --json)" 'E-INDETERMINATE'
printf 'V: 1/0\n' > "$TMP/b9e.ltir"
ckc "1/0 仍登记 ∞" "$($BIN verify "$TMP/b9e.ltir")" 'L01 True'

echo "== B10 大写 E 科学计数法：整体进 NUM，不拆成隐式乘 =="
printf 'V: 1.1E17\n' > "$TMP/b10a.ltir"
ckc "1.1E17 → 110000000000000000" "$($BIN verify "$TMP/b10a.ltir")" 'L01 True  110000000000000000'
printf 'V: 2E3\n' > "$TMP/b10b.ltir"
ckc "2E3 → 2000" "$($BIN verify "$TMP/b10b.ltir")" 'L01 True  2000'
printf 'V: 1.1E-3\n' > "$TMP/b10c.ltir"
ckc "1.1E-3 → 11/10000" "$($BIN verify "$TMP/b10c.ltir")" 'L01 True  11/10000'
printf 'V: 1.1e17\n' > "$TMP/b10d.ltir"
ckn "小写 e 仍是欧拉数，不冒充科学计数" "$($BIN verify "$TMP/b10d.ltir")" '110000000000000000'
printf 'V: 1.1E17 + 1\n' > "$TMP/b10e.ltir"
ckc "大数仍能参与四则" "$($BIN verify "$TMP/b10e.ltir")" 'L01 True  110000000000000001'

echo "== 授权收尾：四个独立码入表且各归其位 =="
ckc "码表含 E-GODEL-II" "$(cat lab_codes.tsv)" 'E-GODEL-II'
ckc "码表含 E-STRENGTH-NOCERT" "$(cat lab_codes.tsv)" 'E-STRENGTH-NOCERT'
ckc "码表含 E-STRENGTH-INCOMPARABLE" "$(cat lab_codes.tsv)" 'E-STRENGTH-INCOMPARABLE'
ckc "码表含 E-STRENGTH-CYCLE" "$(cat lab_codes.tsv)" 'E-STRENGTH-CYCLE'
printf 'con: ZFC -> ZFC\n' > "$TMP/au1.ltir"
ckc "con 自指 → E-GODEL-II" "$($BIN verify "$TMP/au1.ltir")" 'E-GODEL-II'
ckc "con 自指仍 UNDECIDED 不并 False" "$($BIN verify "$TMP/au1.ltir")" '0 True, 0 False, 1 UNDECIDED'
printf 'strength_le: Q <= PA -> True\n' > "$TMP/au2.ltir"
ckc "强度比较 → E-STRENGTH-NOCERT" "$($BIN verify "$TMP/au2.ltir")" 'E-STRENGTH-NOCERT'
ckn "强度比较不再蹭通用码" "$($BIN verify "$TMP/au2.ltir")" 'E-SCOPE-CONSISTENCY'
printf 'interprets: ZF in ZFC\n' > "$TMP/au3.ltir"
ckc "interprets 同档" "$($BIN verify "$TMP/au3.ltir")" 'E-STRENGTH-NOCERT'
ck "Gödel II 定理卷 0 三态" "$($BIN verify fixtures/godel_ii.ltir | grep -c '^L0')" '0'
ckc "定理卷 counts 全 0" "$($BIN verify fixtures/godel_ii.ltir --json)" '"counts":{"True":0,"False":0,"UNDECIDED":0}'
ckn "候选三码未入正式码表" "$(cat lab_codes.tsv)" 'E-RULE-CONFLICT'
ck "码表 E- 条目 59 → 65（0.1.2/0.2.0 四行 + B 批 E-META-RANGE/E-META-ANCHOR 两行）" "$(grep -c '^E-' lab_codes.tsv)" '65'

echo "== EXAM_SCORING 计分立法（0.2.0 反号修复，豆包发现） =="
exrun(){ printf '%s\nsubmit:\n' "$2" > "$TMP/ex.ltir"; bash ./ltir_exam.sh "$TMP/ex.ltir" 2>/dev/null | grep -oE 'score=[0-9]+/100' | head -1; }
ck "纯标签卷 0 分" "$(exrun x 'notation: a
con_base: b
ordinal: c
benchmark: d')" 'score=0/100'
ck "strength_le 缺证书弃权卷 15" "$(exrun x 'strength_le: PA -> ZFC')" 'score=15/100'
ck "正确判定不再清零 15" "$(exrun x 'A: 1+1=2 -> True')" 'score=15/100'
ck "判定+认账证书链 25（V15+rule:5+by:5）" "$(exrun x 'A: 1+1=2 -> True
rule: arithmetic
by: rule: arithmetic')" 'score=25/100'
ck "编造 by 零加成（豆包 hole）" "$(exrun x 'A: 1+1=2 -> True
by: 我编的')" 'score=15/100'
ck "六判定认账证书满分 100" "$(exrun x 'rule: arithmetic
rule: more
V: 1+1=2
V: 2+2=4
V: 3+3=6
V: 4+4=8
V: 5+5=10
V: 6+6=12
by: rule: arithmetic
by: rule: more')" 'score=100/100'
ck "空卷 0 分" "$(exrun x '')" 'score=0/100'
ck "展示标签已移出 CERTS 正则" "$(grep -c 'interprets:' ltir_exam.sh)" '0'

echo "-----------------------------------------------"
echo "$PASS / $TOTAL 项满足"
echo "-----------------------------------------------"
[ "$PASS" = "$TOTAL" ]
