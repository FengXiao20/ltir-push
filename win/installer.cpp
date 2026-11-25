// ltir006_setup.exe —— 0.0.6 强化修复版 · Win10+ 单文件安装器
//
// 形态：这一个 exe 里就带着「程序本身 + 完整目录」（尾部附加 stored zip）。
//   双击本 exe      → 交互安装：选盘 / 选目录 / 是否双击 .ltir / 退出方式 / 秒数
//   装完后目录里有  → ltir006.exe（程序）、uninstall.exe（卸载程序，本 stub 的副本）、
//                     ltir006.json（配置）、lab_codes.tsv、domains.tsv、core/、fixtures/、文档
//   双击 .ltir       → 文件关联 → ltir006.exe open 卷.ltir → 三态结果 → 按任意键 / N 秒自动关
//   命令            → ltir006.exe verify|eval|config|open|uninstall
//
// 纯 Win32 API + C++ 标准库，零第三方。注册表只写 HKCU，不需要管理员。
// 卸载逻辑只实现这一份：uninstall.exe 就是本 stub 的副本，靠自身文件名进入卸载模式。
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <windows.h>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>

static const char* MAGIC = "LTIRPAY!";   // 尾部 16 字节：magic(8) + payload 长度(8, LE)

static unsigned short rd16(const std::string& s, std::size_t p) {
  return (unsigned short)((unsigned char)s[p] | ((unsigned char)s[p+1] << 8));
}
static unsigned int rd32(const std::string& s, std::size_t p) {
  return (unsigned int)((unsigned char)s[p]) | ((unsigned int)(unsigned char)s[p+1] << 8)
       | ((unsigned int)(unsigned char)s[p+2] << 16) | ((unsigned int)(unsigned char)s[p+3] << 24);
}
static unsigned long long rd64(const std::string& s, std::size_t p) {
  unsigned long long v = 0;
  for (int i = 7; i >= 0; --i) v = (v << 8) | (unsigned char)s[p+i];
  return v;
}
static std::string selfPath() {
  char buf[MAX_PATH*4] = {0};
  const DWORD n = GetModuleFileNameA(NULL, buf, (DWORD)sizeof(buf)-1);
  return std::string(buf, n ? n : 0);
}
static std::string dirOf(const std::string& p) {
  const std::size_t q = p.find_last_of("\\/");
  return q == std::string::npos ? std::string(".") : p.substr(0,q);
}
static std::string baseOf(const std::string& p) {
  const std::size_t q = p.find_last_of("\\/");
  return q == std::string::npos ? p : p.substr(q+1);
}

/* 读自身尾部的 payload（stored zip） */
static bool readPayload(std::string& out) {
  const std::string self = selfPath();
  HANDLE h = CreateFileA(self.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE) return false;
  const DWORD sz = GetFileSize(h, NULL);
  if (sz < 32) { CloseHandle(h); return false; }
  char tail[16] = {0}; DWORD got = 0;
  SetFilePointer(h, (LONG)(sz-16), NULL, FILE_BEGIN);
  ReadFile(h, tail, 16, &got, NULL);
  if (got != 16 || memcmp(tail, MAGIC, 8) != 0) { CloseHandle(h); return false; }
  const unsigned long long len = rd64(std::string(tail,16), 8);
  if (len == 0 || len + 16 > sz) { CloseHandle(h); return false; }
  SetFilePointer(h, (LONG)(sz-16-(DWORD)len), NULL, FILE_BEGIN);
  out.resize((std::size_t)len);
  ReadFile(h, &out[0], (DWORD)len, &got, NULL);
  CloseHandle(h);
  return got == (DWORD)len;
}

/* 解析 stored zip：返回 (文件名, 内容) 列表。方法必须为 0（store），不依赖 deflate。 */
struct Entry { std::string name, data; };
static bool unzipStored(const std::string& p, std::vector<Entry>& out) {
  if (p.size() < 22) return false;
  long long eocd = -1;
  const long long lo = (long long)p.size() > 65557 ? (long long)p.size()-65557 : 0;
  for (long long i = (long long)p.size()-22; i >= 0 && i >= lo; --i) {
    if (p[(std::size_t)i]=='P' && p[(std::size_t)i+1]=='K' && p[(std::size_t)i+2]==5 && p[(std::size_t)i+3]==6) { eocd=i; break; }
  }
  if (eocd < 0) return false;
  const unsigned short n = rd16(p, (std::size_t)eocd+10);
  unsigned int pos = rd32(p, (std::size_t)eocd+16);
  for (unsigned short k = 0; k < n; ++k) {
    if (pos+46 > p.size() || rd32(p,pos) != 0x02014b50u) return false;
    const unsigned short method = rd16(p,pos+10);
    const unsigned int csize = rd32(p,pos+20);
    const unsigned short fnlen = rd16(p,pos+28), exlen = rd16(p,pos+30), cmlen = rd16(p,pos+32);
    const unsigned int lh = rd32(p,pos+42);
    if (method != 0) return false;                 // 只认 store，组装脚本用 ZIP_STORED
    if (lh+30 > p.size()) return false;
    const unsigned short lfn = rd16(p,lh+26), lex = rd16(p,lh+28);
    const std::size_t ds = (std::size_t)lh+30+lfn+lex;
    if (ds+csize > p.size()) return false;
    Entry e;
    e.name = p.substr(pos+46, fnlen);
    e.data = p.substr(ds, csize);
    out.push_back(e);
    pos += 46 + fnlen + exlen + cmlen;
  }
  return !out.empty();
}

static void mkDirs(const std::string& path) {
  std::string cur;
  for (std::size_t i = 0; i < path.size(); ++i) {
    const char ch = path[i];
    if (ch=='\\' || ch=='/') { if (!cur.empty()) CreateDirectoryA(cur.c_str(), NULL); cur.push_back('\\'); }
    else cur.push_back(ch);
  }
  if (!cur.empty()) CreateDirectoryA(cur.c_str(), NULL);
}
static bool writeFile(const std::string& path, const std::string& data) {
  mkDirs(dirOf(path));
  HANDLE h = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE) return false;
  DWORD w = 0;
  BOOL ok = WriteFile(h, data.data(), (DWORD)data.size(), &w, NULL);
  CloseHandle(h);
  return ok && w == data.size();
}
static std::string join(const std::string& dir, const std::string& name) {
  std::string r = dir + "\\";
  for (char ch : name) r.push_back(ch=='/' ? '\\' : ch);
  return r;
}

/* advapi32 动态装载：交叉编译时不需要 mingw 导入库，Win10+ 系统自带该 DLL */
static FARPROC regFn(const char* name) {
  static HMODULE h = NULL;
  if (!h) h = LoadLibraryA("advapi32.dll");
  return h ? GetProcAddress(h, name) : NULL;
}
typedef LONG (WINAPI *RegCreateKeyExA_t)(HKEY,LPCSTR,DWORD,LPSTR,DWORD,REGSAM,LPSECURITY_ATTRIBUTES,PHKEY,LPDWORD);
typedef LONG (WINAPI *RegSetValueExA_t)(HKEY,LPCSTR,DWORD,DWORD,const BYTE*,DWORD);
typedef LONG (WINAPI *RegCloseKey_t)(HKEY);
typedef LONG (WINAPI *RegDeleteTreeA_t)(HKEY,LPCSTR);
static LONG rCreate(HKEY a,LPCSTR b,DWORD c,LPSTR d,DWORD e,REGSAM f,LPSECURITY_ATTRIBUTES g,PHKEY h,LPDWORD i){
  RegCreateKeyExA_t p=(RegCreateKeyExA_t)regFn("RegCreateKeyExA");
  return p?p(a,b,c,d,e,f,g,h,i):1L;
}
static LONG rSet(HKEY a,LPCSTR b,DWORD c,DWORD d,const BYTE* e,DWORD f){
  RegSetValueExA_t p=(RegSetValueExA_t)regFn("RegSetValueExA");
  return p?p(a,b,c,d,e,f):1L;
}
static void rClose(HKEY a){ RegCloseKey_t p=(RegCloseKey_t)regFn("RegCloseKey"); if(p)p(a); }
static void rDelTree(HKEY a,LPCSTR b){ RegDeleteTreeA_t p=(RegDeleteTreeA_t)regFn("RegDeleteTreeA"); if(p)p(a,b); }

/* 文件关联：HKCU，Win10+ 免管理员 */
static bool assoc(bool on, const std::string& dir) {
  (void)dir;
  if (!on) {
    rDelTree(HKEY_CURRENT_USER, "Software\\Classes\\.ltir");
    rDelTree(HKEY_CURRENT_USER, "Software\\Classes\\LTIR.Document");
    return true;
  }
  const std::string exe = dir + "\\ltir006.exe";
  const std::string cmd = "\"" + exe + "\" open \"%1\"";
  HKEY k = NULL;
  if (rCreate(HKEY_CURRENT_USER,"Software\\Classes\\.ltir",0,NULL,0,KEY_WRITE,NULL,&k,NULL)!=ERROR_SUCCESS) return false;
  const char* prog="LTIR.Document";
  rSet(k,NULL,0,REG_SZ,(const BYTE*)prog,(DWORD)strlen(prog)+1); rClose(k); k=NULL;
  if (rCreate(HKEY_CURRENT_USER,"Software\\Classes\\LTIR.Document\\shell\\open\\command",0,NULL,0,KEY_WRITE,NULL,&k,NULL)!=ERROR_SUCCESS) return false;
  rSet(k,NULL,0,REG_SZ,(const BYTE*)cmd.c_str(),(DWORD)cmd.size()+1); rClose(k);
  return true;
}
static std::string jesc(const std::string& s) {
  std::string o; for (char ch:s) { if (ch=='\\'||ch=='"') o.push_back('\\'); o.push_back(ch); } return o;
}
static bool writeCfg(const std::string& dir, bool a, const std::string& exitMode, int sec) {
  char b[1024];
  snprintf(b,sizeof(b),
    "{\n"
    "  \"install_dir\": \"%s\",\n"
    "  \"associate_ltir\": %s,\n"
    "  \"exit_mode\": \"%s\",\n"
    "  \"auto_seconds\": %d,\n"
    "  \"version\": \"0.0.6 强化修复版\"\n"
    "}\n", jesc(dir).c_str(), a?"true":"false", exitMode.c_str(), sec);
  return writeFile(join(dir,"ltir006.json"), std::string(b));
}

static std::string ask(const char* prompt, const std::string& def) {
  printf("%s [%s]: ", prompt, def.c_str());
  char buf[512]={0};
  if (!fgets(buf,sizeof(buf),stdin)) return def;
  std::string s(buf);
  while(!s.empty() && (s.back()=='\n'||s.back()=='\r'||s.back()==' ')) s.pop_back();
  return s.empty()?def:s;
}

/* ── 卸载模式：uninstall.exe（本 stub 的副本）从这里进 ── */
static int doUninstall(bool silent) {
  const std::string self = selfPath(), dir = dirOf(self);
  printf("LTIR 0.0.6 强化修复版 · 卸载\n");
  printf("目录: %s\n\n", dir.c_str());
  assoc(false, dir);
  printf("  [ok] 已移除 .ltir 文件关联（HKCU）\n");
  std::string payload; std::vector<Entry> es;
  int removed = 0;
  if (readPayload(payload) && unzipStored(payload, es)) {
    for (const Entry& e : es) {
      const std::string p = join(dir, e.name);
      if (DeleteFileA(p.c_str())) { ++removed; printf("  [ok] %s\n", e.name.c_str()); }
    }
  }
  const char* extra[] = {"ltir006.json","uninstall.exe","ltir006.exe",NULL};
  for (int i=0; extra[i]; ++i) { const std::string p=join(dir,extra[i]);
    if (DeleteFileA(p.c_str())) { ++removed; printf("  [ok] %s\n", extra[i]); } }
  // 逆序删空目录（不含安装根，保留用户可能放进去的 .ltir 卷）
  const char* dirs[] = {"core\\include\\ltir","core\\include","core\\src","core","fixtures","win",NULL};
  for (int i=0; dirs[i]; ++i) RemoveDirectoryA(join(dir,dirs[i]).c_str());
  // 运行中的 exe 通常删不掉：登记重启后删除，失败就如实提示
  if (!MoveFileExA(self.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
    printf("\n[i] uninstall.exe 正在运行，请重启后手动删除它（其余已清理）。\n");
  printf("\n卸载完成：共清理 %d 个文件。配置与关联已移除。\n", removed);
  if (!silent) { printf("\n按任意键退出...\n"); (void)getchar(); }
  return 0;
}

int main(int argc, char** argv) {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  // 卸载程序判定：自身文件名含 uninstall，或显式 --uninstall
  std::string bn = baseOf(selfPath());
  for (char& ch : bn) ch = (char)tolower((unsigned char)ch);
  bool isUninstall = bn.find("uninstall") != std::string::npos;
  bool silent = false, explicitUninstall = false;
  for (int i=1;i<argc;++i) {
    std::string a=argv[i];
    if (a=="--silent"||a=="/S") silent=true;
    if (a=="--uninstall"||a=="/U") explicitUninstall=true;
  }
  if (isUninstall || explicitUninstall) return doUninstall(silent);

  printf("================================================\n");
  printf(" LTIR 0.0.6 强化修复版 · Win10+ 安装程序\n");
  printf(" 单文件自释放：本 exe 内已带程序与完整目录\n");
  printf("================================================\n\n");

  // 静默参数：/S /D=路径 /ASSOC=0|1 /EXIT=key|auto /SEC=秒
  std::string dest="", exitMode="key"; int sec=5; bool doAssoc=true;
  for (int i=1;i<argc;++i) {
    std::string a=argv[i];
    if (a.rfind("/D=",0)==0) dest=a.substr(3);
    else if (a.rfind("/ASSOC=",0)==0) doAssoc=(a.substr(7)=="1");
    else if (a.rfind("/EXIT=",0)==0) exitMode=(a.substr(6)=="auto"?"auto":"key");
    else if (a.rfind("/SEC=",0)==0) sec=atoi(a.substr(5).c_str());
  }
  if (silent) { if (dest.empty()) dest="C:\\LTIR"; }
  else {
    const std::string drv = ask("安装盘符（只填字母）", "C");
    const std::string defDir = std::string(1, drv.empty()?'C':drv[0]) + ":\\LTIR";
    dest = ask("安装目录", defDir);
    const std::string a = ask("是否双击 .ltir 直接运行？[Y/n]", "Y");
    doAssoc = !(a=="n"||a=="N"||a=="no"||a=="0");
    const std::string m = ask("运行结束后：1=按任意键退出（默认） 2=N 秒自动关闭", "1");
    exitMode = (m=="2"||m=="auto") ? "auto" : "key";
    if (exitMode=="auto") { const std::string s=ask("自动关闭秒数","5");
      int n=atoi(s.c_str()); if(n>=0&&n<=3600) sec=n; }
  }
  if (sec<0||sec>3600) sec=5;

  std::string payload; std::vector<Entry> es;
  if (!readPayload(payload) || !unzipStored(payload, es)) {
    printf("\n[E-EXT] 安装包尾部数据损坏，无法释放。请重新下载 ltir006_setup.exe。\n");
    if (!silent) { printf("\n按任意键退出...\n"); (void)getchar(); }
    return 1;
  }
  printf("\n正在释放到 %s ...\n", dest.c_str());
  mkDirs(dest);
  int n=0; bool all=true;
  for (const Entry& e : es) {
    if (writeFile(join(dest,e.name), e.data)) { ++n; printf("  [ok] %s\n", e.name.c_str()); }
    else { all=false; printf("  [!!] 写入失败: %s\n", e.name.c_str()); }
  }
  if (!all) { printf("\n[E-EXT] 部分文件写入失败（可能是权限或路径问题）。\n");
              if (!silent) { printf("\n按任意键退出...\n"); (void)getchar(); } return 1; }
  if (!writeCfg(dest, doAssoc, exitMode, sec)) printf("  [!!] ltir006.json 写入失败\n");
  if (doAssoc) { if (assoc(true,dest)) printf("  [ok] 已关联 .ltir（HKCU，双击即可运行）\n");
                 else printf("  [!!] 文件关联写入失败，可用 ltir006.exe config --set associate=true 重试\n"); }
  // 卸载程序 = 本 stub 的副本（尾部 payload 一起带过去，卸载时靠它列清单）
  if (!CopyFileA(selfPath().c_str(), join(dest,"uninstall.exe").c_str(), FALSE))
    printf("  [!!] uninstall.exe 复制失败，可用 ltir006.exe uninstall 命令代替\n");
  else printf("  [ok] uninstall.exe（卸载程序）\n");

  printf("\n------------------------------------------------\n");
  printf(" 安装完成：%d 个文件 → %s\n", n, dest.c_str());
  printf(" 命令执行：%s\\ltir006.exe verify 卷.ltir\n", dest.c_str());
  printf(" 改配置　：ltir006.exe config --show | --set exit=auto --set seconds=5\n");
  printf(" 卸载　　：双击 uninstall.exe，或 ltir006.exe uninstall\n");
  const std::string exitDesc = (exitMode=="auto")
      ? (std::string("（")+std::to_string(sec)+" 秒后自动关闭）")
      : std::string("（按任意键退出，默认）");
  printf(" 退出方式：%s%s\n", exitMode.c_str(), exitDesc.c_str());
  printf("------------------------------------------------\n");
  if (!silent) { printf("\n按任意键退出...\n"); (void)getchar(); }
  return 0;
}
