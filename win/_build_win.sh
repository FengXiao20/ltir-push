#!/usr/bin/env bash
# Windows PE 交叉编译（zig 0.17.1 / x86_64-windows-gnu / UCRT 由系统提供）
# 参数刻意与缓存探针一致（-O0 -fno-exceptions -fno-rtti），复用 _zigcache 已编译单元。
# 注册表 API 走动态装载，不需要 -ladvapi32 导入库。
cd /data/workspace/ltir006 || exit 9
Z=/data/workspace/_zigtool/ziglang/zig
export ZIG_GLOBAL_CACHE_DIR=/data/workspace/_zigcache
mkdir -p win

echo "START $(date +%T)" > win/_build_all.log
echo RUNNING > win/_build_all.rc

$Z c++ -target x86_64-windows-gnu -O0 -fno-exceptions -fno-rtti \
   -I core/include \
   -o win/ltir006.exe src/ltir006.cpp core/src/rational.cpp \
   >> win/_build_all.log 2>&1
echo $? > win/_build_main.rc

$Z c++ -target x86_64-windows-gnu -O0 -fno-exceptions -fno-rtti \
   -o win/stub.exe win/installer.cpp \
   >> win/_build_all.log 2>&1
echo $? > win/_build_stub.rc

echo "END $(date +%T)" >> win/_build_all.log
echo DONE > win/_build_all.rc
