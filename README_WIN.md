# LTIR 0.0.6 强化修复版 · Windows 10+ 版

**只交一个 exe：`ltir006_setup.exe`**。它是单文件自释放安装程序 —— 程序本体、
完整目录、资源文件、卸载程序全在这一个 exe 里，没有第二个安装包、没有依赖、
不需要管理员权限、不需要 .NET / VC 运行库。

语言层与 Linux / Termux 版**同一份源码**：四件套标签（`<calc>` / `<C_and_E>` /
`<answer>` / `<subject domain>`）、`S: E: P: Q:` 装饰、24 个领域、三态锁死
`True / False / UNDECIDED`、真值自产、`^` 显示宽度指位、`zf/zfc` 台账只登记不判，
全部照带。内核 11 件 SHA 门禁 `aecc9726…` 不变。

---

## 一、安装

双击 `ltir006_setup.exe`，按提示答四问：

```
安装盘符（只填字母） [C]:          ← 选盘
安装目录 [C:\LTIR]:                ← 选目录（可改到 D 盘等任意路径）
是否双击 .ltir 直接运行？[Y/n]:    ← 写文件关联（HKCU，Win10+ 免管理员）
运行结束后：1=按任意键退出（默认） 2=N 秒自动关闭:
自动关闭秒数 [5]:
```

默认行为：**按任意键退出**（双击跑完不会一闪而过）。装完目录里会有：

```
<安装目录>\
  ltir006.exe        ← 程序本体（命令入口）
  uninstall.exe      ← 卸载程序（安装器副本）
  ltir006.json       ← 配置（可由命令改）
  lab_codes.tsv      ← 错误码表（只读消费）
  domains.tsv        ← 24 个领域
  core\              ← 11 件内核（SHA 门禁取证）
  fixtures\          ← 示例卷 demo.ltir / zfc.ltir
  src\ tests\ 文档\  ← 完整源码与图纸
```

静默安装（给脚本用）：

```
ltir006_setup.exe /S /D=D:\LTIR /ASSOC=1 /EXIT=auto /SEC=8
```

## 二、双击 .ltir

安装时选了 `Y`，之后**双击任意 `.ltir` 卷**即：

```
LTIR 0.0.6 强化修复版 · D:\卷\a.ltir

L01 True
L02 False  E-VAL-MISMATCH
L03 UNDECIDED  E-INF
── 3 lines: 1 True, 1 False, 1 UNDECIDED ──

按任意键退出...          ← 默认；也可设成 N 秒自动关
```

Windows 双击时工作目录是**卷所在目录**，不是安装目录 —— 所以资源一律按
`ltir006.exe` 所在目录取，解释不会退化成只回码名。

## 三、命令执行 LTIR

```
cd /d <安装目录>
ltir006.exe verify 卷.ltir                 判卷（简明档）
ltir006.exe verify 卷.ltir --explain       详细档（^ 指位 + 码表解释）
ltir006.exe verify 卷.ltir --json          JSON
ltir006.exe verify 卷.ltir --lang=en       强制英语
ltir006.exe eval "1/3+1/6"                单表达式
ltir006.exe open 卷.ltir                  等同双击（含退出等待）
ltir006.exe --version                     → ltir006 0.0.6 强化修复版
```

## 四、改配置：JSON + 命令，两条路都通

`ltir006.json`（安装目录内，扁平 JSON，零第三方解析）：

```json
{
  "install_dir": "D:\\LTIR",
  "associate_ltir": true,
  "exit_mode": "key",
  "auto_seconds": 5,
  "version": "0.0.6 强化修复版"
}
```

| 字段 | 取值 | 含义 |
|---|---|---|
| `exit_mode` | `key`（默认）/ `auto` | 跑完按任意键退出 / N 秒自动关闭 |
| `auto_seconds` | 0–3600 | `auto` 时等待秒数 |
| `associate_ltir` | `true` / `false` | 是否双击 `.ltir` 直接运行 |

命令改（改完立即写回 JSON，关联同步生效）：

```
ltir006.exe config --show
ltir006.exe config --set exit=auto --set seconds=8
ltir006.exe config --set exit=key
ltir006.exe config --set associate=false
```

## 五、卸载

三种，任选：

```
双击 <安装目录>\uninstall.exe
ltir006.exe uninstall
ltir006_setup.exe /U           （或 uninstall.exe /S 静默）
```

卸载会移除 `.ltir` 文件关联（HKCU）、按清单删除已释放文件、`ltir006.json`、
`ltir006.exe`、`uninstall.exe`，并清掉空目录。正在运行的卸载程序自身若删不掉，
会登记「重启后删除」，如实提示，不假装完成。**安装根目录不删**，你放在里面的
`.ltir` 卷不会被误清。

## 六、实现说明

- 纯 Win32 API + C++17 标准库，零第三方依赖；注册表 API 动态装载
  `advapi32.dll`，所以交叉编译不需要 Windows SDK 导入库。
- 控制台设 `CP_UTF8`，中文解释与 `√ π ∞` 正常显示。
- Win10 起支持（用 `RegDeleteTreeA` / `MoveFileExA`，Vista+ API），未测 Win7/8，
  也不打算支持。
- **图标不做**（本令明确）。
- 文件关联只写 `HKCU\Software\Classes`，不动 `HKCR`，不需要提权。

## 七、验证状态（如实）

沙盒是 Linux，**`ltir006_setup.exe` 本体没在真 Windows 上跑过**。已做的取证：

- 两个内嵌 PE 均 `MZ` 头、PE32+ / x86_64（zig 0.17.1 交叉编译，0 error / 0 warning）
- 导入表只有 `KERNEL32.dll` + `api-ms-win-crt-*` —— **UCRT 是 Win10+ 自带组件**，
  不需要额外装 VC 运行库；`advapi32` 走动态装载，无静态依赖
- 自释放包按 `installer.cpp` 同款读法静态复算：尾标、payload 长度、
  central directory 条目数与走位、全部 `method=0`、内嵌 `ltir006.exe` 的 `MZ` 逐项通过
- Linux / Termux 侧同源源码回归 **69 / 69**（加 Windows 分支后重跑，0 error 0 warning）
- 内核门禁 `bash ltir006.sh c` → `OK: 11 文件与取证一致`，`aecc9726…` 未动

详细数据与过程报备见 `RUNRESULT_WIN.md`。
真机待回填：双击安装、双击 `.ltir`、`config --set`、卸载。有 bug 直接反馈。

---

# English summary

Windows build instructions for the frozen `win10+` branch — see the Chinese text above
for the legacy steps. The main line targets Linux + Termux; the Windows branch is
frozen as of 0.0.6 and not rebuilt (see BACKLOG.md). The legacy package lives in `win/`.
