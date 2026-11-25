#!/usr/bin/env bash
# ltir.sh —— 一键启动（自带寻址 + 底片收容，0.0.9）
#   ./ltir.sh           一键：SHA 门禁 → 编译 → SMOKE → 演示
#   ./ltir.sh b         只编译      ./ltir.sh t   只自测
#   ./ltir.sh d         只演示      ./ltir.sh c   只查内核 SHA
#   ./ltir.sh v        版本（读 VERSION）
#   ./ltir.sh u        卸载（转交 uninstall.sh）
#   ./ltir.sh tri       打印「当前终端只输出三态」的设置命令
# 平台：Linux + Termux（Windows 分支冻结，不维护）
set -e
cd "$(cd "$(dirname "$0")" && pwd)"

IS_TERMUX=0
case "${PREFIX:-}" in *com.termux*) IS_TERMUX=1 ;; esac
[ -n "${TERMUX_VERSION:-}" ] && IS_TERMUX=1
case "$(uname -o 2>/dev/null || echo)" in Android) IS_TERMUX=1 ;; esac

case "$(pwd)" in /storage/emulated/*|/sdcard/*|/mnt/sdcard/*)
  echo "检测到项目在 noexec 存储（$(pwd)）——二进制在此永远执行不了"
  DEST="$HOME/ltir006_lab"
  if [ "$(pwd)" != "$DEST" ]; then
      if [ -f "$DEST/LTIR_STAMP" ] && cmp -s ./LTIR_STAMP "$DEST/LTIR_STAMP"; then
          echo "底片一致 → 直接入驻 $DEST（不重搬，保护新增文件）"
      else
          echo "底片缺失或不一致 → 自动重搬 → $DEST"
          rm -rf "$DEST.tmp"; mkdir -p "$DEST.tmp"
          for f in ./core ./fixtures ./axioms ./src ./tests ./Makefile ./ltir.sh \
                   ./termux ./BACKLOG.md ./AXIOM_NOTES.md ./VERSION ./LTIR_STAMP \
                   ./uninstall.sh ./GITHUB_INTRO.md \
                   ./RUN_core_sha.txt ./README.md ./SYNTAX.md ./QUICK_TOUR.md \
                   ./lab_codes.tsv ./domains.tsv ./RUNRESULT.md; do
            [ -e "$f" ] && cp -r "$f" "$DEST.tmp/"
          done
          for f in Makefile src core axioms termux BACKLOG.md lab_codes.tsv domains.tsv \
                   VERSION LTIR_STAMP ltir.sh uninstall.sh GITHUB_INTRO.md; do
            [ -e "$DEST.tmp/$f" ] || { echo "E-MOVE: 必备件缺失：$f"; rm -rf "$DEST.tmp"; exit 1; }
          done
          [ -d "$DEST" ] && rm -rf "$DEST"
          mv "$DEST.tmp" "$DEST"
      fi
      cd "$DEST"
  fi
  chmod +x ltir.sh ;;
esac

BIN=./ltir006
banner() { printf '\n\033[1;32m== %s ==\033[0m\n' "$1"; }
build() {
  if [ "$IS_TERMUX" = 1 ]; then
    banner "编译（Termux：clang++ 自适应）"; make -s CXX=clang++ ltir006
  else
    banner "编译"; make -s ltir006
  fi
  echo "OK: $BIN"
}
check()   { banner "内核 SHA 门禁"; sha256sum -c RUN_core_sha.txt --quiet && echo "OK: 11 文件与取证一致"; }
test_()   { [ -x "$BIN" ] || build; banner "SMOKE"; bash tests/smoke009.sh; }
demo()    {
  banner "演示（fixtures/demo.ltir）"
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
case "${1:-run}" in
  b) build ;;
  c) check ;;
  t) test_ ;;
  d) [ -x "$BIN" ] || build; demo ;;
  v) cat VERSION; [ -x "$BIN" ] || build; $BIN --version ;;
  R) rm -rf "$HOME/ltir006_lab"; exec bash ./ltir.sh ;;
  u) exec bash ./uninstall.sh ;;
  tri) echo 'export LTIR_OUTPUT=tri'; echo '复制到终端执行：本终端所有 verify 只输出 True/False/UNDECIDED'; echo '恢复：export LTIR_OUTPUT=normal' ;;
  run|"") check && build && test_ && demo; banner "全流程完成" ;;
  *) echo "未知命令: $1（b/c/t/d/v/R/u/tri 或留空=一键）"; exit 2 ;;
esac
