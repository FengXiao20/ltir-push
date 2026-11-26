#!/usr/bin/env bash
# ltir_ui.sh —— LTIR 输出层（套用于 0.1.1，不推进版本号）
#
# 契约
#   stderr = 进度条 / 阶段标题 / 一行汇总 / 失败块   stdout 永远不写（留给数据与回执）
#   进度条：默认本地就画，纯 \b 退格重绘 —— 不用 \r、不用 \033[K、不用删除转义
#           帧恒 25 列纯 ASCII（[####----------]  42%），窄屏不折行，故 \b 真能退回行首
#           编译 4 帧 + SMOKE 20 帧（5% 粒度），绝不逐 case 刷一屏
#   关闭：非 TTY / CI=1 / TERM=dumb / LTIR_UI=off / LTIR_UI_BAR=off → 零 \b、零 ANSI
#   后门：LTIR_UI_VERBOSE=1 → 原样满屏 ✓（与条互斥）
#   语言：TZ > /etc/localtime > LANG 启发式识别，中英文只换「人话层」
#         True/False/UNDECIDED、E-XXX 错误码、demo 冻结基线、JSON 字段一律不翻译
#         硬覆盖优先：LTIR_LANG=zh|en（启发式不是真理，绝不替用户做主）
#   零第三方依赖、零 /tmp、不动 core 11 件、不动 RUN_core_sha.txt
#
# 子命令：compile [cxx] | smoke [script] [total] | demo [paper] | env | selftest

LTIR_BIN=${LTIR_BIN:-./ltir006}
LTIR_WIDTH=${LTIR_WIDTH:-18}
LTIR_UI_DIR=${LTIR_UI_DIR:-./.ltir_ui}
LTIR_SMOKE_TOTAL=${LTIR_SMOKE_TOTAL:-420}
LTIR_DEMO_BASE=${LTIR_DEMO_BASE:-'24 lines: 20 True, 3 False, 1 UNDECIDED'}

# 帧宽 = '[' + LTIR_WIDTH + ']' + ' ' + '%3d%%'(4)
LTIR_FRAME_W=$((LTIR_WIDTH + 7))
LTIR_BS=''
for (( _ltir_i=0; _ltir_i<LTIR_FRAME_W; _ltir_i++ )); do LTIR_BS+=$'\b'; done
unset _ltir_i

# ---------------------------------------------------------------- 语言层

# 只认明确的中国/港澳台时区与 +8 写法；单独 'CST' 不认（美国中部/古巴也用）
ltir_lang_detect() {
  local v='' loc=''
  case "${LTIR_LANG:-}" in
    zh|zh_*|ZH|CN|Chinese|chinese)      LTIR_LANG_DETECTED=zh; LTIR_LANG_SRC='override'; return 0 ;;
    en|en_*|EN|C|English|english)       LTIR_LANG_DETECTED=en; LTIR_LANG_SRC='override'; return 0 ;;
  esac
  v=${TZ:-}
  if [ -z "$v" ] && [ -L /etc/localtime ]; then
    v=$(readlink /etc/localtime 2>/dev/null || true)
  elif [ -z "$v" ] && [ -r /etc/timezone ]; then
    v=$(cat /etc/timezone 2>/dev/null || true)
  fi
  case "$v" in
    *Asia/Shanghai*|*Asia/Chongqing*|*Asia/Harbin*|*Asia/Urumqi*|*Asia/Kashgar*|\
    *Asia/Hong_Kong*|*Asia/Macau*|*Asia/Macao*|*Asia/Taipei*|*PRC*|\
    *CST-8*|*GMT-8*|*Etc/GMT-8*|*UTC+8*|*+0800*|*+08:00*)
      LTIR_LANG_DETECTED=zh; LTIR_LANG_SRC="tz:$v"; return 0 ;;
  esac
  loc="${LANG:-}${LC_ALL:-}${LC_MESSAGES:-}"
  case "$loc" in
    zh*|ZH*) LTIR_LANG_DETECTED=zh; LTIR_LANG_SRC="locale:$loc"; return 0 ;;
  esac
  LTIR_LANG_DETECTED=en
  if [ -n "$v" ]; then LTIR_LANG_SRC="tz:$v"; else LTIR_LANG_SRC='default'; fi
  return 0
}
ltir_lang_detect

# 人话层词条；机器词一律原样
ui_t() {
  local k=${1:-}
  if [ "${LTIR_LANG_DETECTED:-en}" = zh ]; then
    case "$k" in
      sha_gate)    printf '内核 SHA 门禁' ;;
      compile)     printf '编译（4 阶段校验）' ;;
      smoke)       printf 'SMOKE（只吐失败块）' ;;
      demo)        printf '演示（纯检查）' ;;
      done)        printf '全流程完成' ;;
      env)         printf '[env] %s → UI=%s（LTIR_LANG=zh|en 可覆盖）' "$LTIR_LANG_SRC" "$LTIR_LANG_DETECTED" ;;
      ok_sha)      printf 'OK: 11 文件与取证一致' ;;
      ok_smoke)    printf 'OK, 0 failed · %s / %s · demo/回归基线一致' "$2" "$3" ;;
      ok_demo)     printf 'OK: demo %s（冻结基线一致）' "$2" ;;
      fail_demo)   printf 'FAIL: demo 基线漂移，展开 diff' ;;
      fail_compile) printf 'FAIL: 编译+链接失败，尾部输出：' ;;
      fail_smoke)  printf 'FAIL: SMOKE 中断 (exit %s)，尾部：' "$2" ;;
      fail_block)  printf '\n---- 失败块 (%s 项) ----\n' "$2" ;;
      fail_total)  printf 'FAILED: %s 项 · 完整日志 %s' "$2" "$3" ;;
      noexec)      printf '检测到项目在 noexec 存储（%s）——二进制在此永远执行不了' "$2" ;;
      move_same)   printf '底片 + 输出层哈希一致 → 直接入驻 %s（不重搬）' "$2" ;;
      move_diff)   printf '底片/输出层不一致 → 自动重搬 → %s' "$2" ;;
      miss)       printf 'E-MOVE: 必备件缺失：%s' "$2" ;;
      unknown)     printf '未知命令: %s（b/c/t/d/v/R/u/ui/env/verbose/tri 或留空=一键）' "$2" ;;
      tri1)        printf '复制到终端执行：本终端所有 verify 只输出 True/False/UNDECIDED' ;;
      tri2)        printf '恢复：export LTIR_OUTPUT=normal' ;;
      *)           printf '%s' "$k" ;;
    esac
  else
    case "$k" in
      sha_gate)    printf 'Core SHA gate' ;;
      compile)     printf 'Compiling (4 stages)' ;;
      smoke)       printf 'SMOKE (fail-block only)' ;;
      demo)        printf 'Demo (check-only)' ;;
      done)        printf 'All done' ;;
      env)         printf '[env] %s -> UI=%s (override: LTIR_LANG=zh|en)' "$LTIR_LANG_SRC" "$LTIR_LANG_DETECTED" ;;
      ok_sha)      printf 'OK: 11 files match evidence' ;;
      ok_smoke)    printf 'OK, 0 failed · %s / %s · frozen baselines intact' "$2" "$3" ;;
      ok_demo)     printf 'OK: demo %s (frozen baseline intact)' "$2" ;;
      fail_demo)   printf 'FAIL: demo baseline drift, showing diff' ;;
      fail_compile) printf 'FAIL: compile+link failed, tail:' ;;
      fail_smoke)  printf 'FAIL: SMOKE aborted (exit %s), tail:' "$2" ;;
      fail_block)  printf '\n---- failure block (%s items) ----\n' "$2" ;;
      fail_total)  printf 'FAILED: %s items · full log %s' "$2" "$3" ;;
      noexec)      printf 'Project on noexec storage (%s) -- binary can never run here' "$2" ;;
      move_same)   printf 'Stamp + UI hash match -> reuse %s (no re-stage)' "$2" ;;
      move_diff)   printf 'Stamp/UI mismatch -> auto re-stage -> %s' "$2" ;;
      miss)       printf 'E-MOVE: required file missing: %s' "$2" ;;
      unknown)     printf 'Unknown command: %s (b/c/t/d/v/R/u/ui/env/verbose/tri/exam, blank = all)' "$2" ;;
      tri1)        printf 'Run in this terminal: every verify prints only True/False/UNDECIDED' ;;
      tri2)        printf 'Restore: export LTIR_OUTPUT=normal' ;;
      *)           printf '%s' "$k" ;;
    esac
  fi
}

# ---------------------------------------------------------------- 开关 / \b 条

# ---------------- 模式优先级（高 → 低，命中即停）----------------
#   1. CI=1 / LTIR_UI=off        → 绝对静默（Agent 管道纪律，--verbose 也翻不掉它）
#   2. 非 TTY / TERM=dumb         → 静默
#   3. LTIR_UI_VERBOSE=1          → 满屏 ✓（与条互斥）
#   4. LTIR_UI_BAR=off            → 有标题无条
#   5. 默认 TTY                   → \b 进度条
ltir_ui_mode() {   # stdout: MODE<TAB>REASON
  if [ "${CI:-}" = 1 ]; then printf 'silent\tCI=1'; return 0; fi
  if [ "${LTIR_UI:-on}" = off ]; then printf 'silent\tLTIR_UI=off'; return 0; fi
  if [ "${TERM:-}" = dumb ]; then printf 'silent\tTERM=dumb'; return 0; fi
  if [ ! -t 2 ] && [ "${LTIR_FORCE_TTY:-0}" != 1 ]; then
    printf 'silent\tstderr 非 TTY'; return 0
  fi
  if [ "${LTIR_UI_VERBOSE:-0}" = 1 ]; then printf 'verbose\tLTIR_UI_VERBOSE=1'; return 0; fi
  case "${LTIR_UI_BAR:-on}" in
    off|0|no|false) printf 'title-only\tLTIR_UI_BAR=%s' "${LTIR_UI_BAR}"; return 0 ;;
  esac
  printf 'bar\t默认 TTY'
  return 0
}

ltir_ui_on() {
  local m; m=$(ltir_ui_mode)
  case "$m" in silent*|verbose*) return 1 ;; esac
  return 0
}

LTIR_DREW=0

ltir_progress_on() {
  ltir_ui_on || return 1
  case "${LTIR_UI_BAR:-on}" in off|0|no|false) return 1 ;; esac
  return 0
}

# 纯 \b：先退 LTIR_FRAME_W 格，再重画等宽帧覆盖；没有任何 \r / \033[K / 删除转义
_ltir_draw() {
  ltir_progress_on || return 0
  local cur=$((10#$1)) total=$((10#$2)) done pct fill='' i frame
  (( total <= 0 )) && total=1
  (( cur > total )) && cur=$total
  done=$(( cur * LTIR_WIDTH / total ))
  pct=$(( cur * 100 / total ))
  for (( i=0; i<done; i++ )); do fill+='#'; done
  for (( i=done; i<LTIR_WIDTH; i++ )); do fill+='-'; done
  frame=$(printf '[%s] %3d%%' "$fill" "$pct")
  if [ "${LTIR_DREW:-0}" = 1 ]; then printf '%s' "$LTIR_BS" >&2; fi
  printf '%s' "$frame" >&2
  LTIR_DREW=1
}

_ltir_end() {
  [ "${LTIR_DREW:-0}" = 1 ] && printf '\n' >&2
  LTIR_DREW=0
  return 0
}

_ui_dir() { mkdir -p "$LTIR_UI_DIR" 2>/dev/null || LTIR_UI_DIR=.; }

# ---------------------------------------------------------------- 编译

ltir_ui_compile() {
  local cxx=${1:-${CXX:-g++}} log n=4
  _ui_dir; log="$LTIR_UI_DIR/compile.log"

  command -v "$cxx" >/dev/null 2>&1 || {
    printf 'FAIL: compiler not found: %s\n' "$cxx" >&2; return 2; }
  _ltir_draw 1 "$n"

  for f in src/ltir006.cpp src/value.hpp core/src/rational.cpp core/include/ltir/rational.hpp Makefile RUN_core_sha.txt; do
    [ -f "$f" ] || { _ltir_end; printf 'FAIL: source missing %s\n' "$f" >&2; return 2; }
  done
  sha256sum -c RUN_core_sha.txt --quiet 2>/dev/null || {
    _ltir_end; printf 'FAIL: core SHA gate failed\n' >&2; return 2; }
  _ltir_draw 2 "$n"

  : > "$log"
  if ! make -s CXX="$cxx" ltir006 >> "$log" 2>&1; then
    _ltir_end
    ui_t fail_compile >&2; printf '\n' >&2
    tail -20 "$log" >&2
    return 2
  fi
  _ltir_draw 3 "$n"

  [ -x "$LTIR_BIN" ] && "$LTIR_BIN" --version >/dev/null 2>&1 || {
    _ltir_end; printf 'FAIL: %s missing or not executable\n' "$LTIR_BIN" >&2; return 2; }
  _ltir_draw 4 "$n"
  _ltir_end
  printf 'OK: %s (%d/%d stages)\n' "$LTIR_BIN" "$n" "$n" >&2
  return 0
}

# ---------------------------------------------------------------- 测试

ltir_ui_smoke() {
  local script=${1:-tests/smoke009.sh} total=${2:-$LTIR_SMOKE_TOTAL}
  local log rcf failf tsv rc=0 fails=0 n=0 idx line j s dup step last_step=-1
  local plan_seen=0 p real_total
  _ui_dir
  log="$LTIR_UI_DIR/smoke_last.log"; rcf="$LTIR_UI_DIR/smoke_rc"; failf="$LTIR_UI_DIR/smoke_fail"
  tsv="$LTIR_UI_DIR/test_events.tsv"
  : > "$log"; : > "$failf"; printf '0\n' > "$rcf"; : > "$tsv"
  # 结构化事件通道：ck/ckc 认 LTIR_TSV 才双写；mock/旧脚本不认 → 自动退化文本解析
  export LTIR_TSV="$tsv"

  while IFS= read -r line; do
    n=$((n+1))
    printf '%s\n' "$line" >> "$log"
    if [[ $line == '__LTIR_RC__='* ]]; then
      printf '%s\n' "${line#*=}" > "$rcf"; continue
    fi
    if [[ $line == '__LTIR_PLAN__='* ]]; then
      p=${line#*=}
      [[ $p =~ ^[0-9]+$ ]] && { total=$p; plan_seen=1; }
      continue
    fi
    if [[ $line =~ ^[[:space:]]*\[([0-9]+)\][[:space:]] ]]; then
      idx=${BASH_REMATCH[1]}
      step=$(( 10#$idx * 20 / total ))   # 5% 粒度 → 最多 21 帧，不逐 case 刷
      [ "$step" -gt 20 ] && step=20
      if [ "$step" != "$last_step" ]; then _ltir_draw "$step" 20; last_step=$step; fi
    fi
    [[ $line == *'✗'* ]] && { fails=$((fails+1)); printf '%s\n' "$n" >> "$failf"; }
    [ "${LTIR_UI_VERBOSE:-0}" = 1 ] && printf '%s\n' "$line" >&2
  done < <( { set +e; bash "$script" 2>&1; r=$?; printf '__LTIR_RC__=%s\n' "$r"; } )
  unset LTIR_TSV

  rc=$(cat "$rcf")
  # 权威总数：TSV（实跑逐条，最准）> 收尾行正则（无计划/mock 时的兼容回退）> 计划/入参（中途进度）
  if [ -s "$tsv" ]; then
    total=$(grep -c '^TEST_' "$tsv" 2>/dev/null || true)
    fails=$(grep -c '^TEST_FAIL' "$tsv" 2>/dev/null || true)
  elif [ "$plan_seen" = 0 ]; then
    real_total=$(grep -E '^[0-9]+ / [0-9]+ 项满足' "$log" 2>/dev/null | tail -1 | sed 's#.* / \([0-9]*\) .*$#\1#')
    [ -n "$real_total" ] && total="$real_total"
  fi
  # 只有全绿才画满 100%；中途失败/提前退出时进度条冻结在最后一步
  if [ "$fails" = 0 ] && [ "$rc" = 0 ]; then _ltir_draw 20 20; fi
  _ltir_end

  if [ "${LTIR_UI_VERBOSE:-0}" = 1 ]; then
    printf 'VERBOSE: %s / %s items (exit %s)\n' "$total" "$total" "$rc" >&2
    [ "$rc" = 0 ] && [ "$fails" = 0 ] || return 1
    return 0
  fi

  if [ "$fails" = 0 ] && [ "$rc" = 0 ]; then
    ui_t ok_smoke "$total" "$total" >&2; printf '\n' >&2
    # 成功不留 405 行全量日志：只留一行精简摘要（时间 / 405 / rc / 版本）
    local ver ts
    ver=$(tr -d '\n' < VERSION 2>/dev/null || true); [ -n "$ver" ] || ver=unknown
    ts=$(date '+%Y-%m-%dT%H:%M:%S%z' 2>/dev/null) || ts=$(date)
    printf 'SMOKE OK · %s · %s/%s · rc=%s · ltir006 %s\n' "$ts" "$total" "$total" "$rc" "$ver" > "$log"
    rm -f "$failf" "$rcf" "$tsv" 2>/dev/null || true
    return 0
  fi
  if [ "$fails" = 0 ] && [ "$rc" != 0 ]; then
    ui_t fail_smoke "$rc" >&2; printf '\n' >&2
    tail -12 "$log" >&2
    return 2
  fi

  # 失败块：分组标题 + ✗ 行（自带 got/want）+ ^ 指位 / 解释续行；不吐相邻 ✓
  local -a L=() pick=()
  mapfile -t L < "$log"
  while read -r ln; do
    local g=$((ln-1))
    while (( g>=1 )); do
      [[ ${L[$((g-1))]} == '=='* ]] && break
      (( g-- ))
    done
    (( g>=1 )) && pick+=( "$g" )
    pick+=( "$ln" )
    for (( j=ln+1; j<=ln+3 && j<=${#L[@]}; j++ )); do
      line=${L[$((j-1))]}
      [[ $line =~ ^[[:space:]]*\[[0-9]+\][[:space:]]*[✓✗] ]] && break
      [[ $line == '=='* ]] && break
      pick+=( "$j" )
    done
  done < "$failf"
  ui_t fail_block "$fails" >&2
  mapfile -t pick_sorted < <(printf '%s\n' "${pick[@]}" | sort -n -u)
  for j in "${pick_sorted[@]}"; do printf '%s\n' "${L[$((j-1))]}" >&2; done
  ui_t fail_total "$fails" "$log" >&2; printf '\n' >&2
  return 1
}

# ---------------------------------------------------------------- demo

ltir_ui_demo() {
  local paper=${1:-fixtures/demo.ltir} out
  out=$( "$LTIR_BIN" verify "$paper" 2>/dev/null )
  if [[ $out == *"$LTIR_DEMO_BASE"* ]]; then
    ui_t ok_demo "$LTIR_DEMO_BASE" >&2; printf '\n' >&2
    return 0
  fi
  ui_t fail_demo >&2; printf '\n' >&2
  diff <(printf '%s\n' "$LTIR_DEMO_BASE") <(printf '%s\n' "$out") >&2 || true
  return 1
}

# ---------------------------------------------------------------- env 自报

ltir_ui_env() {
  local mr r m
  mr=$(ltir_ui_mode); r=${mr#*$'\t'}; m=${mr%%$'\t'*}
  printf 'UI=%s\nSRC=%s\nTZ=%s\nLANG=%s\nFRAME_W=%s\nBAR=%s\nMODE=%s\nMODE_REASON=%s\n' \
    "$LTIR_LANG_DETECTED" "$LTIR_LANG_SRC" "${TZ:-<unset>}" "${LANG:-<unset>}" \
    "$LTIR_FRAME_W" '\b-only' "$m" "$r"
}

# ---------------------------------------------------------------- 自测（6 项）

ltir_ui_selftest() {
  local d; _ui_dir; d="$LTIR_UI_DIR/selftest"; mkdir -p "$d"
  cat > "$d/ltir006" <<'MOCK'
#!/usr/bin/env bash
echo "L01 True"
echo "── 24 lines: 20 True, 3 False, 1 UNDECIDED ──"
MOCK
  chmod +x "$d/ltir006"
  cat > "$d/smoke.sh" <<'MOCK'
#!/usr/bin/env bash
echo "== 分组 =="
echo "  [01] ✓ a"
echo "  [02] ✓ b"
echo "  [03] ✗ c   got=<> want=<yes>"
echo "    A: 2+2==5 -> True"
echo "           ^"
echo "  [04] ✓ d"
echo "3 / 4 项满足"
MOCK
  chmod +x "$d/smoke.sh"

  printf '[1/6] 非 TTY 静默（无 \\b / 无 \\r） ... ' >&2
  local quiet
  quiet=$( CI=1 LTIR_UI_DIR="$d/w1" LTIR_BIN="$d/ltir006" bash "$0" smoke "$d/smoke.sh" 4 2>&1 )
  [[ $quiet != *$'\b'* && $quiet != *$'\r'* ]] || { echo NG; return 2; }
  [[ $quiet == *'FAILED: 1 项'* || $quiet == *'FAILED: 1 items'* ]] || { echo NG; return 2; }
  echo OK >&2

  printf '[2/6] 失败块只吐一段、成功行不漏 ... ' >&2
  [[ $quiet == *'got=<> want=<yes>'* && $quiet == *'^'* ]] || { echo NG; return 2; }
  [[ $quiet != *'[01] ✓'* ]] || { echo NG; return 2; }
  echo OK >&2

  printf '[3/6] demo 纯检查 ... ' >&2
  LTIR_BIN="$d/ltir006" bash "$0" demo >/dev/null 2>&1 || { echo NG; return 2; }
  echo OK >&2

  printf '[4/6] 全绿只给一行 ... ' >&2
  cat > "$d/smoke_ok.sh" <<'MOCK'
#!/usr/bin/env bash
echo "  [01] ✓ a"; echo "  [02] ✓ b"; echo "2 / 2 项满足"
MOCK
  local okout rc=0
  okout=$( CI=1 LTIR_UI_DIR="$d/w2" bash "$0" smoke "$d/smoke_ok.sh" 2 2>&1 ) || rc=$?
  [[ $rc -eq 0 && $okout == *'OK, 0 failed'* ]] || { echo "NG rc=$rc"; return 2; }
  echo OK >&2

  printf '[5/6] \\b 条：等宽帧 / 零 \\r / 零 \\033[K ... ' >&2
  local barout
  barout=$( CI= TERM=xterm LTIR_FORCE_TTY=1 LTIR_UI_DIR="$d/w3" bash -c '
    . "$1" >/dev/null 2>&1
    _ltir_draw 1 4; _ltir_draw 4 4; _ltir_end
  ' _ "$0" 2>&1 )
  [[ $barout == *$'\b'* ]] || { echo 'NG no backspace'; return 2; }
  [[ $barout != *$'\r'* && $barout != *$'\033[K'* ]] || { echo 'NG escape found'; return 2; }
  [[ $barout == *'100%'* ]] || { echo 'NG no 100%'; return 2; }
  echo OK >&2

  printf '[6/6] 时区识别中英文 + 硬覆盖优先 ... ' >&2
  local z y x
  z=$( TZ=Asia/Shanghai LANG=C LTIR_UI_DIR="$d/w4" bash -c '. "$1" >/dev/null 2>&1; printf "%s" "$LTIR_LANG_DETECTED"' _ "$0" )
  y=$( TZ=UTC LANG=C LTIR_UI_DIR="$d/w5" bash -c '. "$1" >/dev/null 2>&1; printf "%s" "$LTIR_LANG_DETECTED"' _ "$0" )
  x=$( TZ=Asia/Shanghai LANG=C LTIR_LANG=en LTIR_UI_DIR="$d/w6" bash -c '. "$1" >/dev/null 2>&1; printf "%s" "$LTIR_LANG_DETECTED"' _ "$0" )
  [[ $z = zh && $y = en && $x = en ]] || { echo "NG z=$z y=$y x=$x"; return 2; }
  echo OK >&2

  rm -rf "$d"
  echo 'selftest: 6/6 OK' >&2
  return 0
}

# 被 source 时只载函数，不执行子命令
if [ "${BASH_SOURCE[0]}" = "$0" ]; then
  case "${1:-}" in
    compile)  shift; ltir_ui_compile "$@" ;;
    smoke)    shift; ltir_ui_smoke "$@" ;;
    demo)     shift; ltir_ui_demo "$@" ;;
    env)      shift; ltir_ui_env "$@" ;;
    selftest) shift; ltir_ui_selftest "$@" ;;
    *) printf 'usage: bash ltir_ui.sh {compile [cxx]|smoke [script] [total]|demo [paper]|env|selftest}\n' >&2; exit 4 ;;
  esac
fi
