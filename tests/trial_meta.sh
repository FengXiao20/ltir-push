#!/usr/bin/env bash
# LTIR 0.2.0 元编程试验自检（只登记 / 0 三态 / 不自举）
set -u
cd "$(cd "$(dirname "$0")/.." && pwd)"
BIN=./ltir006
PASS=0; TOTAL=0
TMP="$(mktemp -d "${TMPDIR:-/tmp}/ltir_meta.XXXXXX")"
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

echo "== 六词不再 E-UNK-LINE =="
for w in gap reformulate quote reflect depth trace; do
  printf "%s: %s\n" "$w" "probe" > "$TMP/$w.ltir"
  ckn "$w: 登记不报错" "$($BIN verify "$TMP/$w.ltir")" 'E-UNK-LINE'
done

echo "== 试验卷全卷 0 三态 =="
ck "试验卷 stdout 零三态词" "$($BIN verify fixtures/metaprogram_trial.ltir | grep -Ec '^(True|False|UNDECIDED)|L0')" "0"
O="$($BIN verify fixtures/metaprogram_trial.ltir --json)"
ckc "json meta_gaps=1" "$O" '"meta_gaps":1'
ckc "json meta_rules=1" "$O" '"meta_rules":1'
ckc "json meta_quotes=1" "$O" '"meta_quotes":1'
ckc "json meta_depth=12" "$O" '"meta_depth":12'
ckc "counts 全 0" "$O" '"counts":{"True":0,"False":0,"UNDECIDED":0}'

echo "== 历史卷 v2 全卷 0 三态 =="
ck "历史卷 stdout 零三态词" "$($BIN verify fixtures/history_gaps.ltir | grep -Ec '^(True|False|UNDECIDED)|L0')" "0"
ckc "历史卷 --tri 全空" "$($BIN verify fixtures/history_gaps.ltir --tri | wc -c | tr -d ' ')" '0'

echo "== 冲突 / 非法参数复用 E-DEF-SIG，不静默择一 =="
printf 'gap: dup  where: A\ngap: dup  where: B\n' > "$TMP/dup.ltir"
ckc "同 id 两条 gap 弃权" "$($BIN verify "$TMP/dup.ltir")" 'E-DEF-SIG'
printf 'depth: 999\n' > "$TMP/deep.ltir"
ckc "depth 越界给码（0.2.0 B 立专码）" "$($BIN verify "$TMP/deep.ltir")" 'E-META-RANGE'
printf 'reflect: invent\n' > "$TMP/rf.ltir"
ckc "未知 reflect 目标给码" "$($BIN verify "$TMP/rf.ltir")" 'E-DEF-SIG'

echo "== depth: 真解 8 步上限，旧卷零回归 =="
printf 'define: f(x) = x + 1\nV: f(1)\n' > "$TMP/d1.ltir"
ckc "缺省 8 步：f(1)=2" "$($BIN verify "$TMP/d1.ltir")" 'L01 True  2'
printf 'define: f(x) = x + 1\ndepth: 12\nV: f(1)\n' > "$TMP/d2.ltir"
ckc "depth: 12 结果不漂" "$($BIN verify "$TMP/d2.ltir")" 'L01 True  2'

echo "== trace: 走 stderr，不污染 stdout =="
printf 'define: f(x) = x + 1\ntrace: on\nexpand: f(1)\n' > "$TMP/tr.ltir"
SO="$($BIN verify "$TMP/tr.ltir" 2>/dev/null)"
ckn "trace 不进 stdout" "$SO" '[trace'
ckc "trace 进 stderr" "$($BIN verify "$TMP/tr.ltir" 2>&1 >/dev/null)" '[trace'

echo "== 0.1.2 三件在叠加版里仍成立 =="
printf 'solve: x^2 + 1 >= 0\n' > "$TMP/ov1.ltir"
ckc "B8 恒号" "$($BIN verify "$TMP/ov1.ltir")" 'x ∈ R'
printf 'V: 1/110000000000000000\n' > "$TMP/ov2.ltir"
ckc "B9 直除" "$($BIN verify "$TMP/ov2.ltir")" '1/110000000000000000'
printf 'V: 1.1E17\n' > "$TMP/ov3.ltir"
ckc "B10 大写 E" "$($BIN verify "$TMP/ov3.ltir")" '110000000000000000'

echo "== 0.2.0 B：撞名不覆盖 / 缺锚不静默 / 残词有意报错 =="
printf 'sig: f : Num -> Num\nsig: f : Poly -> Set\nV: 1\n' > "$TMP/sr.ltir"
ckc "sig 重定义弃权，不静默覆盖" "$($BIN verify "$TMP/sr.ltir")" 'E-DEF-SIG'
printf 'gap: lonely where: x > 0\n' > "$TMP/an1.ltir"
ckc "裸 gap 缺 sig+by 锚弃权" "$($BIN verify "$TMP/an1.ltir")" 'E-META-ANCHOR'
printf 'sig: f : Num -> Num\nreformulate: f(x) = x + 1\nby: c1\n' > "$TMP/an2.ltir"
ckc "孤儿 reformulate 无 gap 弃权" "$($BIN verify "$TMP/an2.ltir")" 'E-META-ANCHOR'
printf 'quote: <ltir> V: 1 + 1\n' > "$TMP/q1.ltir"
ckc "quote 未闭合残词 E-SYN" "$($BIN verify "$TMP/q1.ltir")" 'E-SYN'
printf 'trace: bogus\n' > "$TMP/tb.ltir"
ckc "trace 非法值 E-META-RANGE" "$($BIN verify "$TMP/tb.ltir")" 'E-META-RANGE'
printf 'depth: 0\n' > "$TMP/dz.ltir"
ckc "depth 0 也 E-META-RANGE" "$($BIN verify "$TMP/dz.ltir")" 'E-META-RANGE'
ckc "reflect gaps 人话档列已登记 id" "$($BIN verify fixtures/metaprogram_trial.ltir)" 'quad_disc_neg'

echo "-----------------------------------------------"
echo "$PASS / $TOTAL 项满足"
echo "-----------------------------------------------"
[ "$PASS" = "$TOTAL" ]
