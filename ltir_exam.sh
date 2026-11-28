#!/usr/bin/env bash
# LTIR 0.2.0 试卷库 · shell 层执行器（判定核 ./ltir006 一字不动）
#   bash ltir.sh exam                     抽题 + 考前须知 + 题面，stdin 收答卷
#   bash ltir.sh exam answer.ltir        打开旧卷重判（同样命令）
#   --stdin / --paper=PATH / --json / --ndjson / --tri / --output=
#   --seed= / --difficulty= / --count= / --human=y|n / --no-clock / --list
# 契约：stdout=题面+唯一一条交卷回执；stderr=须知/人话；非 TTY 不弹交互。
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE" || exit 2
BIN=./ltir006
CFG=exam.json
DIFFS="trivial easy normal hard very-hard prize"
[ -x "$BIN" ] || { printf 'E-EXAM-CONFIG: %s 未编译，先跑一次 bash ltir.sh\n' "$BIN" >&2; exit 2; }

T0=$(date +%s 2>/dev/null || printf '0')
err(){ printf '%s\n' "$*" >&2; }

cfg_get(){
  local k="$1" v=''
  [ -f "$CFG" ] || { printf ''; return 0; }
  v=$(grep -E "^[[:space:]]*\"?${k}\"?[[:space:]]*:" "$CFG" 2>/dev/null | head -1 || true)
  [ -n "$v" ] || { printf ''; return 0; }
  v=${v#*:}; v=${v%%,*}
  printf '%s' "$v" | tr -d '\"' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//'
}

OUT=''; SEED=''; DIFF=''; HUMAN=''; COUNT=''; HINTS=''
JSONF=0; NDJSON=0; TRIF=0; STDIN=0; PAPER=''; NOCLOCK=0; LIST=0; ANSFILE=''
for a in "$@"; do
  case "$a" in
    --output=*) OUT="${a#*=}" ;;
    --json)     JSONF=1; OUT=json ;;
    --ndjson)   NDJSON=1; OUT=ndjson ;;
    --tri)      TRIF=1; OUT=tri ;;
    --seed=*)   SEED="${a#*=}" ;;
    --difficulty=*) DIFF="${a#*=}" ;;
    --human=*)  HUMAN="${a#*=}" ;;
    --count=*)  COUNT="${a#*=}" ;;
    --hints=*)  HINTS="${a#*=}" ;;
    --paper=*)  PAPER="${a#*=}" ;;
    --stdin)    STDIN=1 ;;
    --no-clock) NOCLOCK=1 ;;
    --list)     LIST=1 ;;
    *.ltir)     ANSFILE="$a" ;;
    *) err "E-EXAM-CONFIG: unknown option $a"; exit 2 ;;
  esac
done
if [ "$JSONF" = 1 ] && [ "$NDJSON" = 1 ]; then err 'E-EXAM-OUTPUT: --json 与 --ndjson 冲突'; exit 4; fi
if [ "$TRIF" = 1 ] && [ "$JSONF" = 1 ]; then err 'E-EXAM-OUTPUT: --tri 与 --json 冲突'; exit 4; fi

# 配置校验（白名单 mini reader，坏 JSON 不静默吞）
if [ -f "$CFG" ]; then
  FIRSTC="$(tr -d '[:space:]' < "$CFG" | head -c 1)"
  LASTC="$(tr -d '[:space:]' < "$CFG" | tail -c 1)"
  [ "$FIRSTC" = '{' ] || { err 'E-EXAM-CONFIG: exam.json 不是对象'; exit 2; }
  [ "$LASTC" = '}' ] || { err 'E-EXAM-CONFIG: exam.json 未闭合'; exit 2; }
fi

[ -n "$OUT" ]   || OUT="$(cfg_get output)";   : "${OUT:=default}"
[ -n "$SEED" ]  || SEED="$(cfg_get seed)";    : "${SEED:=42}"
[ -n "$DIFF" ]  || DIFF="$(cfg_get difficulty)"; : "${DIFF:=normal}"
[ -n "$HUMAN" ] || HUMAN="$(cfg_get human)";
[ -n "$COUNT" ] || COUNT="$(cfg_get count)";  : "${COUNT:=1}"
[ -n "$HINTS" ] || HINTS="$(cfg_get hints)";  : "${HINTS:=2}"
case "$OUT" in default|json|ndjson|tri) ;; *) err 'E-EXAM-OUTPUT'; exit 4 ;; esac
case "$HUMAN" in y|n) ;; *) HUMAN='' ;; esac
case " $DIFFS " in *" $DIFF "*) ;; *) err "E-EXAM-DIFFICULTY: $DIFF"; exit 2 ;; esac
case "$SEED" in *[!0-9-]*) err 'E-EXAM-CONFIG: seed 必须是整数'; exit 2 ;; esac
case "$COUNT" in *[!0-9]*) err 'E-EXAM-CONFIG: count 必须是整数'; exit 2 ;; esac

if [ "$LIST" = 1 ]; then
  for d in $DIFFS; do
    for f in "exams/$d"/*.ltir; do
      [ -f "$f" ] && printf '%s\t%s\n' "$d" "$(basename "$f" .ltir)"
    done
  done
  exit 0
fi

# 君子协议：json 预置 y/n → 静默跳过询问；命令行覆盖；非 TTY 一律不弹
if [ -z "$HUMAN" ]; then
  if [ -t 0 ] && [ "${CI:-}" != 1 ] && [ -z "$ANSFILE" ] && [ "$STDIN" = 0 ]; then
    err '你是否人类？ y/n'
    IFS= read -r hans || hans=''
    case "$hans" in y|Y) HUMAN=y ;; n|N) HUMAN=n ;; *) HUMAN=undisclosed ;; esac
  else
    HUMAN=undisclosed
  fi
fi

# 抽题：目录排序 + 纯整数哈希，零浮点、跨平台可复现
PAPERS=()
for f in "exams/$DIFF"/*.ltir; do [ -f "$f" ] && PAPERS+=("$f"); done
[ "${#PAPERS[@]}" -gt 0 ] || { err "E-EXAM-EMPTY: exams/$DIFF"; exit 2; }
pick_one(){
  local best='' bestv='' v h f
  for f in "${PAPERS[@]}"; do
    h=$(printf '%s' "${SEED}|${f}" | sha256sum | cut -c1-8)
    v=$((16#$h))
    if [ -z "$best" ] || [ "$v" -lt "$bestv" ]; then best="$f"; bestv=$v; fi
  done
  printf '%s' "$best"
}
PAPER_FILE="$(pick_one)"
EXAM_ID="$(basename "$PAPER_FILE" .ltir)"

hdr(){ grep -E "^${1}:" "$2" 2>/dev/null | head -1 | sed "s/^${1}:[[:space:]]*//"; }
HINT_LINES="$(grep -E '^hint:' "$PAPER_FILE" 2>/dev/null | head -n "$HINTS" | sed 's/^hint:[[:space:]]*/- /')"

# 考前声明：静态说明书，不含任何对当前卷的对错（延迟反馈只管判定）
err '== LTIR 考前须知 =='
err "题  $EXAM_ID · $DIFF · seed=$SEED"
err '答  写 .ltir，多行随便；普通回车只换行，不交卷'
err '交  单独一行 submit:        EOF 不算交卷'
err '判  submit: 前不回显任何对错/错因；交卷后一次性给'
err '时  本机相对秒数，不作公证'
err "人  human=$HUMAN（json 预置/命令覆盖，未询问）  --human=y 可改"
err '出  json | ndjson | tri | default       --output= 覆盖'
err ''

# 题面 stdout（题面不是判定，tri 模式下仍给；--paper 落文件给 Agent 自己读）
# 0.1.1 bugfix B6：json/ndjson 模式 stdout 只留机器回执，题面走 stderr（agent 管道契约）
if [ -z "$ANSFILE" ]; then
  if [ -n "$PAPER" ]; then
    cat "$PAPER_FILE" > "$PAPER" || { err 'E-EXAM-CONFIG: --paper 写不了'; exit 2; }
    printf '{"event":"paper","exam_id":"%s","path":"%s"}\n' "$EXAM_ID" "$PAPER"
  else
    if [ "$OUT" = json ] || [ "$OUT" = ndjson ]; then cat "$PAPER_FILE" >&2
    else cat "$PAPER_FILE"; fi
  fi
else
  [ -f "$ANSFILE" ] || { err "E-EXAM-CONFIG: $ANSFILE"; exit 2; }
fi

# 收答卷：交卷前零回显
if [ -n "$ANSFILE" ]; then
  ANS="$(cat "$ANSFILE")"
else
  if [ "$STDIN" = 1 ] || [ ! -t 0 ]; then ANS="$(cat)"
  else
    err '（粘贴答题卷；Ctrl-D 结束输入，但必须有 submit: 才算交卷）'
    ANS="$(cat)"
  fi
fi

HAS_SUB=0
while IFS= read -r ln || [ -n "$ln" ]; do
  z="$(printf '%s' "$ln" | tr -d '[:space:]')"
  if [ "$z" = 'submit:' ]; then HAS_SUB=1; break; fi
done <<EOF
$ANS
EOF
if [ "$HAS_SUB" != 1 ]; then
  err 'E-EXAM-UNSUBMITTED: 卷里没有 submit:，EOF 不算交卷'
  # 0.1.1 bugfix B6：机器模式给 JSON 错误回执，stdout 不再是纯题面
  if [ "$OUT" = json ]; then
    printf '{"exam_id":"%s","difficulty":"%s","seed":%s,"submit":false,"error":"E-EXAM-UNSUBMITTED"}\n' "$EXAM_ID" "$DIFF" "$SEED"
  elif [ "$OUT" = ndjson ]; then
    printf '{"event":"error","exam_id":"%s","code":"E-EXAM-UNSUBMITTED","submit":false}\n' "$EXAM_ID"
  fi
  exit 3
fi

TMPA="$(mktemp "${TMPDIR:-$HOME}/ltir_exam.XXXXXX")" || { err 'E-EXAM-CONFIG: 临时卷建不了'; exit 2; }
trap 'rm -f "$TMPA"' EXIT
printf '%s\n' "$ANS" | grep -v '^[[:space:]]*submit:[[:space:]]*$' > "$TMPA"

# 判卷走现成 CLI；打分只看「有没有按法庭纪律写字」，不判证明的数学正确性
TRI="$($BIN verify "$TMPA" --tri 2>/dev/null || true)"
VJSON='['; first=1; DIAGS='['; dfirst=1
while IFS= read -r w; do
  [ -n "$w" ] || continue
  [ "$first" = 1 ] || VJSON="$VJSON,"
  VJSON="$VJSON\"$w\""; first=0
  case "$w" in True) ;; *)
    [ "$dfirst" = 1 ] || DIAGS="$DIAGS,"
    DIAGS="$DIAGS\"$w\""; dfirst=0 ;; esac
done <<EOF
$TRI
EOF
VJSON="$VJSON]"; DIAGS="$DIAGS]"

# ══ 计分立法 EXAM_SCORING.md（0.2.0 · 反号修复，豆包发现） ══
# 方向（TA 令）：判定主项 / 证书加成 / 展示归零——分数与「判得对不对」同号。
#   V     = --tri 判定行数（True/False/UNDECIDED 同酬 15 分，cap 6=90；ERROR 已被 --tri 排除，不冒充 UNDECIDED）
#   CERTS = 真证书锚 by:/rule:/cex:/inj:/surj:/bij:（加成 5 分/条，cap 2=10）；展示标签移出，0 分
#   SCORE = V=0 → 0（只甩声明词 / 纯标签卷 / 越界声明一律零分）
#           否则 min(100, V*15 + min(CERTS,2)*5)
# 废除：40 分 LINES 保底、CLAIMS ±20/−15、「无证书断言清零」——四项叠加曾致「越判越吃亏」。
TRI_N=$(printf '%s\n' "$TRI" | grep -cE '^(True|False|UNDECIDED)$' || true); TRI_N=${TRI_N:-0}
# 证书验真（0.2.0 B2，豆包发现）：by: 引用须认账——与引擎 registerByRef 同款
# 双向子串匹配本卷 axiom:/ref:/rule:/gap: 登记名 + 经典白名单；「by: 我编的」不再拿加成。
# 其余五类（rule:/cex:/inj:/surj:/bij:）自证登记照旧计入。
# 认账来源：①本卷 axiom/rule/gap 登记名 ②题面卷声明的规则名 ③经典白名单
# 匹配口径：整串精确相等（去首尾空白）；不再双向子串——「by: a」「by: 0」不再蹭分
bear_name(){ # 登记行 -> 取「名」：axiom 取等号/空格前标识符，gap 取首词，其余取整行
  local s="${1#*:}"
  s="${s#"${s%%[![:space:]]*}"}"; s="${s%"${s##*[![:space:]]}"}"
  case "$1" in
    axiom:*) printf '%s' "${s%%[ =]*}" ;;
    gap:*)   printf '%s' "${s%%[[:space:]]*}" ;;
    *)       printf '%s' "$s" ;;
  esac
}
KNOWN=()
while IFS= read -r ln; do
  [ -z "$ln" ] && continue
  n="$(bear_name "$ln")"; [ -n "$n" ] && KNOWN+=("$n")
done < <(grep -hE '^(axiom:|ref:|rule:|gap:)' "$TMPA" 2>/dev/null)
if [ -n "${PAPER_FILE:-}" ] && [ -f "${PAPER_FILE:-}" ]; then
  while IFS= read -r ln; do
    [ -z "$ln" ] && continue
    case "$ln" in
      by:*rule:*) n="${ln#*rule:}" ;;
      rule:*)     n="${ln#rule:}" ;;
      *) continue ;;
    esac
    n="${n#"${n%%[![:space:]]*}"}"; n="${n%"${n##*[![:space:]]}"}"
    [ -n "$n" ] && KNOWN+=("$n")
  done < <(grep -hE '^(by:[[:space:]]*rule:|rule:)' "$PAPER_FILE" 2>/dev/null)
fi
known_hit(){ nm="$1"
  nm="${nm#"${nm%%[![:space:]]*}"}"; nm="${nm%"${nm##*[![:space:]]}"}"
  [ -z "$nm" ] && return 1
  case "$nm" in calc|Kunen1971|Godel1940|Cohen1963) return 0 ;; esac
  for a in ${KNOWN[@]+"${KNOWN[@]}"}; do [ "$a" = "$nm" ] && return 0; done
  return 1; }
# 带码行不进证书分：E-SCOPE*/E-OVF/E-INDETERMINATE 属诚实弃权仍计；E-REF-UNKNOWN / E-CEX-INVALID 等不实引用不计
declare -a LCODE=()
while IFS=' ' read -r ln code; do
  [ -n "$ln" ] && LCODE[$((10#$ln-1))]="${code:-}"
done < <($BIN verify "$TMPA" 2>/dev/null | awk '/^L[0-9]+/{n=$1; sub(/^L/,"",n); c=$3; if (c ~ /^E-/) print n" "c; else print n" "}')
CERTS=0; CIDX=0
while IFS= read -r cl; do
  case "$cl" in ''|'#'*) continue ;; esac
  CIDX=$((CIDX+1))
  code="${LCODE[$((CIDX-1))]:-}"
  allow=0
  case "$code" in ''|E-SCOPE*|E-OVF|E-INDETERMINATE*) allow=1 ;; esac
  [ "$allow" = 1 ] || continue
  case "$cl" in
    by:*) bid="${cl#by:}"; bid="${bid#"${bid%%[![:space:]]*}"}"
          case "$bid" in rule:*) bid="${bid#rule:}"; bid="${bid#"${bid%%[![:space:]]*}"}" ;; esac
          known_hit "$bid" && CERTS=$((CERTS+1)) ;;
    rule:*|cex:*|inj:*|surj:*|bij:*) CERTS=$((CERTS+1)) ;;
  esac
done < "$TMPA"
if [ "$TRI_N" -eq 0 ]; then
  SCORE=0
else
  SCORE=$((TRI_N*15)); [ "$SCORE" -gt 90 ] && SCORE=90
  [ "$CERTS" -gt 2 ] && CERTS=2
  SCORE=$((SCORE + CERTS*5)); [ "$SCORE" -gt 100 ] && SCORE=100
fi

if [ "$NOCLOCK" = 1 ]; then ELAPSED=0
else ELAPSED=$(( $(date +%s 2>/dev/null || printf '0') - T0 )); fi
[ "$ELAPSED" -lt 0 ] && ELAPSED=0

OBJ="{\"exam_id\":\"$EXAM_ID\",\"difficulty\":\"$DIFF\",\"seed\":$SEED,\"submit\":true,\"elapsed_s\":$ELAPSED,\"score\":$SCORE,\"verdicts\":$VJSON,\"diagnostics\":$DIAGS,\"human\":\"$HUMAN\",\"output\":\"$OUT\"}"

case "$OUT" in
  json)   printf '%s\n' "$OBJ" ;;
  ndjson) printf '{"event":"notice","exam_id":"%s","difficulty":"%s","human":"%s"}\n' "$EXAM_ID" "$DIFF" "$HUMAN"
          printf '{"event":"paper","exam_id":"%s","seed":%s}\n' "$EXAM_ID" "$SEED"
          printf '{"event":"submit","elapsed_s":%s}\n' "$ELAPSED"
          printf '{"event":"result","score":%s,"verdicts":%s}\n' "$SCORE" "$VJSON" ;;
  tri)    printf '%s\n' "$TRI"; err "score=$SCORE/100 · elapsed=${ELAPSED}s · human=$HUMAN" ;;
  *)      printf 'score=%s/100 · elapsed=%ss · difficulty=%s · human=%s · seed=%s\n' \
                "$SCORE" "$ELAPSED" "$DIFF" "$HUMAN" "$SEED" ;;
esac
# 0 分是成绩，不是系统失败 —— exit 0
exit 0
