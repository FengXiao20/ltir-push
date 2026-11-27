#!/usr/bin/env bash
# 强度草案 · 试验自检（不是官方 SMOKE，不进 ltir.sh）
# 纪律：0 三态、0 标量、0 自动线性化；值标签只登记不参与判定。
set -u
cd "$(cd "$(dirname "$0")/.." && pwd)"
BIN=./ltir006; PASS=0; TOTAL=0
TMP="$(mktemp -d "${TMPDIR:-/tmp}/ltir_trial.XXXXXX")"; trap 'rm -rf "$TMP"' EXIT
ck(){ TOTAL=$((TOTAL+1)); if [ "$2" = "$3" ]; then PASS=$((PASS+1)); printf '  [%02d] OK  %s\n' "$TOTAL" "$1";
  else printf '  [%02d] FAIL %s  got=<%s> want=<%s>\n' "$TOTAL" "$1" "$2" "$3"; fi; }
ckc(){ TOTAL=$((TOTAL+1)); if printf '%s' "$2" | grep -qF -- "$3"; then PASS=$((PASS+1)); printf '  [%02d] OK  %s\n' "$TOTAL" "$1";
  else printf '  [%02d] FAIL %s  missing=<%s>\n' "$TOTAL" "$1" "$3"; fi; }
sig(){ "$BIN" verify "$1" --json | grep -o '"theory_sig":"[0-9a-f]*"' | cut -d'"' -f4; }

# 同一公式、两个公理名
printf 'lang: L = {0}\naxiom: Sep = forall x (x in A and P(x))\nrule: MP\ntheory_sig: ?\n' > "$TMP/a.ltir"
printf 'lang: L = {0}\naxiom: Q = forall x (x in A and P(x))\nrule: MP\ntheory_sig: ?\n'   > "$TMP/b.ltir"
# v1（默认）：公理名参与 hash → 不同
ck "v1 公理名仍影响指纹（零回归）" "$([ "$(sig "$TMP/a.ltir")" != "$(sig "$TMP/b.ltir")" ] && echo yes)" 'yes'
# v2：只喂公式本体 → 相同
sed '1i sig_mode: formula' "$TMP/a.ltir" > "$TMP/af.ltir"
sed '1i sig_mode: formula' "$TMP/b.ltir" > "$TMP/bf.ltir"
ck "v2 剔公理名：同公式同指纹" "$([ "$(sig "$TMP/af.ltir")" = "$(sig "$TMP/bf.ltir")" ] && echo yes)" 'yes'
ck "v2 与 v1 口径不同" "$([ "$(sig "$TMP/a.ltir")" != "$(sig "$TMP/af.ltir")" ] && echo yes)" 'yes'
# v2 下改一字公式仍变
printf 'sig_mode: formula\nlang: L = {0}\naxiom: Sep = forall x (x in A or P(x))\nrule: MP\ntheory_sig: ?\n' > "$TMP/c.ltir"
ck "v2 公式本体改动仍变" "$([ "$(sig "$TMP/af.ltir")" != "$(sig "$TMP/c.ltir")" ] && echo yes)" 'yes'
# 展示型值标签不进指纹
sed '1i ordinal: epsilon_0' "$TMP/af.ltir" > "$TMP/al.ltir"
ck "值标签不反过来定义理论身份" "$([ "$(sig "$TMP/af.ltir")" = "$(sig "$TMP/al.ltir")" ] && echo yes)" 'yes'
# 试验卷：0 三态 + JSON 字段 + tri 干净
T="$($BIN verify fixtures/strength_trial.ltir)"
ckc "试验卷 0 三态" "$T" '── 0 lines: 0 True, 0 False, 0 UNDECIDED ──'
ckc "人话档只报标签数" "$T" 'strength_labels: 5 (display-only; no verdict, no scalar)'
ckc "JSON 带 sig_mode" "$($BIN verify fixtures/strength_trial.ltir --json)" '"sig_mode":"formula"'
ckc "JSON 带 strength_labels" "$($BIN verify fixtures/strength_trial.ltir --json)" '"strength_labels":5'
ck "tri stdout 无三态词" "$($BIN verify fixtures/strength_trial.ltir --tri | grep -cE 'True|False|UNDECIDED')" '0'
# 计分立法 EXAM_SCORING.md（0.2.0，豆包发现反号后重立）：判定主项 / 证书加成 / 展示归零。
# 旧断言「零判定纯标签卷 100」正是被废的反号行为，卷已按新法补真判定。
printf 'submit:\nA: 1+1=2 -> True\nA: 2+2=4 -> True\nrule: MP\nrule: MT\nby: rule: MP\nby: rule: MT\n' > "$TMP/ans.ltir"
ckc "判定+证书链 40（2×15+2×5，证书须认账）" "$(bash ./ltir_exam.sh "$TMP/ans.ltir" --json --no-clock --human=n 2>/dev/null)" '"score":40'
printf 'submit:\nV: 1+1=2\nV: 2+2=4\nby: 我编的\nby: 胡写的\n' > "$TMP/ans2.ltir"
ckc "编造 by 不给加成（豆包 hole，90 封顶路径）" "$(bash ./ltir_exam.sh "$TMP/ans2.ltir" --json --no-clock --human=n 2>/dev/null)" '"score":30'
ckc "keys 卷按新法 55（V=3=45 + 证书cap2=10）" "$(bash ./ltir_exam.sh exams/keys/injection-check.ltir --json --no-clock --human=n 2>/dev/null)" '"score":55'
echo "-----------------------------------------------"
echo "$PASS / $TOTAL 项满足"
echo "-----------------------------------------------"
[ "$PASS" = "$TOTAL" ]
