#!/usr/bin/env bash
# ltir.sh —— 一键启动（自带寻址 + 底片收容，0.0.9）
#   ./ltir.sh           一键：SHA 门禁 → 编译 → SMOKE → 演示
#                       本地默认就画 \b 进度条；全绿只吐一行汇总，失败才展开
#   ./ltir.sh b / t / d / c   只编译 / 只自测 / 只演示 / 只查 SHA
#   ./ltir.sh verbose   满屏 ✓（与进度条互斥）   ./ltir.sh env   自报识别到的环境
#   ./ltir.sh v / u / tri     版本 / 卸载 / 三态输出设置命令
#   LTIR_LANG=zh|en bash ltir.sh   或  ./ltir.sh --lang=en verbose
#   LTIR_UI_BAR=off / LTIR_UI=off / CI=1   关条 / 全静默
# 平台：Linux + Termux，单包交付
# 输出层：ltir_ui.sh（stderr=条/标题/汇总/失败块，stdout 不写；非 TTY 自动静默）
set -e

# --lang 必须在加载输出层之前生效（语言在加载时识别一次）
while [ $# -gt 0 ]; do
  case "$1" in
    --lang=*) export LTIR_LANG="${1#*=}"; shift ;;
    *) break ;;
  esac
done

cd "$(cd "$(dirname "$0")" && pwd)"
. ./ltir_ui.sh

IS_TERMUX=0
case "${PREFIX:-}" in *com.termux*) IS_TERMUX=1 ;; esac
[ -n "${TERMUX_VERSION:-}" ] && IS_TERMUX=1
case "$(uname -o 2>/dev/null || echo)" in Android) IS_TERMUX=1 ;; esac

# 环境识别一行（只在真 TTY / 非 CI 时给，Agent 管道里不加噪音）
if [ -t 2 ] && [ "${CI:-}" != 1 ] && [ "${LTIR_UI_VERBOSE:-0}" != 1 ]; then
  ui_t env >&2; printf '\n' >&2
fi

banner() {
  if [ -t 2 ] && [ "${CI:-}" != 1 ]; then
    printf '\n\033[1;32m== %s ==\033[0m\n' "$1" >&2
  else
    printf '\n== %s ==\n' "$1" >&2
  fi
}

case "$(pwd)" in /storage/emulated/*|/sdcard/*|/mnt/sdcard/*)
  ui_t noexec "$(pwd)" >&2; printf '\n' >&2
  DEST="$HOME/ltir_lab"
  if [ "$(pwd)" != "$DEST" ]; then
      # 底片 + 输出层「内容哈希」双重比对：MT 覆盖外存后不必手敲 rm -rf
      _src_h=''; _dst_h=''
      [ -f ./ltir_ui.sh ]      && { _src_h=$(sha256sum ./ltir_ui.sh);      _src_h=${_src_h%% *}; }
      [ -f "$DEST/ltir_ui.sh" ] && { _dst_h=$(sha256sum "$DEST/ltir_ui.sh"); _dst_h=${_dst_h%% *}; }
      if [ -f "$DEST/LTIR_STAMP" ] && cmp -s ./LTIR_STAMP "$DEST/LTIR_STAMP" \
         && [ -n "$_src_h" ] && [ "$_src_h" = "$_dst_h" ]; then
          ui_t move_same "$DEST" >&2; printf '\n' >&2
      else
          ui_t move_diff "$DEST" >&2; printf '\n' >&2
          rm -rf "$DEST.tmp"; mkdir -p "$DEST.tmp"
          for f in ./core ./fixtures ./axioms ./src ./tests ./Makefile ./ltir.sh \
                   ./ltir_ui.sh ./termux ./BACKLOG.md ./AXIOM_NOTES.md ./VERSION ./LTIR_STAMP \
                   ./uninstall.sh ./GITHUB_INTRO.md \
                   ./RUN_core_sha.txt ./README.md ./SYNTAX.md ./QUICK_TOUR.md \
                   ./lab_codes.tsv ./domains.tsv ./RUNRESULT.md; do
            [ -e "$f" ] && cp -r "$f" "$DEST.tmp/"
          done
          for f in Makefile src core axioms termux BACKLOG.md lab_codes.tsv domains.tsv \
                   VERSION LTIR_STAMP ltir.sh ltir_ui.sh uninstall.sh GITHUB_INTRO.md; do
            [ -e "$DEST.tmp/$f" ] || { ui_t miss "$f" >&2; printf '\n' >&2; rm -rf "$DEST.tmp"; exit 1; }
          done
          [ -d "$DEST" ] && rm -rf "$DEST"
          mv "$DEST.tmp" "$DEST"
      fi
      cd "$DEST"
  fi
  chmod +x ltir.sh ;;
esac

BIN=./ltir006
build() {
  banner "$(ui_t compile)"
  if [ "$IS_TERMUX" = 1 ]; then bash ./ltir_ui.sh compile clang++
  else                        bash ./ltir_ui.sh compile g++; fi
}
check()   { banner "$(ui_t sha_gate)"; sha256sum -c RUN_core_sha.txt --quiet && { ui_t ok_sha >&2; printf '\n' >&2; }; }
test_()   { [ -x "$BIN" ] || build; banner "$(ui_t smoke)"; bash ./ltir_ui.sh smoke tests/smoke009.sh; }
demo_verbose() {
  banner "$(ui_t demo)"
  $BIN verify fixtures/demo.ltir
  banner "详细档（前 12 行）"
  $BIN verify fixtures/demo.ltir --explain | head -12
  banner "zf/zfc 台账保留"
  $BIN verify fixtures/zfc.ltir
  printf '\n单表达式： '; $BIN eval '\frac{1}{2}+\frac{1}{3}'
  printf '三角：     '; $BIN eval 'sin(2*pi/3)'
  printf '常量折叠： '; $BIN eval '√(16) + π^0'
  printf '复数：     '; $BIN eval '(1+i)^2'
}
demo()    {
  if [ "${LTIR_UI_VERBOSE:-0}" = 1 ]; then demo_verbose; else
    banner "$(ui_t demo)"; bash ./ltir_ui.sh demo fixtures/demo.ltir
  fi
}
case "${1:-run}" in
  b) build ;;
  c) check ;;
  t) test_ ;;
  d) [ -x "$BIN" ] || build; demo ;;
  v) cat VERSION; [ -x "$BIN" ] || build; $BIN --version ;;
  R) rm -rf "$HOME/ltir_lab"; exec bash ./ltir.sh ;;
  u) exec bash ./uninstall.sh ;;
  ui) bash ./ltir_ui.sh selftest ;;
  env) bash ./ltir_ui.sh env ;;
  verbose) export LTIR_UI_VERBOSE=1; exec bash ./ltir.sh ;;
  tri) printf '%s\n' 'export LTIR_OUTPUT=tri' "$(ui_t tri1)" "$(ui_t tri2)" ;;
  run|"")
    if check && build && test_ && demo; then
      banner "$(ui_t done)"
    else
      rc=$?
      printf 'FAIL: pipeline aborted (exit %s)\n' "$rc" >&2
      exit "$rc"
    fi ;;
  *) ui_t unknown "$1" >&2; printf '\n' >&2; exit 2 ;;
esac
