# Overview — 开发者快速入门与总览

> 适用对象：首次接触本项目的开发者
> 作用：在阅读各分篇文档之前，先建立对项目整体结构、运行流程与文档体系的完整认识。

## 1. 项目一句话概述

**云寸（yc_GetWeather）** 是一个 Windows 控制台天气程序：通过当前 IP 定位城市，从中国天气网（weather.com.cn）抓取实时天气与预警信息，并用彩色文本输出到控制台。数据与日志存放在 `%AppData%\yc\`。

- 本仓库同时是 UI 版 `yc_GetWeather_UI` 复用的**公共核心库**（`云寸.cpp` 除外）。
- 支持的平台分支：`_CONSOLE`（控制台，libcurl）与 `DeskTop_yc`（Windows App SDK / UWP，winrt HttpClient）。

## 2. 分层架构

自顶向下分为四层，依赖关系**单向**（上层可调下层，下层不反向依赖）：

```
云寸.cpp      入口 / UI / 流程编排（唯一不可复用）
   ↓
Weather       核心业务：拼 URL、下载、解析、定位状态
   ↓
WebPage       内容封装：一次 HTTP 响应的正文与元数据
   ↓
Tools         基础设施：数据目录、读文件、取时间、异步日志
```

| 层 | 职责 | 可复用 |
| --- | --- | --- |
| `云寸.cpp` | 入口、控制台 UI、流程编排 | ❌ 含 `main()`，属“应用” |
| `Weather` | 三套 URL 的下载与解析、位置状态 | ✅ |
| `WebPage` | 承载一次响应的正文与元数据 | ✅ |
| `Tools` | 数据目录、文件读取、时间、异步日志 | ✅ |

两条硬规则：**依赖单向**（`Tools.h` 顶部明确注明“不得包含 `WebPage.h`”，以防循环包含）；**平台差异由宏隔离**（`_CONSOLE` / `DeskTop_yc` 只出现在实现层，不外泄到接口）。

> 想把 `Weather` 及以下各层接入自己的工程（例如 UI 版 `yc_GetWeather_UI`），做法、初始化顺序与踩坑清单见 [Integration.md](Integration.md)。

横切关注点：

- **异常体系** `ycresult`（`WeatherException.h`）—— 统一错误表达与处理策略；
- **彩色输出** `colored_cout`（第三方，`third_party.md`）；
- **平台开关** `pch.h`（`_CONSOLE` / `DeskTop_yc`）。

## 3. 一次运行的完整数据流

```
main()
 ├─ 初始化：控制台 UTF-8、curl_global_init、创建 %AppData%\yc、启动日志系统
 ├─ new Weather(Type::instant)
 │
 ├─ get_weather_kit(location)      // IP 定位
 │     get_weather → get_CURL → wgeo.weather.com.cn/ip/
 │     analysis → 解析 ip/id/addr → Position::setPosition(城市名, 城市代码)
 │
 ├─ get_weather_kit(instant)       // 实时天气（依赖上一步的城市代码）
 │     get_weather → get_CURL → d1.weather.com.cn/sk_2d/<code>.html
 │     analysis → 解析 cityname/temp/WD/WS/weather/aqi/time/date
 │
 ├─ get_weather_kit(warnings)      // 天气预警（同样是 <code>）
 │     get_weather → get_CURL → d1.weather.com.cn/dingzhi/<code>.html
 │     analysis → 解析 "w" / w9
 │
 └─ 打印彩色结果（AQI 分级上色）→ pause → ShutdownLogSystem
```

关键实现细节（如 URL 模板、解析器原理、日志系统机制）见各分篇文档。

## 4. 目录结构（主要文件）

| 文件 | 说明 | 文档 |
| --- | --- | --- |
| `云寸.cpp` | 程序入口 | [云寸.md](云寸.md) |
| `weather.h/.cpp` | 核心业务类 | [weather.md](weather.md) |
| `WebPage.h/.cpp` | 内容封装 | [WebPage.md](WebPage.md) |
| `Tools.h/.cpp` | 工具与异步日志 | [Tools.md](Tools.md) |
| `Tools_.cpp` | 遗留旧版（不编译） | [Tools_.md](Tools_.md) |
| `WeatherException.h/.cpp` | 异常体系 | [WeatherException.md](WeatherException.md) |
| `pch.h/.cpp` | 预编译头 / 平台开关 | [pch.md](pch.md) |
| `colored_cout.h/.cpp` | 彩色输出（第三方精简版） | [third_party.md](third_party.md) |
| `resource.h` / `云寸.rc` | 图标与版本资源 | [resource.md](resource.md) |
| `云寸.sln` / `云寸.vcxproj` | 工程与构建配置 | [build_config.md](build_config.md) |
| `curl/`、`colored-cout/` | 第三方子模块 | [third_party.md](third_party.md) |
| `README.md` / `Release.md` / `LICENSE` | 仓库级文档 | [repo_docs.md](repo_docs.md) |

## 5. 编译与运行要点

- IDE：Visual Studio（工程文件 `云寸.sln`），目标为 Windows 控制台程序，C++20。
- 编译选项需填 `/utf-8`（属性 → C/C++ → 命令行 → 其他选项），配合 `SetConsoleOutputCP(CP_UTF8)` 解决中文乱码。
- 依赖 libcurl（静态链接，宏 `CURL_STATICLIB`）；子模块需先 `git submodule update --init`。
- 数据与日志目录：`%AppData%\yc\`（历史天气缓存 `*.txt`、日志 `log\yc_log_<时间>.log`）。
- 详细配置见 [build_config.md](build_config.md)。

## 6. 学习路径（建议阅读顺序）

1. [云寸.md](云寸.md) —— 先看清入口的编排与顺序；
2. [weather.md](weather.md) —— 核心业务与解析逻辑；
3. [WebPage.md](WebPage.md) —— 内容如何在文件/内存与解析之间流转；
4. [Tools.md](Tools.md) —— 基础设施与异步日志；
5. [WeatherException.md](WeatherException.md) —— 错误处理约定；
6. [pch.md](pch.md) / [build_config.md](build_config.md) —— 平台开关与工程配置；
7. [third_party.md](third_party.md) —— 第三方库简述；
8. [known_issues.md](known_issues.md) —— 已知风险与后续开发方向（**改代码前必读**）；
9. [Integration.md](Integration.md) —— 把核心接入你自己的工程时的做法、初始化顺序与踩坑清单。

## 7. 阅读与维护约定

- 本目录文档采用统一结构：`标题 + 元信息`，随后为 `1.概述 / 2.文件与来源 / 3.关键定义 / 4.主要接口 / 5.工作流程·调用关系 / 6.依赖关系 / 7.注意事项`。
- 第三方库（curl、colored-cout）只作简短说明，见 [third_party.md](third_party.md)。
- 文档命名尽量贴合源文件名：`weather.md`、`WebPage.md`、`Tools.md` 等；无法对应单一源文件的用英文描述名（`build_config.md`、`repo_docs.md`、`known_issues.md`）。
- 修改代码后，请同步更新对应分篇；引入或修复问题时，请更新 [known_issues.md](known_issues.md)。
