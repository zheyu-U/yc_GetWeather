# 云寸（yc_GetWeather）开发者文档

本目录存放项目内每个文件/模块的开发者文档，供后续维护、二次开发与 UI 版（yc_GetWeather_UI）复用参考。

## 文档索引

> 阅读建议：先看 [Overview.md](Overview.md) 快速入门，再按需查阅各分篇；改动代码前请先读 [known_issues.md](known_issues.md)。

| 文档 | 对应文件 | 说明 |
| --- | --- | --- |
| [Overview.md](Overview.md) | 全项目 | 开发者快速入门与总览 |
| [云寸.md](云寸.md) | `云寸.cpp` | 程序入口，流程编排与输出 |
| [weather.md](weather.md) | `weather.h` / `weather.cpp` | 核心业务：下载、解析、定位 |
| [WebPage.md](WebPage.md) | `WebPage.h` / `WebPage.cpp` | 网页/响应内容封装 |
| [Tools.md](Tools.md) | `Tools.h` / `Tools.cpp` | 工具函数与异步日志系统 |
| [Tools_.md](Tools_.md) | `Tools_.cpp` | 遗留旧版（未编译，含缺陷） |
| [WeatherException.md](WeatherException.md) | `WeatherException.h` / `.cpp` | 自定义异常体系 |
| [pch.md](pch.md) | `pch.h` / `pch.cpp` | 预编译头与平台开关 |
| [resource.md](resource.md) | `resource.h` / `云寸.rc` | 图标与版本资源 |
| [build_config.md](build_config.md) | `云寸.sln` / `.vcxproj` / `.filters` | 工程与构建配置 |
| [repo_docs.md](repo_docs.md) | `README.md` / `Release.md` / `.gitmodules` 等 | 仓库文档与子模块 |
| [third_party.md](third_party.md) | `curl/` / `colored-cout/` | 第三方库简述 |
| [Integration.md](Integration.md) | 全项目（面向使用者） | 集成与复用指南：把核心接入自己的工程 |
| [known_issues.md](known_issues.md) | 全项目 | 已知问题清单与后续开发方向 |

## 项目一句话概述

云寸是一个 Windows 控制台天气程序：通过当前 IP 定位城市，从中国天气网（weather.com.cn）抓取实时天气与预警信息，并用彩色文本输出到控制台。数据与日志存放在 `%AppData%\yc\`。

## 关键架构要点

- **双平台分支**：`_CONSOLE`（控制台，用 libcurl）与 `DeskTop_yc`（Windows App SDK / UWP，用 winrt HttpClient）两套网络实现，由预处理宏切换。
- **分层**：`云寸.cpp`（入口/UI） → `Weather`（业务） → `WebPage`（内容） → `Tools`（基础设施）。
- **自研解析器**：`Weather::ReadQuoteContent()` 手写字符串匹配，非标准 JSON 解析，格式脆弱。
- **异步日志**：生产者-消费者队列 + `std::jthread` + `condition_variable` 批量落盘。

## 文档规范

- **命名**：源文件类文档尽量贴合源文件名（`weather.md`、`WebPage.md`、`Tools.md`、`WeatherException.md` 等）；无法对应单一源文件的用描述名（`Overview.md`、`Integration.md`、`build_config.md`、`repo_docs.md`、`known_issues.md`）。
- **结构**：源文件类文档统一采用「标题 + 元信息（源文件 / 所属层 / 职责概述）」+「1.概述 / 2.文件与来源 / 3.关键定义 / 4.主要接口 / 5.工作流程·调用关系 / 6.依赖关系 / 7.注意事项」。
- **第三方库**：只作简短说明，集中在 [third_party.md](third_party.md)。
- **换行符**：本目录文档统一使用 CRLF + UTF-8(BOM)，编辑时请保持一致。
