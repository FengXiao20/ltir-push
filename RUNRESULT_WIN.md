# RUNRESULT · LTIR 0.0.6 强化修复版 · Windows 10+ 交付

**结论：单文件自释放安装程序已产出，静态验收全过；真 Windows 未实跑，待回填。**

## 一、构建环境（如实）

| 项 | 值 |
|---|---|
| 构建主机 | Linux 沙盒（x86_64，g++ 11.4.0 仅用于 Linux 回归） |
| 交叉工具链 | zig 0.17.1，`-target x86_64-windows-gnu`，`-std=c++17 -O2` |
| 目标 | Win10+ / x86_64 / console 子系统 |
| 依赖 | 零第三方；`advapi32.dll` 用 `LoadLibraryA` 动态装载，链接期不需要 mingw 导入库 |
| 内核 | 11 件 SHA 门禁 `OK: 11 文件与取证一致`（`aecc9726…`） |

`file` 结果：`win/ltir006.exe` / `win/stub.exe` 均为
`PE32+ executable (console) x86-64, for MS Windows`。

## 二、产物清单

| 文件 | 字节 | 说明 |
|---|---|---|
| `ltir006_setup.exe` | 4,217,653 | **唯一交付 exe** = stub + payload + 16B 尾标 |
| `win/stub.exe` | 1,177,088 | 安装器 / 卸载器本体（内嵌于 setup） |
| `ltir006.exe` | 2,825,216 | 程序本体（内嵌于 payload） |
| payload | 3,040,549 | 33 件 `ZIP_STORED`，无压缩 |
| 交付 zip | 见压缩包属性 | `ltir006_setup.exe` + `README_WIN.md` + 本文件 |

payload 33 件含：`ltir006.exe`、`lab_codes.tsv`、
`domains.tsv`、`core/` 11 件、`fixtures/`、`src/`、`tests/`、`win/` 源码与构建脚本、
`README_WIN.md` 及全套图纸文档。

## 三、静态验收（逐项通过）

1. `MZ` 头 + PE32+ x86-64，两个内嵌 PE ✓
2. 尾标 `LTIRPAY!` + `<Q` 长度 = payload 实际长度 ✓
3. **按 `installer.cpp` 同款 `rd16/rd32` 读法**（不借助 zipfile）逐项解析 33 件，
   local header / central directory 走位接 EOCD ✓
4. 33 件 `method=0`，与沙盒源文件**逐字节一致** ✓
5. 内嵌 `ltir006.exe` 取回后仍是 `MZ` ✓
6. 中文 UTF-8 串确已编进 PE：`按任意键退出` / `安装目录` / `卸载完成`
   （`strings` 默认只列 ASCII，用 `grep -a` 取证） ✓
7. 动态装载字符串 `advapi32.dll` / `LoadLibraryA` / `RegCreateKeyExA` 在，
   构建命令无 `-ladvapi32` ✓
8. **同源源码 Linux 回归**：`0 error / 0 warning`，**69 / 69 项满足** ✓
9. 内核 SHA 门禁 `OK: 11 文件与取证一致` ✓

## 四、功能对照（代码级，非真机）

| 要求 | 落点 |
|---|---|
| 只交一个 exe | `ltir006_setup.exe` 自带全部目录 |
| 选盘 / 选目录 | 交互问答；静默 `/S /D=D:\LTIR` |
| 是否双击打开运行 | 交互 `Y/n`；写 `HKCU\Software\Classes\.ltir`，免管理员 |
| 双击 `.ltir` 直接运行 | 关联 → `ltir006.exe open 卷.ltir` → 判卷三态 |
| 默认按任意键退出 | `exit_mode: key` + `_getch()` |
| 可设 N 秒自动结束 | `exit_mode: auto` + `Sleep(N*1000)`，0–3600 秒 |
| JSON 配置 | 安装目录 `ltir006.json`，扁平 JSON，零第三方解析 |
| 命令改配置 | `config --show` / `--set exit=|seconds=|associate=` |
| 命令执行 LTIR | `verify` / `eval` / `open` / `--version` |
| 卸载命令 + 卸载程序 | `ltir006.exe uninstall`；`uninstall.exe`（安装器副本，靠文件名进卸载模式） |
| 图标 | 不做（本令明确） |

**双击时的路径坑已修**：Windows 把 cwd 给成卷所在目录，资源一律按
`GetModuleFileNameA` 取 exe 目录，`lab_codes.tsv` / `domains.tsv` / JSON 不会读丢。

## 五、构建曲折（按 .tools 教训，拿数据说话）

- 首轮构建报 `unable to find dynamic system library "advapi32"`，原因是 zig
  的 mingw 只有 `.def`、无现成导入库 → 改成 `LoadLibraryA` 动态装载，两个 PE 这才编通。
- zig 交叉编译 libc++ / compiler_rt 极慢，`/data/workspace` 写速 100 MB/s、
  `/tmp` 990 MB/s；平台多次 502 打断，缓存 obj 数从 16 → 37 逐轮累积。
  最终把工具链 + 缓存整体搬进 `/tmp` 完成链接，产物时间戳 21:58。
- 沙盒**没有 Wine / Windows**，两个 PE 一刻也没在真机执行过 —— 上面所有
  「通过」都是静态取证 + Linux 同源回归，不冒充真机验收。

## 六、真机待回填（有 bug 直接反馈）

1. 双击 `ltir006_setup.exe` → 选盘 / 选目录 / 是否关联 → 装完目录是否齐 35 件（33 件 payload + 安装期生成的 `ltir006.json` / `uninstall.exe`）
2. 双击任意 `.ltir` → 是否出三态 + `按任意键退出`（默认不闪退）
3. `ltir006.exe config --set exit=auto --set seconds=3` → JSON 是否改写、是否 3 秒自动关
4. `ltir006.exe config --set associate=false` → 关联是否解除
5. `uninstall.exe` 双击 / `ltir006.exe uninstall` → 关联与文件是否清干净
6. 中文解释与 `√ π ∞` 在 Win10 控制台（`CP_UTF8`）显示是否正常
7. 装到含空格路径（如 `D:\My LTIR\`）时引号是否够用

## 七、未尽事项（只列，不做）

- 真机六项回填（上节）
- 安装程序图形界面（当前为控制台问答）
- 图标、版本资源、数字签名 / SmartScreen 白名单
- Win7 / Win8（本令只 Win10+）
- 历史挂账：i18n 钩子、P2/P3 数学模块、`text` 自动 caret 推断、
  C++ 性能核预留位、浏览器宿主
- `.ltir` 卷的「打开方式」多版本共存时的关联冲突裁决
