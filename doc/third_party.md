# third_party — 第三方库简述

> 所属层：第三方依赖　|　对应：`curl/`、`colored-cout/` 子模块与根目录 `colored_cout.h/.cpp`
> 职责概述：简要说明项目使用的第三方库，细节以各库官方文档为准。

## 1. 概述

本项目依赖两个第三方库：**libcurl**（网络请求）与 **colored-cout**（彩色控制台输出）。两者均以 Git 子模块形式引入（见 `.gitmodules`）。本页只作简要说明。

## 2. 库一览

| 库 | 仓库 | 许可 | 在本项目中的用途 |
| --- | --- | --- | --- |
| libcurl | https://github.com/curl/curl | curl 许可（类 MIT） | `_CONSOLE` 分支下载天气数据（`Weather::get_CURL`） |
| colored-cout | https://github.com/yurablok/colored-cout | Unlicense（公共域） | 控制台彩色输出（`clr::` 颜色 + `operator<<`） |

## 3. 接入方式

### 3.1 libcurl

- 子模块目录：`curl/`（完整源码）。
- 头文件目录：`curl_include_lib/`。
- 工程以静态库方式链接：预处理宏 `CURL_STATICLIB`（Win32 配置另有 `BUILDING_LIBCURL`）。
- 使用点：`weather.cpp` 的 `get_CURL()`。
- 初始化：`云寸.cpp` 中 `curl_global_init(CURL_GLOBAL_ALL)`。

### 3.2 colored-cout

- 上游子模块：`colored-cout/`（完整版，含自身 README、UNLICENSE、示例图）。
- 项目实际使用：根目录的**精简定制版** `colored_cout.h` / `colored_cout.cpp`。
- 用法：`std::cout << clr::red << "红字" << clr::green << "绿字" << clr::reset << std::endl;`
- 颜色枚举 `clr`（Windows 用 `uint16_t`）：前景 `grey/blue/green/cyan/red/magenta/yellow/white`，背景 `on_*` 系列，`reset = 0xFF`。
- 前景与背景为**独立状态**（设置前景不影响已设背景）；非 `_WIN32`/`__unix__`/`__APPLE__` 平台会 `#error unsupported`。

## 4. 注意事项

- 根目录精简版 `colored_cout.*` 与子模块完整版 `colored-cout/colored_cout.h` **内容不同**，维护时勿混淆。
- 颜色状态基于 `static` 局部变量与控制台属性，`std::cout` / `std::cerr` 混用可能互相影响。
- 本项目只支持 Windows 控制台，实际走 Windows 分支。
- 相关引用关系另见 `repo_docs.md`（子模块声明）。
