#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""组装 ltir006_setup.exe（单文件自释放）= stub.exe + stored zip + 16B 尾标
并做「installer.cpp 同款读法」的静态自洽校验，最后打交付 zip。
在 Linux 沙盒侧跑，产出的是真 PE 文件（不在本机执行）。
"""
import os, sys, zipfile, struct, io, shutil

ROOT = "/data/workspace/ltir006"
WIN  = os.path.join(ROOT, "win")
MAGIC = b"LTIRPAY!"

# payload 内的路径 -> 沙盒源文件
PAY = {}

def add(rel, src):
    if not os.path.isfile(src):
        raise SystemExit("[E-EXT] 缺件: " + src)
    PAY[rel.replace("\\", "/")] = src

# ① 程序本体（Windows PE）
add("ltir006.exe", os.path.join(WIN, "ltir006.exe"))
# ② 资源与完整目录（排除 Linux 二进制、win/ 构建产物、日志、缓存）
SKIP_FILE = {"ltir006", "ltir006_setup.exe", "RUNRESULT_WIN.md"}
# 前两件是构建中间件；RUNRESULT_WIN.md 是交付侧取证，只进压缩包不进安装目录
# exe / zip / log / rc 是构建中间件，不进安装目录；win/ 下的源码与脚本照带
SKIP_EXT  = {".log", ".rc", ".o", ".obj", ".pyc", ".exe", ".zip"}
for base, dirs, files in os.walk(ROOT):
    relbase = os.path.relpath(base, ROOT)
    if relbase == ".": relbase = ""
    dirs[:] = [d for d in dirs if d not in ("__pycache__", ".git")]
    for f in files:
        if relbase == "" and f in SKIP_FILE: continue
        if os.path.splitext(f)[1].lower() in SKIP_EXT: continue
        src = os.path.join(base, f)
        rel = (relbase + "/" + f).lstrip("/") if relbase else f
        add(rel, src)

# ③ 写成 stored zip（method=0，installer.cpp 的 unzipStored 只认 store）
buf = io.BytesIO()
with zipfile.ZipFile(buf, "w", compression=zipfile.ZIP_STORED) as z:
    for rel, src in sorted(PAY.items()):
        # 固定 mtime：同一批源文件重打得到同一份 setup.exe，便于哈希核对
        zi = zipfile.ZipInfo(rel, date_time=(2026, 11, 24, 0, 0, 0))
        zi.compress_type = zipfile.ZIP_STORED
        zi.external_attr = 0o644 << 16
        with open(src, "rb") as fp:
            z.writestr(zi, fp.read())
payload = buf.getvalue()

stub = os.path.join(WIN, "stub.exe")
if not os.path.isfile(stub):
    raise SystemExit("[E-EXT] stub.exe 未编译")
with open(stub, "rb") as fp:
    stub_bytes = fp.read()
if stub_bytes[:2] != b"MZ":
    raise SystemExit("[E-SYN] stub.exe 不是 PE（缺 MZ）")

setup = os.path.join(ROOT, "ltir006_setup.exe")
with open(setup, "wb") as fp:
    fp.write(stub_bytes)
    fp.write(payload)
    fp.write(MAGIC + struct.pack("<Q", len(payload)))

# ── 静态自洽：完全按 installer.cpp 的 readPayload / unzipStored 逻辑读一遍 ──
with open(setup, "rb") as fp:
    blob = fp.read()
tail = blob[-16:]
assert tail[:8] == MAGIC, "尾标 magic 不符"
plen = struct.unpack("<Q", tail[8:])[0]
assert plen == len(payload), "payload 长度不符"
p = blob[-16-plen:-16]
with zipfile.ZipFile(io.BytesIO(p), "r") as z:
    names = z.namelist()
    assert all(i.compress_type == zipfile.ZIP_STORED for i in z.infolist()), "非 stored 项"
    assert names == sorted(names), "顺序不符"
    assert "ltir006.exe" in names and "lab_codes.tsv" in names and "domains.tsv" in names
    assert z.read("ltir006.exe")[:2] == b"MZ", "内嵌程序不是 PE"

# eocd / central directory 手工复核（对应 installer.cpp 的 rd16/rd32）
e = p.rfind(b"PK\x05\x06")
assert e >= 0
n = struct.unpack_from("<H", p, e+10)[0]
pos = struct.unpack_from("<I", p, e+16)[0]
assert n == len(PAY), "central directory 条目数不符 (%d vs %d)" % (n, len(PAY))
for k in range(n):
    assert struct.unpack_from("<I", p, pos)[0] == 0x02014b50
    method = struct.unpack_from("<H", p, pos+10)[0]
    assert method == 0, "method != 0 (store)"
    fnl, exl, cml = (struct.unpack_from("<H", p, pos+o)[0] for o in (28,30,32))
    pos += 46 + fnl + exl + cml
assert pos == e, "central directory 走位与 EOCD 不接"

# ── 交付 zip ──
outzip = "/data/workspace/ltir006_win10_0.0.6_强化修复版.zip"
with zipfile.ZipFile(outzip, "w", compression=zipfile.ZIP_DEFLATED) as z:
    z.write(setup, "ltir006_setup.exe")
    for f in ("README_WIN.md", "RUNRESULT_WIN.md"):
        pth = os.path.join(ROOT, f)
        if os.path.isfile(pth): z.write(pth, f)

print("OK  setup =", setup, os.path.getsize(setup), "B")
print("    payload =", len(payload), "B /", len(PAY), "件 stored")
print("    zip     =", outzip, os.path.getsize(outzip), "B")
print("    PE MZ  :", blob[:2], " 尾标 :", tail[:8].decode(), plen)
