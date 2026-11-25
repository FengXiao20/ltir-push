#!/usr/bin/env bash
# LTIR 卸载（Linux / Termux）
# 只删收容目录与文件管理器 hook；不碰用户自己的 .ltir 卷、不碰解压目录。
set -e
cd "$HOME" 2>/dev/null || true   # 别在被删的目录里执行
LAB="$HOME/ltir006_lab"
HOOK="$HOME/bin/termux-file-editor"
echo "LTIR 卸载：保留你自己写的 .ltir 卷与外层解压目录"
if [ -d "$LAB" ]; then rm -rf "$LAB"; echo "已删 $LAB"; else echo "$LAB 不存在，跳过"; fi
if [ -f "$HOOK" ]; then rm -f "$HOOK"; echo "已删 $HOOK"; else echo "$HOOK 不存在，跳过"; fi
echo "完成。外层解压目录请自行删除。"
