# pch — 预编译头与平台开关

> 源文件：`pch.h` / `pch.cpp`　|　所属层：构建基础设施
> 职责概述：预编译头，加速编译；同时是"控制台 / 桌面"双平台宏开关的核心位置。

## 1. 概述

预编译头（PCH）用于加速编译。`pch.cpp` 仅 `#include "pch.h"`，是生成 PCH 的唯一编译单元；所有其它 `.cpp` 首行均 `#include "pch.h"`。`pch.h` 还是**平台开关的核心位置**。

## 2. 文件与来源

| 项 | 内容 |
| --- | --- |
| 源文件 | `pch.h`（12 行）/ `pch.cpp`（1 行） |
| 是否参与编译 | 是（`pch.cpp` 在 `ClCompile` 列表） |
| 是否第三方 | 否 |

## 3. 关键定义

`pch.h` 全文：

```cpp
#pragma once
#include <memory>

#ifdef DeskTop_yc
#include <winrt/Windows.Web.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Web.Syndication.h>
#endif // DeskTop_yc

//适配 yc_GetWeather_UI
```

### 平台宏对照

| 宏 | 含义 | 网络实现 |
| --- | --- | --- |
| `_CONSOLE` | 控制台程序 | libcurl（`Weather::get_CURL`） |
| `DeskTop_yc` | Windows App SDK / UWP（UI 版） | winrt HttpClient（`Weather::get_HttpClient`） |

## 4. 主要接口

（无函数接口，仅头文件与宏开关。）

## 5. 工作流程 / 调用关系

`pch.h` 在 `DeskTop_yc` 下引入 winrt 头，配合 `weather.cpp`、`Tools.cpp` 等处 `#ifdef DeskTop_yc` 分支，实现"同一份核心代码支持两种前端"。`pch.cpp` 作为唯一 PCH 编译单元被编译，其余 `.cpp` 复用编译好的 PCH。

## 6. 依赖关系

- **被依赖**：所有 `.cpp`（`#include "pch.h"`）。
- **依赖**：`DeskTop_yc` 分支下依赖 winrt 头文件。

## 7. 注意事项 / 已知问题

- `云寸.vcxproj` 当前配置只定义 `_CONSOLE`（见 `build_config.md`），因此 `DeskTop_yc` 分支不会被编译，仅在 UI 项目（`yc_GetWeather_UI`）中启用。
- 注释"适配 yc_GetWeather_UI"表明本仓库是 UI 版复用的公共核心库。
