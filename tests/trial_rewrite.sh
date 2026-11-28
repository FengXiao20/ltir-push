#!/usr/bin/env bash
# LTIR 0.2.1 重制版自检：四工位新词 / 试验门禁 / 七格修复 / L1-L7 护栏
# 只新增文件，不覆盖任何既有测试；从干净 0.2.0 覆盖后 make 再跑。
set -u
cd "$(cd "$(dirname "$0")/.." && pwd)"
BIN=./ltir006
PASS=0; TOTAL=0
TMP="$(mktemp -d "${TMPDIR:-/tmp}/ltir_rw.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT
ck(){ TOTAL=$((TOTAL+1)); if [ "$2" = "$3" ]; then PASS=$((PASS+1));
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; else
  printf '  [%02d] ✗ %s   got=<%s> want=<%s>\n' "$TOTAL" "$1" "$2" "$3"; fi; }
ckc(){ TOTAL=$((TOTAL+1)); if printf '%s' "$2" | grep -qF -- "$3"; then PASS=$((PASS+1));
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; else
  printf '  [%02d] ✗ %s   missing=<%s>\n' "$TOTAL" "$1" "$3"; fi; }
ckn(){ TOTAL=$((TOTAL+1)); if printf '%s' "$2" | grep -qF -- "$3"; then
  printf '  [%02d] ✗ %s   unexpected=<%s>\n' "$TOTAL" "$1" "$3"; else PASS=$((PASS+1));
  printf '  [%02d] ✓ %s\n' "$TOTAL" "$1"; fi; }
V(){ "$BIN" verify "$@" 2>/dev/null || true; }
J(){ "$BIN" verify "$@" --json 2>/dev/null || true; }

echo "== ① 五词入表，不再 E-UNK-LINE =="
for w in shape bind rewrite split mode; do
  printf '%s: probe\n' "$w" > "$TMP/$w.ltir"
  ckn "$w 已立法" "$(V "$TMP/$w.ltir")" 'E-UNK-LINE'
done
printf 'my_syntax: foo\n' > "$TMP/unk.ltir"
ckc "未立法词仍 E-UNK-LINE" "$(V "$TMP/unk.ltir")" 'E-UNK-LINE'

echo "== ② 示例卷：写法变写法，True 由旧判定核给 =="
O="$(V fixtures/rewrite_trial.ltir)"
JS="$(J fixtures/rewrite_trial.ltir)"
RW=$(grep -n '^rewrite:' fixtures/rewrite_trial.ltir | cut -d: -f1)
ckc "PERM_2 回投出 True 10" "$O" 'L01 True  10'
ckc "json derived_from 指向触发行" "$JS" '"derived_from":'"$RW"
ckc "json file_line 不漂移" "$JS" '"file_line":'"$RW"
ckc "json meta_mode=trial" "$JS" '"meta_mode":"trial"'
ckc "json meta_shapes=1" "$JS" '"meta_shapes":1'
ckc "json meta_binds=1" "$JS" '"meta_binds":1'
ckc "json meta_rewrites=1" "$JS" '"meta_rewrites":1'
ckc "counts 1/0/0" "$JS" '"counts":{"True":1,"False":0,"UNDECIDED":0}'
ckc "meta_illegal=0" "$JS" '"meta_illegal":0'
ckc "人话档自报试验态" "$O" 'rewrite: 1 replayed (mode: trial)'
ckn "quote 未被 eval（无 20 之类伪值）" "$O" 'True  20'

echo "== ③ 旧词零改义：expand 仍 δ / grammar 仍只收 BNF =="
printf 'define: f(x) = x + 1\nV: f(1)\n' > "$TMP/delta.ltir"
ckc "expand/define δ 展开照旧" "$(V "$TMP/delta.ltir")" 'L01 True  2'
printf 'grammar: shape ::= "PERM_2" num\nsig: f : Num -> Num\nby: c1\n' > "$TMP/gram.ltir"
ckc "grammar 仍只登记" "$(J "$TMP/gram.ltir")" '"grammars":1'
ckc "grammar 不生成解析器" "$(J "$TMP/gram.ltir")" '"meta_rewrites":0'

echo "== ④ bug 1：reformulate 撞名不再 last-wins =="
cat > "$TMP/b1.ltir" <<'EOF'
mode: trial
sig: f : Num -> Num
by: c1
gap: g where: x
reformulate: f(a,b,c,d) = a*d+b*c
reformulate: f(a,b,c,d) = a*d-b*c
EOF
ckc "同 fn 两条给专码" "$(V "$TMP/b1.ltir")" 'E-REWRITE-CONFLICT'
ckn "无静默 True 行" "$(V "$TMP/b1.ltir")" 'L01 True'

echo "== ⑤ bug 2：file_line 漂移已收口 =="
ckc "产物行带 derived_from" "$JS" '"derived_from":'
ckc "产物 file_line == 触发行" "$JS" '"file_line":'"$RW"

echo "== ⑥ bug 3：witness 体也扫 L3a =="
printf 'witness: A: 2+2==5 -> True\n' > "$TMP/b3.ltir"
ckc "witness 夹手写结论拒收" "$(V "$TMP/b3.ltir")" 'E-REWRITE-REDEEM'
printf 'witness: V: 1/110000000000000000  ->  1/110000000000000000\n' > "$TMP/b3ok.ltir"
ckn "历史卷 witness 对照仍可登记" "$(V "$TMP/b3ok.ltir")" 'E-REWRITE-REDEEM'

echo "== ⑦ bug 4：错位码归位 =="
cat > "$TMP/b4.ltir" <<'EOF'
mode: trial
sig: f : Num -> Num
by: c1
gap: g where: x
reformulate: f(a) = a
rewrite: no_such_shape
EOF
ckc "未知 shape id 用专码" "$(V "$TMP/b4.ltir")" 'E-SHAPE-UNKNOWN'
ckn "不再蹭 E-DEF-SIG" "$(V "$TMP/b4.ltir")" 'E-DEF-SIG'
cat > "$TMP/b4b.ltir" <<'EOF'
mode: trial
sig: f : Num -> Num
by: c1
gap: g where: x
shape: s ::= "H" slots: a,b target: f
reformulate: f(a) = a
bind: s <- 1,2
rewrite: s
EOF
ckc "槽位数/参数名不符用专码" "$(V "$TMP/b4b.ltir")" 'E-REWRITE-ARITY'

echo "== ⑧ bug 5：load: 模块路径不再静默丢 meta 行 =="
printf 'module: m\ngap: loaded_gap where: x\nreformulate: loaded_rule(a) = a\n' > "$TMP/mod.ltir"
printf 'load: %s/mod.ltir\n' "$TMP" > "$TMP/b5.ltir"
ckc "模块 meta_gaps 计入" "$(J "$TMP/b5.ltir")" '"meta_gaps":1'
ckc "模块 meta_rules 计入" "$(J "$TMP/b5.ltir")" '"meta_rules":1'
ckc "模块路径给诊断" "$(V "$TMP/b5.ltir")" 'E-LOAD-META'

echo "== ⑨ bug 6：双 quote 无「取第一个」选择规则 =="
cat > "$TMP/b6.ltir" <<'EOF'
mode: trial
sig: f : Num -> Num
by: c1
gap: g where: x
shape: s ::= "H" slots: a,b target: f
reformulate: f(a,b) = a+b
bind: s <- 1,2
quote: <ltir> V: 9+9 </ltir>
quote: <ltir> V: 1+1 </ltir>
rewrite: s
EOF
ckc "取值来自 bind，不是 quote" "$(V "$TMP/b6.ltir")" 'L01 True  3'
ckn "不再出 18" "$(V "$TMP/b6.ltir")" 'True  18'
printf 'V: PERM_2([[1,2],[3,4]])\n' > "$TMP/qeval.ltir"
ckc "quote 内层单独跑仍弃权" "$(V "$TMP/qeval.ltir")" 'E-SCOPE-TRANSCENDENTAL'

echo "== ⑩ bug 7：槽位字面量严格 charset =="
cat > "$TMP/b7.ltir" <<'EOF'
mode: trial
sig: f : Num -> Num
by: c1
gap: g where: x
shape: s ::= "H" slots: a,b target: f
reformulate: f(a,b) = a+b
bind: s <- 1/2/3,2
EOF
ckc "1/2/3 连除拒收" "$(V "$TMP/b7.ltir")" 'E-REWRITE-ARITY'
cat > "$TMP/b7ok.ltir" <<'EOF'
mode: trial
sig: f : Num -> Num
by: c1
gap: g where: x
shape: s ::= "H" slots: a,b target: f
reformulate: f(a,b) = a+b
bind: s <- 1/2,-3
rewrite: s
EOF
ckc "单个分数与负数照收" "$(V "$TMP/b7ok.ltir")" 'L01 True  -5/2'

echo "== ⑪ 兑换权：走私判定词先验拒收 =="
cat > "$TMP/l3.ltir" <<'EOF'
mode: trial
sig: f : Num -> Num
by: c1
gap: g where: x
reformulate: f(a,b) = (A: 2+2==5 -> True)
EOF
ckc "走私宏给 E-REWRITE-REDEEM" "$(V "$TMP/l3.ltir")" 'E-REWRITE-REDEEM'
ckn "不产假 True" "$(V "$TMP/l3.ltir")" 'True  2'
ckc "meta_illegal 计数" "$(J "$TMP/l3.ltir")" '"meta_illegal":1'
printf 'sig: f : Num -> Num\nby: c1\ngap: g where: x\nquote: <ltir> A: 2+2==5 -> True </ltir>\n' > "$TMP/l3q.ltir"
ckc "quote 内层手写结论也拒收" "$(V "$TMP/l3q.ltir")" 'E-REWRITE-REDEEM'

echo "== ⑫ split: 只拆账本，不关闭目标 =="
printf 'split: t into: gap: g ; witness: w ; by: c ; con: PA -> PA\n' > "$TMP/sp.ltir"
ckc "自证结构撞 E-GODEL-II" "$(V "$TMP/sp.ltir")" 'E-GODEL-II'
printf 'split: t into: gap: g ; witness: w\n' > "$TMP/sp2.ltir"
ckc "split 缺 by 锚弃权" "$(V "$TMP/sp2.ltir")" 'E-META-ANCHOR'
ckc "split 体有 by 时只登记" "$(J fixtures/rewrite_gentzen.ltir)" '"meta_splits":1'
ckn "split 不产三态" "$(J fixtures/rewrite_gentzen.ltir)" '"counts":{"True":0,"False":0,"UNDECIDED":1}'

echo "== ⑬ mode: strict 缺省门禁（旧卷零回归） =="
cat > "$TMP/strict.ltir" <<'EOF'
sig: f : Num -> Num
by: c1
gap: g where: x
shape: s ::= "H" slots: a,b target: f
reformulate: f(a,b) = a+b
bind: s <- 1,2
rewrite: s
EOF
ckc "strict 下 rewrite 只登记" "$(J "$TMP/strict.ltir")" '"meta_rewrites":0'
ckc "strict 下 0 三态" "$(J "$TMP/strict.ltir")" '"counts":{"True":0,"False":0,"UNDECIDED":0}'
printf 'mode: bogus\n' > "$TMP/mode.ltir"
ckc "mode 非法值硬错" "$(V "$TMP/mode.ltir")" 'E-META-RANGE'

echo "== ⑭ L4 差分恒等：counts(展开卷) = counts(基准卷) + counts(产物卷) =="
grep -v '^rewrite:' fixtures/rewrite_trial.ltir > "$TMP/base.ltir"
printf 'at: a=1,b=2,c=3,d=4\nV: a*d + b*c\n' > "$TMP/prod.ltir"
B=$(J "$TMP/base.ltir" | grep -o '"True":[0-9]*' | cut -d: -f2)
E=$(J fixtures/rewrite_trial.ltir | grep -o '"True":[0-9]*' | cut -d: -f2)
P=$(J "$TMP/prod.ltir" | grep -o '"True":[0-9]*' | cut -d: -f2)
ck "差分恒等 $E = $B + $P" "$E" "$((B+P))"
ck "基准卷 0 三态" "$(J "$TMP/base.ltir" | grep -o '"counts":{"True":[0-9]*,"False":[0-9]*,"UNDECIDED":[0-9]*}')" '"counts":{"True":0,"False":0,"UNDECIDED":0}'

echo "== ⑮ L6 确定性：同卷重跑逐字一样 =="
A=$(V fixtures/rewrite_trial.ltir | md5sum | cut -d' ' -f1)
B2=$(V fixtures/rewrite_trial.ltir | md5sum | cut -d' ' -f1)
ck "两次 stdout md5 一致" "$A" "$B2"

echo "== ⑯ 学术案例：Gentzen 外部证书 / Gödel II 自证护栏 =="
ck "Gentzen 卷 0 三态" "$(J fixtures/rewrite_gentzen.ltir | grep -o '"counts":{"True":[0-9]*,"False":[0-9]*,"UNDECIDED":[0-9]*}')" '"counts":{"True":0,"False":0,"UNDECIDED":0}'
ckc "Gentzen 卷外部证书只登记" "$(J fixtures/rewrite_gentzen.ltir)" '"meta_splits":1'
ckc "Gödel II 自证弃权" "$(V fixtures/rewrite_godel.ltir)" 'E-GODEL-II'
ckn "Gödel II 卷不产 True 行" "$(V fixtures/rewrite_godel.ltir)" 'L01 True'

echo "== ⑰ 零回归：老护栏一条不松 =="
printf 'V: 0/0\n' > "$TMP/z1.ltir"; ckc "0/0 仍 E-INDETERMINATE" "$(V "$TMP/z1.ltir")" 'E-INDETERMINATE'
printf 'V: 1.1E17\n' > "$TMP/z2.ltir"; ckc "B10 大写 E" "$(V "$TMP/z2.ltir")" '110000000000000000'
printf 'solve: x^2 + 1 >= 0\n' > "$TMP/z3.ltir"; ckc "B8 恒号" "$(V "$TMP/z3.ltir")" 'x ∈ R'
printf 'V: 1/110000000000000000\n' > "$TMP/z4.ltir"; ckc "B9 直除" "$(V "$TMP/z4.ltir")" '1/110000000000000000'
printf 'con: ZFC -> ZFC\n' > "$TMP/z5.ltir"; ckc "Gödel II 独立码" "$(V "$TMP/z5.ltir")" 'E-GODEL-II'
printf 'strength_le: VP <= VNP -> True\n' > "$TMP/z6.ltir"; ckc "强度比较缺证书" "$(V "$TMP/z6.ltir")" 'E-STRENGTH-NOCERT'
printf 'A: VP != VNP -> True\n' > "$TMP/z7.ltir"; ckc "外部声称换不来 True" "$(V "$TMP/z7.ltir")" 'UNDECIDED'

echo "== ⑱ 输出契约 / 门禁 =="
ck "0 三态卷 --tri stdout 0 字节" "$("$BIN" verify fixtures/rewrite_gentzen.ltir --tri 2>/dev/null | wc -c | tr -d ' ')" "0"
CI_N=$(CI=1 "$BIN" verify fixtures/rewrite_gentzen.ltir 2>/dev/null | grep -Ec '^(True|False|UNDECIDED)|^L0[0-9]')
ck "CI=1 台账汇报不含三态词行" "$CI_N" "0"
ckn "--json 不含裸 ERROR 行" "$JS" '"ERROR"'
ckc "版本仍 0.2.0" "$JS" '"version":"0.2.0"'
BAD=0; while read -r h f; do [ -z "$h" ] && continue
  a=$(sha256sum "$f" 2>/dev/null | cut -d' ' -f1); [ "$a" = "$h" ] || BAD=1
done < RUN_core_sha.txt
ck "core 门禁 11/11 未动" "$BAD" "0"
ck "码表 65 -> 69 行" "$(grep -c '^E-' lab_codes.tsv)" "69"

echo "== ⑲ r2 / bug A：L3 禁词表补齐第三态 UNDECIDED =="
cat > "$TMP/a1.ltir" <<'EOF'
mode: trial
sig: f : Num -> Num
by: c1
gap: g where: x
shape: s ::= "H" slots: a,b target: f
reformulate: f(a,b) = a+b -> UNDECIDED
bind: s <- 1,2
rewrite: s
EOF
ckc "宏体 -> UNDECIDED 先验拒收" "$(V "$TMP/a1.ltir")" 'E-REWRITE-REDEEM'
ckc "meta_illegal 计数" "$(J "$TMP/a1.ltir")" '"meta_illegal":1'
ckc "不再 replay" "$(J "$TMP/a1.ltir")" '"meta_rewrites":0'
printf 'quote: <ltir> V: 5+5 -> UNDECIDED </ltir>\n' > "$TMP/a2.ltir"
ckc "宽档 quote 内层也拒" "$(V "$TMP/a2.ltir")" 'E-REWRITE-REDEEM'
printf 'witness: V: 5+5 -> UNDECIDED\n' > "$TMP/a3.ltir"
ckc "宽档 witness 体也拒" "$(V "$TMP/a3.ltir")" 'E-REWRITE-REDEEM'
printf 'gap: g where: x -> undecided\nby: c1\nsig: f : Num -> Num\n' > "$TMP/a4.ltir"
ckc "小写 undecided 也拒" "$(V "$TMP/a4.ltir")" 'E-REWRITE-REDEEM'
printf 'gap: g where: x -> 弃权\nby: c1\nsig: f : Num -> Num\n' > "$TMP/a5.ltir"
ckc "中文弃权词也拒" "$(V "$TMP/a5.ltir")" 'E-REWRITE-REDEEM'
printf 'witness: solve: x^2 + 1 >= 0  ->  x ∈ R\n' > "$TMP/a6.ltir"
ckn "历史卷式对照尾巴不误伤" "$(V "$TMP/a6.ltir")" 'E-REWRITE-REDEEM'

echo "== ⑳ r2 / bug A 顺带：旧核不再吞手写 UNDECIDED（既有立法保留） =="
printf 'V: 5+5 -> UNDECIDED\n' > "$TMP/a7.ltir"
ckc "V 手写第三态 E-SYN" "$(V "$TMP/a7.ltir")" 'E-SYN'
printf 'A: 2+2==5 -> UNDECIDED\n' > "$TMP/a8.ltir"
ckc "A 手写第三态不再改判 False" "$(V "$TMP/a8.ltir")" 'E-SYN'
printf 'A: 2+2=4 -> Maybe\n' > "$TMP/a9.ltir"
ckc "词库外判定词补回全禁版：给码 E-SYN" "$(V "$TMP/a9.ltir")" 'E-SYN'
printf 'V: 5+5 -> 乱写的尾巴\n' > "$TMP/a10.ltir"
ckc "乱写尾巴同样给码 E-SYN（不再静默吞）" "$(V "$TMP/a10.ltir")" 'E-SYN'
printf 'A: 2+2==4 -> True\n' > "$TMP/a11.ltir"
ckc "合法 True 声明零回归" "$(V "$TMP/a11.ltir")" 'L01 True'
printf 'A: 2+2=4 -> True。\n' > "$TMP/a12.ltir"
ckc "中文句号尾巴零回归" "$(V "$TMP/a12.ltir")" 'L01 True'
printf 'A: 3^3 == 27 -> True 那么结论成立。\n' > "$TMP/a13.ltir"
ckc "中文续句尾巴零回归" "$(V "$TMP/a13.ltir")" 'L01 True'
printf 'A: 2+2==4 -> True by: ModusPonens\n' > "$TMP/a14.ltir"
ckc "行尾 by: 零回归" "$(V "$TMP/a14.ltir")" 'L01 True'
printf 'A: forall: x in R -> True\n' > "$TMP/a15.ltir"
ckn "量词声明不被 guard 误伤" "$(V "$TMP/a15.ltir")" 'E-SYN'

echo "== ㉑ r2 / bug B：reflect 四个新目标有 stdout 出口（L5 可复原） =="
O2="$(V fixtures/rewrite_trial.ltir)"
ckc "rewrites 逐字列出 at 产物" "$O2" 'at: a=1, b=2, c=3, d=4'
ckc "rewrites 逐字列出 V 产物" "$O2" 'V: a*d + b*c'
ckc "mode 有 stdout 出口" "$O2" 'reflect: mode -> trial'
cat > "$TMP/rf.ltir" <<'EOF'
mode: strict
sig: f : Num -> Num
by: c1
gap: g where: x
shape: s ::= "H" slots: a,b target: f
reformulate: f(a,b) = a+b
bind: s <- 1,2
split: t into: gap: g ; witness: w ; by: c1
reflect: shapes
reflect: binds
reflect: splits
reflect: mode
EOF
RF="$(V "$TMP/rf.ltir")"
ckc "shapes 有 stdout 出口" "$RF" 'reflect: shapes ->'
ckc "binds 有 stdout 出口" "$RF" 'reflect: binds ->'
ckc "splits 有 stdout 出口" "$RF" 'reflect: splits ->'
ckc "split 逐字内容" "$RF" 't into: gap: g ; witness: w ; by: c1'
ckc "mode strict 出口" "$RF" 'reflect: mode -> strict'
ck "reflect 卷 --tri stdout 0 字节" "$("$BIN" verify "$TMP/rf.ltir" --tri 2>/dev/null | wc -c | tr -d ' ')" "0"
ckn "--json 不被 reflect 输出污染" "$(J "$TMP/rf.ltir")" 'reflect:'

echo "== ㉒ r2 / bug C：文档 L1 措辞与实现自洽 =="
ckc "L1 不再写「rewrite 缺 trial 即弃权」" "$(grep 'L1 锚定' PATCH_0.2.1_REMAKE.md)" 'strict 下只登记'
ckc "strict 无码行为仍成立" "$(J "$TMP/strict.ltir")" '"meta_rewrites":0'
ckn "strict 不产 E-META-ANCHOR" "$(V "$TMP/strict.ltir")" 'E-META-ANCHOR'

echo "-----------------------------------------------"
echo "$PASS / $TOTAL 项满足"
echo "-----------------------------------------------"
[ "$PASS" = "$TOTAL" ]
