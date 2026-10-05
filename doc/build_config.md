# build_config — 工程与构建配置

> 源文件：`云寸.sln` / `云寸.vcxproj` / `云寸.vcxproj.filters` / `云寸.vcxproj.user`　|　所属层：构建配置
> 职责概述：Visual Studio 解决方案与工程文件的配置说明（目标为 Windows 控制台程序）。

## 1. 概述

项目以 Visual Studio 解决方案（`云寸.sln`）组织，核心是工程文件 `云寸.vcxproj`。目标为 Windows 控制台程序（`SubSystem = Console`），语言标准 C++20（`Debug|x64` 为 C++23）。

## 2. 文件与来源

| 文件 | 说明 |
| --- | --- |
| `云寸.sln` | 解决方案文件 |
| `云寸.vcxproj` | 工程文件（构建配置、编译列表、预处理宏等） |
| `云寸.vcxproj.filters` | IDE 中文件分组（虚拟文件夹），不影响编译 |
| `云寸.vcxproj.user` | 个人调试配置，通常不应提交版本库 |

## 3. 关键定义

### 3.1 构建配置与预处理宏

`云寸.vcxproj` 定义 4 个配置：`Debug|x64`、`Release|x64`、`Debug|Win32`、`Release|Win32`（前两者为实际使用）。

| 配置 | PreprocessorDefinitions | LanguageStandard |
| --- | --- | --- |
| Debug \| Win32 | `WIN32;_DEBUG;_CONSOLE;CURL_STATICLIB;BUILDING_LIBCURL` | `stdcpp20` |
| Release \| Win32 | `WIN32;NDEBUG;_CONSOLE;CURL_STATICLIB;BUILDING_LIBCURL` | `stdcpp20` |
| Debug \| x64 | `_DEBUG;_CONSOLE;CURL_STATICLIB` | `stdcpp23` |
| Release \| x64 | `NDEBUG;_CONSOLE;CURL_STATICLIB` | `stdcpp20` |

- `SubSystem` 均为 `Console`。
- `CURL_STATICLIB` 表示静态链接 libcurl；`BUILDING_LIBCURL`（仅 Win32 配置）表示编译 curl 库本身。

### 3.2 编译单元与头文件

`ClCompile`：

```
colored_cout.cpp, pch.cpp, Tools.cpp, weather.cpp,
WeatherException.cpp, WebPage.cpp, 云寸.cpp
```

`ClInclude`：

```
colored_cout.h, pch.h, resource.h, resource1.h,
Tools.h, weather.h, WeatherException.h, WebPage.h
```

### 3.3 资源与其他

- `Image Include="Assets\icon256.ico"` —— 图标。
- `ResourceCompile Include="云寸.rc"` —— 资源脚本。

## 4. 主要接口

（配置文件，无代码接口。）

## 5. 工作流程 / 调用关系

VS 依据 `云寸.vcxproj` 的配置编译各编译单元、链接 libcurl 静态库、编译资源脚本，输出 `云寸.exe`。`.filters` 仅决定 IDE 显示分组，`.user` 保存本地调试设置。

## 6. 依赖关系

- `云寸.vcxproj` 依赖各源文件、`curl_include_lib`（libcurl 头/库）、`Assets` 资源。
- 被 `云寸.sln` 引用。

## 7. 注意事项 / 已知问题

- 编译选项提示：需在"项目属性 → C/C++ → 命令行 → 其他选项"填 `/utf-8`，使源码按 UTF-8 编译，配合 `SetConsoleOutputCP(CP_UTF8)` 解决中文乱码。
- `Tools_.cpp` **未包含**在编译列表中（遗留文件，见 `Tools_.md`）。
- `resource1.h` 被引用但仓库中**不存在**，属残留引用（详见 `known_issues.md` 第 9 条）。
- 建议：清理 `resource1.h` 残留引用；若不再需要 Win32 配置，可精简为 x64 两个配置。
