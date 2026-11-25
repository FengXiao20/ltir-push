// ltir/src/cli.hpp —— CLI 层唯一对外接口（非内核头文件）
#pragma once
namespace ltir {
// 返回 exit code：0=通过 / 1=验证失败 / 2=格式错误（宪法 4）
int runCli(int argc, char** argv) noexcept;
}
