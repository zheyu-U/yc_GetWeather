# weather — 核心业务类

> 源文件：`weather.h` / `weather.cpp`　|　所属层：业务
> 职责概述：程序核心。保存全部天气字段，构造请求 URL 并下载数据，用自研解析器提取字段，并在步骤之间传递定位结果。

## 1. 概述

`yc::Weather` 是整个程序的核心类，负责四件事：

1. 保存天气数据的全部字段；
2. 构造请求 URL，并通过 libcurl / HttpClient 下载数据；
3. 用自研的 `ReadQuoteContent()` 解析下载到的文本；
4. 通过 `namespace Position` 在多次调用之间传递定位结果（城市名与城市代码）。

`weather.cpp` 顶部 `#include "pch.h"`，并使用 `using namespace Tools;`。

## 2. 文件与来源

| 项 | 内容 |
| --- | --- |
| 源文件 | `weather.h`（66 行）/ `weather.cpp`（约 503 行） |
| 是否参与编译 | 是（`ClCompile` / `ClInclude`） |
| 是否第三方 | 否 |
| 目标平台 | `_CONSOLE`（libcurl）与 `DeskTop_yc`（winrt HttpClient）双分支 |
| 关键依赖 | `WebPage.h`、`Tools.h`、`WeatherException.h`、`curl/curl.h`（`_CONSOLE`） |

## 3. 关键定义

### 3.1 成员变量

| 成员 | 说明 |
| --- | --- |
| `Type contentType` | 当前内容类型：`instant` / `forecast` / `location` / `warnings` |
| `Location` / `LocationCode` | 城市名 / 城市代码（代码用于拼接 URL） |
| `PublicIP` | 本机公网 IP（定位用） |
| `CurrentTem` / `MaxTem` / `MinTem` | 实时 / 最高 / 最低温度 |
| `UpdateTime` / `CurrentDate` / `CurrentWeekday` | 更新时间 / 日期 / 星期 |
| `CurrentAQI` | 空气质量指数 |
| `CurrentWind_Direction` / `CurrentWind_speed` | 风向 / 风速 |
| `CurrentWeather` | 天气现象（如"晴"） |
| `CurrentRainfall` / `Rainfall24` | 当前 / 24 小时降雨量 |
| `Warnings` | 预警文本 |
| `std::vector<std::string> Headers` | HTTP 请求头，成对存储（键、值） |
| `std::string Time`（私有） | 构造时刻的毫秒时间戳，用于 URL 的 `?_=` 防缓存参数 |

### 3.2 构造函数（三个重载）

均会调用 `setHeaders()` 并通过 `Tools::String_GetCurrentTime()` 记录 `Time`：

- `Weather()` —— 类型未定；
- `Weather(Type ct)` —— 指定类型；
- `Weather(Type ct, std::string location, std::string locationCode)` —— 指定类型 + 位置。

### 3.3 `namespace Position`

用命名空间内 `static` 变量在多次 `get_weather_kit` 调用之间传递定位结果：

```cpp
static std::string location = "ERR", code = "ERR";
setPosition(m_location, m_code);              // 写入
getPosition(); getPositionCode();             // 读取
```

## 4. 主要接口

| 接口 | 可见性 | 说明 |
| --- | --- | --- |
| `void analysis(WebPage::WebPage* page)` | public | 解析入口，按 `Type` 提取字段；失败抛 `ycresult` |
| `bool get_CURL(std::string url, std::string fileName)` | public | `_CONSOLE` 分支：libcurl 下载并写入本地文件 |
| `bool get_HttpClient(std::string url, std::string* res)` | public | `DeskTop_yc` 分支：winrt HttpClient 下载到内存 |
| `bool get_weather(Type type, WebPage::WebPage* page)` | public | 按 `Type` 分派请求，成功后把数据装入 `WebPage` |
| `bool get_weather_kit(Type type)` | public | "下载 + 解析"一条龙，对外的简易接口 |
| `void setupTime()` | public | 重新刷新 `Time`（内联） |
| `void setHeaders()` | private | 构造时自动设置请求头 |
| `bool ReadQuoteContent(std::string* FromWhat, std::string What, std::string* ToWhat)` | private | 自研字段提取器 |

### 4.1 `setHeaders()`

向 `Headers` 依次压入（每两条一组：键、值）：

`Accept: */*`、`User-Agent: yc_GetWeather_get/<版本>`、`Referer: http://www.Weather.com.cn/`、`Connection: keep-alive`、`Accept-Language: zh-CN,zh;q=0.9,...`。

### 4.2 `get_CURL(url, fileName)`

`_CONSOLE` 分支的下载实现：

1. 打开 `%AppData%\yc\<fileName>` 用于写入；
2. 遍历 `Headers`（步长 2）构建 `curl_slist`；
3. `curl_easy_setopt` 设置 URL、请求头、写入目标（`CURLOPT_WRITEDATA` 指向 `FILE*`，使用默认写回调）；
4. `curl_easy_perform()` 执行；
5. 成功后读取本机 IP / 目标 IP 写日志，清理资源并返回 `TRUE`；
6. 失败则写警告日志，**不抛异常**，返回 `false`。

> 源码注释明确说明：有返回值的函数不应抛异常，以避免内存泄漏与返回值不确定。

### 4.3 `get_weather(Type, WebPage*)`

按 `Type` 分派三种请求，URL 均带 `?_=<Time>` 防缓存：

| Type | URL 模板 | 用途 |
| --- | --- | --- |
| `location` | `http://wgeo.weather.com.cn/ip/?_=<Time>` | 用当前 IP 定位 |
| `instant` | `http://d1.weather.com.cn/sk_2d/<城市代码>.html?_=<Time>` | 实时天气 |
| `warnings` | `http://d1.weather.com.cn/dingzhi/<城市代码>.html?_=<Time>` | 天气预警 |

下载成功后：`_CONSOLE` 分支调 `page->read(...)`（从本地文件读入），`DeskTop_yc` 分支调 `page->make(...)`（直接从内存构造）。

### 4.4 `get_weather_kit(Type)`

把"下载 + 解析"打包成一步：`new WebPage` → `get_weather` → 成功则 `analysis` → `delete pg`。`analysis` 抛出的异常在此被捕获、记日志并返回 `false`。

### 4.5 `analysis(WebPage*)`

解析入口。先校验内容首字符是否为 `'v'`（天气网返回的 JS 变量格式），否则抛 `invalid_formatting`。随后按 `Type` 提取：

- **instant**：依次提取 `cityname`、`temp"`、`WD`、`WS`、`weather"`、`aqi"`、`time"`、`date"`，任一缺失抛 `content_not_found`；风速为 `"0级"` 时把风向改为"无风"。
- **location**：提取 `ip`、`id`、`addr`，并调用 `Position::setPosition()` 保存城市名与代码供后续使用。
- **warnings**：提取 `"w"`；为空表示无预警，否则再提取 `w9` 作为预警文本。

### 4.6 `ReadQuoteContent(FromWhat, What, ToWhat)`（私有，自研解析器）

原理是**纯字符串匹配**：在 `FromWhat` 中 `find(键名 What)` → 找 `:` 或 `=` → 按值的起始字符（`"`、`{`、`[`）找到对应结束符（`"`、`}`、`]`）→ 截取中间内容写入 `ToWhat`。

## 5. 工作流程 / 调用关系

```
云寸.cpp::main()
  └─ Weather::get_weather_kit(Type::location)
		├─ get_weather(Type::location, page)      // 下载，写入 Location.txt
		│     └─ get_CURL(...)  → wgeo.weather.com.cn/ip/
		└─ analysis(page)                          // 解析 ip/id/addr
			  └─ Position::setPosition(location, code)
  └─ Weather::get_weather_kit(Type::instant)
		├─ get_weather(Type::instant, page)        // 下载
		│     └─ get_CURL(...)  → d1.weather.com.cn/sk_2d/<code>.html
		└─ analysis(page)                          // 解析 cityname/temp/WD/WS/...
  └─ Weather::get_weather_kit(Type::warnings)
		├─ get_weather(Type::warnings, page)       // 下载
		│     └─ get_CURL(...)  → d1.weather.com.cn/dingzhi/<code>.html
		└─ analysis(page)                          // 解析 "w" / w9
```

`get_weather` 内部的下载与 `WebPage` 接口对应关系：

| 分支 | 数据落点 | 调用的 WebPage 接口 |
| --- | --- | --- |
| `_CONSOLE` | 本地文件 `%AppData%\yc\<fileName>` | `page->read(...)` |
| `DeskTop_yc` | 内存字符串 | `page->make(...)` |

## 6. 依赖关系

- **依赖**：`WebPage`（内容封装）、`Tools`（日志/路径/异常）、`WeatherException`（`ycresult`）、libcurl（`_CONSOLE`）或 winrt HttpClient（`DeskTop_yc`）。
- **被依赖**：`云寸.cpp`（入口）。

## 7. 注意事项 / 已知问题

- **解析脆弱**：`ReadQuoteContent` 靠数引号匹配，非 JSON 解析；网页格式变化会导致解析失败或取错值（详见 `known_issues.md` 第 1 条）。
- **内存泄漏**：`get_weather_kit` 在 `analysis` 抛异常的路径上未 `delete pg`；`get_weather` 中 `new std::string` 的 `c_fileName` / `w_fileName` 从未释放（详见 `known_issues.md` 第 3、4 条）。
- **全局状态**：`Position` 使用静态可变全局变量，当前单线程安全，作为库/并发复用时存在隐患（详见 `known_issues.md` 第 6 条）。
- `Type::forecast` 已声明但未实现对应 URL 与解析（详见 `known_issues.md` 第 10 条）。
- `get_HttpClient` 在函数体内 `#include <winrt/...>`，属非常规写法（详见 `known_issues.md` 第 11 条）。
