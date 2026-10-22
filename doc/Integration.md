# Integration — 集成与复用指南

> **所属层**：跨层（面向使用者的接入说明） 

> **职责概述**：说明如何把本项目接入你自己的工程。包括环境准备、文件拷贝、工程配置、生命周期调用顺序、最小示例、平台分支与踩坑清单。  

> **适用对象**：想把天气能力嵌入自己程序（例如 UI 版 `yc_GetWeather_UI`）的开发者。

## 1. 源码级复用


本项目的复用方式是 **源码级复用（source-level reuse）**。

因此"使用这个库"的实际做法固定为五步：

1. 把源文件（`weather.*`、`WebPage.*`、`Tools.*`、`WeatherException.*`、`pch.*`）拷进你自己的工程，不包括 `云寸.cpp`；
2. 在你的工程里定义平台宏（`_CONSOLE` 或 `DeskTop_yc`）；
3. 若要使用curl，自行准备头文件路径并链接 libcurl，定义`_CONSOLE` 宏；
4. 按约定顺序调用初始化 / 收尾函数（**责任在使用者，不在库内部**）；
5. `#include "weather.h"`，使用 `yc::Weather`。

> 若将来要升级为"真正的库"（`.lib` + 导出宏，或 CMake 的 `install`/`export` 目标），现有代码需要改动：为公开符号加导出宏、拆分"实现"与"接口"、把全局状态改为可注入对象。取舍见 [known_issues.md](known_issues.md)。

## 2. API 边界

### 2.1 公开头文件与主要符号

四个头文件构成事实上的 API 边界，其余 `.cpp` 都是实现：

| 头文件 | 命名空间 | 主要符号 |
| --- | --- | --- |
| `weather.h` | `yc` | `Weather` 类；并连带引入 `WebPage.h`、`Tools.h`、`WeatherException.h` |
| `WebPage.h` | `yc`、`yc::WebPage` | `Type`（`instant/forecast/location/warnings`）、`WebPage::DocType`、`WebPage::Encoding`、`WebPage` 类 |
| `Tools.h` | `Tools` | `CheckIfDataFileExists()`、`InitLogSystem()`、`ShutdownLogSystem()`、`log_write()`、`Get_DataLocation()`、`String_GetCurrentTime()`、`readFileIntoString()`；日志级别常量 `Err/Warn/Info/FErr`；`Detail` / `InfoSet` |
| `WeatherException.h` | `yc::ycresult` | `ExcptType`、`AddressMethod`、`ycresult` 类 |

### 2.2 `yc::Weather` 的公开成员

`Weather` 的所有数据成员都是 **public**，集成者直接读字段，不需要 getter：

```cpp
// 用于构造的配置
Type contentType;
std::string Location, LocationCode, PublicIP;

// 实时天气结果
std::string CurrentTem, MaxTem, MinTem, UpdateTime, CurrentDate, CurrentWeekday;
std::string CurrentAQI, CurrentWind_Direction, CurrentWind_speed;
std::string CurrentWeather, CurrentRainfall, Rainfall24, Warnings;

// 请求头（key/value 成对存放）
std::vector<std::string> Headers;
```

公开方法：

| 方法 | 作用 | 备注 |
| --- | --- | --- |
| `Weather()` /  `Weather(Type)` / `Weather(Type, location, locationCode)` | 三个构造函数 | 构造时自动 `setHeaders()` 并记录时间戳 |
| `bool get_weather_kit(Type)` | **推荐入口**：下载 + 解析一步到位 | 内部吞掉解析异常，仅以返回值表达成败；**不向外抛 `ycresult`** |
| `bool get_weather(Type, WebPage*)` | 只下载并生成 `WebPage` | 需自行管理 `WebPage*` 生命周期 |
| `void analysis(WebPage*)` | 只解析已有 `WebPage` | **会抛 `ycresult`**，需自行 catch |
| `bool get_CURL(url, fileName)` | `_CONSOLE` 形态下载 | 失败返回 `false`，按设计**不抛异常** |
| `bool get_HttpClient(url, std::string*)` | `DeskTop_yc` 形态下载 | 同上 |
| `void setupTime()` | 刷新内部时间戳 | 用于重设 `?_=<Time>` 防缓存参数 |

### 2.3 可复用边界

| 文件 | 可否复用 | 原因 |
| --- | --- | --- |
| `weather.h` / `weather.cpp` | ✅ | 核心业务，无 UI 依赖 |
| `WebPage.h` / `WebPage.cpp` | ✅ | 纯数据结构与文件读取 |
| `Tools.h` / `Tools.cpp` | ✅ | 基础设施；注意 `Tools.h` 顶部的硬性警告"不得包含 `WebPage.h`" |
| `WeatherException.h` / `.cpp` | ✅ | 仅依赖标准库 |
| `pch.h` / `pch.cpp` | ✅ | 内容极简，见 3.2 |
| `colored_cout.h` / `.cpp` | ⚠️ 视情况 | 仅控制台需要；GUI 形态通常不需要 |
| `云寸.cpp` | ❌ | 含 `main()`、控制台 UI、`curl_global_init`、`stoi` 上色逻辑，是"应用"而非"库" |
| `Tools_.cpp` | ❌ | 遗留旧版，未参与编译 |

## 3. 集成步骤

### 3.1 步骤一：准备 libcurl

本仓库自带**预编译静态版 libcurl**，无需自行编译：

```
curl_include_lib\
  ├─ libcurl-vc17-x64-debug-static-ipv6-sspi-schannel\
  │    ├─ include\curl\*.h
  │    └─ lib\libcurl_a_debug.lib
  ├─ libcurl-vc17-x64-release-static-ipv6-sspi-schannel\
  │    ├─ include\curl\*.h
  │    └─ lib\libcurl_a.lib
  ├─ libcurl-vc17-x86-debug-static-ipv6-sspi-schannel\      （同上结构）
  └─ libcurl-vc17-x86-release-static-ipv6-sspi-schannel\   （同上结构）
```

注意目录名的含义，选错会导致链接失败或运行期崩溃：

- `x64` / `x86` 必须与你的工程**目标平台一致**；
- `debug` / `release` 必须与你的**配置一致**（`libcurl_a_debug.lib` 对 Debug，`libcurl_a.lib` 对 Release）；
- `static` 表示静态库，因此**必须**定义 `CURL_STATICLIB`，否则会出现 `__imp_curl_*` 未解析符号；
- `sspi` + `schannel` 表示使用 Windows 自带的认证与 TLS（不依赖 OpenSSL），这也是必须额外链接 `Crypt32`、`Normaliz`、`Wldap32` 的原因。

### 3.2 步骤二：拷贝源文件

拷贝以下文件到你的工程（保持文件名不变，因为源码里用的是 `#include "xxx.h"`）：

```
weather.h        weather.cpp
WebPage.h        WebPage.cpp
Tools.h          Tools.cpp
WeatherException.h   WeatherException.cpp
pch.h            pch.cpp
colored_cout.h   colored_cout.cpp     ← 仅控制台形态需要
```

关于 `pch.h`：本项目**并未启用预编译头**（工程文件里没有 `PrecompiledHeader` 属性，MSVC 默认即"不使用"），`pch.h` 只是一个普通头文件，内容是 `#include <memory>`（`DeskTop_yc` 形态另有 winrt 头）。因此：

- 想省事就直接拷贝 `pch.h` / `pch.cpp`；
- 若不想引入，也可删掉各 `.cpp` 首行的 `#include "pch.h"`——除此之外它没有别的作用。

### 3.3 步骤三：配置工程

在"项目属性"中设置：

| 属性页 | 项目 | 值 |
| --- | --- | --- |
| C/C++ → 常规 → 附加包含目录 | `AdditionalIncludeDirectories` | `curl_include_lib\libcurl-vc17-<平台>-<配置>-static-ipv6-sspi-schannel\include` |
| C/C++ → 预处理器 → 预处理器定义 | `PreprocessorDefinitions` | `_CONSOLE`、`CURL_STATICLIB` |
| C/C++ → 语言 → C++ 语言标准 | `LanguageStandard` | `stdcpp20` 或更高（本仓库 `Debug\|x64` 用 `stdcpp23`） |
| C/C++ → 命令行 → 其他选项 | — | `/utf-8` |
| 链接器 → 常规 → 附加库目录 | `AdditionalLibraryDirectories` | `curl_include_lib\...\lib` |
| 链接器 → 输入 → 附加依赖项 | `AdditionalDependencies` | `libcurl_a.lib`（Debug 用 `libcurl_a_debug.lib`）、`Ws2_32.lib`、`Wldap32.lib`、`winmm.lib`、`Crypt32.lib`、`Normaliz.lib` |

两个容易漏掉的点：

- **`/utf-8` 是必需的**。源码里有中文字符串字面量，不加此选项 MSVC 会按本地代码页（GBK）解释源文件，导致中文字面量乱码甚至语法错误。
- **语言标准不低于 C++20**。`Tools.h` 用到 `<source_location>`，`weather.cpp` 用到 `std::format`（皆为 C++20）；`Tools.h` 还包含 `<stacktrace>`（C++23 头）。用 C++17 及以下会直接编译失败。

### 3.4 步骤四：初始化与收尾（顺序不可颠倒）

这是**最容易出错的一步**，因为初始化责任被放在了入口（`云寸.cpp:12-15`）而不是库内部：

```cpp
// ---- 程序启动，严格按此顺序 ----
curl_global_init(CURL_GLOBAL_ALL);   // 1. 控制台形态必须自己做；库内部不调用
Tools::CheckIfDataFileExists();      // 2. 创建 %AppData%\yc 并设置 Tools::DataLocation
Tools::InitLogSystem();              // 3. 创建 log\ 子目录、启动后台刷写线程

// ---- 业务调用 ... ----

// ---- 程序退出前 ----
Tools::ShutdownLogSystem();          // 4. 通知后台线程收尾，等待日志落盘
curl_global_cleanup();               // 5. 与 curl_global_init 配对
```

各步骤为什么不能省：

| 调用 | 为什么必须 |
| --- | --- |
| `curl_global_init` | 设计上由入口负责（`云寸.cpp:13`）。虽然 `curl_easy_init()` 会隐式触发一次全局初始化，但官方不推荐依赖此隐式行为（多线程下不安全） |
| `CheckIfDataFileExists()` | 它给 `Tools::DataLocation` 赋值（`Tools.cpp:305`）。`readFileIntoString()`、`WebPage::makeContent()`、`Weather::get_CURL()` 的文件路径都依赖它。**未调用则缓存读写会落到错误位置** |
| `InitLogSystem()` | 创建 `%AppData%\yc\log\`。若未调用，`log_write` 无处落盘（不会崩溃，但日志丢失） |
| `ShutdownLogSystem()` | 日志是**异步**的：`log_write` 只入队，由后台 `std::jthread` 批量落盘。不调用则进程退出时缓冲区内容可能丢失 |
| `curl_global_cleanup()` | 与 `curl_global_init` 配对（本项目自身遗漏了这一步，见 6.6） |

`CheckIfDataFileExists()` 在 `SHGetKnownFolderPath` 失败时会抛出 **`std::runtime_error`**（`Tools.cpp:247-248`），这是标准库异常而**不是** `ycresult`，所以只 `catch (const ycresult&)` 是接不住的。

### 3.5 步骤五：调用与读取结果

调用顺序有**数据依赖**：`location` 必须先成功，因为后两步的 URL 需要城市代码：

```
get_weather_kit(Type::location)   → 解析出城市名与城市代码 → 写入 Position
get_weather_kit(Type::instant)    → URL 依赖 Position 中的城市代码
get_weather_kit(Type::warnings)   → 同上
```

`Type::forecast` 已声明但**未实现**（见 [known_issues.md](known_issues.md) 第 10 条），不要使用。

结果从 `Weather` 对象的 public 成员读取，例如 `weather.Location`、`weather.CurrentTem`、`weather.Warnings`。

> ⚠️ 读取位置时**请用 `weather.Location` / `weather.LocationCode` 成员，不要用 `Position::getPosition()`**。原因是后者存在"每个翻译单元一份副本"的缺陷，详见 6.2。

### 3.6 最小可运行示例

保存在你自己工程里的 `my_app.cpp`（或替换你现有的 `main`）：

```cpp
#include "pch.h"          // 可选（本项目未启用预编译头，它只包含 <memory>）
#include "weather.h"      // 连带引入 WebPage.h / Tools.h / WeatherException.h 与 curl
#include <iostream>

int main()
{
	// (1) 控制台形态：自行初始化 libcurl
	curl_global_init(CURL_GLOBAL_ALL);

	// (2) 基础设施初始化（顺序不可颠倒）
	try
	{
		Tools::CheckIfDataFileExists();   // 创建 %AppData%\yc 并设置 DataLocation
	}
	catch (const std::exception& e)       // 注意：此处可能是 std::runtime_error
	{
		std::cerr << "初始化数据目录失败: " << e.what() << std::endl;
		curl_global_cleanup();
		return 1;
	}
	Tools::InitLogSystem();

	// (3) 业务对象：构造时自动设置请求头与时间戳
	yc::Weather weather{ yc::Type::instant };

	// (4) 三步取数：location 最先，失败则后续无意义
	bool ok = weather.get_weather_kit(yc::Type::location);
	if (ok) weather.get_weather_kit(yc::Type::instant);
	if (ok) weather.get_weather_kit(yc::Type::warnings);

	// (5) 读取结果：直接访问 public 成员
	if (ok)
	{
		std::cout << "城市: "   << weather.Location                       << '\n'
				  << "气温: "   << weather.CurrentTem           << " ℃" << '\n'
				  << "天气: "   << weather.CurrentWeather                << '\n'
				  << "风向: "   << weather.CurrentWind_Direction
				  << "  风速: " << weather.CurrentWind_speed              << '\n'
				  << "AQI: "    << weather.CurrentAQI                    << '\n'
				  << "更新: "   << weather.CurrentDate << ' ' << weather.UpdateTime << '\n';
		if (!weather.Warnings.empty())
			std::cout << "预警: " << weather.Warnings << '\n';
	}
	else
	{
		std::cerr << "获取天气失败，详见日志文件。" << std::endl;
	}

	// (6) 收尾：确保异步日志落盘
	Tools::ShutdownLogSystem();
	curl_global_cleanup();
	return ok ? 0 : 1;
}
```

要点说明：

- 这里的 `ok` 语义是"**下载并解析成功**"。`get_weather_kit` 内部已 `catch` 掉 `analysis()` 抛出的 `ycresult`，所以正常流程**不会抛异常**，用返回值判断即可。
- `instant` / `warnings` 的失败不会影响已获得的数据，因此可以像示例这样只在成功后再取，也可以无条件都调用。
- 示例刻意**没有**对 `CurrentAQI` 调用 `std::stoi`——原程序在 `云寸.cpp:64` 直接 `std::stoi(weatherInstant->CurrentAQI)`，一旦该字段为空或非数字会抛出未捕获的 `std::invalid_argument`（见 6.4）。

## 4. 平台分支：`_CONSOLE` 与 `DeskTop_yc`

两套网络实现由预处理宏在**编译期**二选一：

| 方面 | `_CONSOLE` | `DeskTop_yc` |
| --- | --- | --- |
| 适用形态 | 控制台 / Win32 桌面程序 | Windows App SDK / UWP（UI 版 `yc_GetWeather_UI`） |
| 网络实现 | libcurl（`Weather::get_CURL`） | C++/WinRT `HttpClient`（`Weather::get_HttpClient`） |
| 响应存放 | 先落盘到 `%AppData%\yc\*.txt`，再 `WebPage::read()` 读回 | 直接在内存中 `WebPage::make()` |
| 需要 `curl_global_init` | ✅ 需要 | ❌ 不需要 |
| 需要 libcurl | ✅ 需要 | ❌ 不需要 |
| 数据目录获取 | `SHGetKnownFolderPath(FOLDERID_RoamingAppData)` | `winrt::Windows::Storage::UserDataPaths` |
| 彩色输出 | 使用 `colored_cout` | 不使用（`Tools.h` 中的包含被 `#ifdef _CONSOLE` 排除） |

切换方式：**只改预处理器定义**（把 `_CONSOLE` 换成 `DeskTop_yc`），`weather.cpp` 内部的 `#ifdef` 分支会自动切换调用点（见 `weather.cpp:275-281`、`343-349`、`405-411`）。

`DeskTop_yc` 形态的三个注意点：

1. 需要 C++/WinRT 支持与相应的 winrt 头文件；`pch.h` 在该宏下会额外引入 `Windows.Web.Http` 等头。
2. `weather.cpp` 与 `Tools.cpp` 中存在**在函数体内写 `#include`** 的写法（如 `weather.cpp:190-192`、`Tools.cpp:255`）。这不是标准用法，某些编译配置下会报错或产生意外结果，迁移时建议把 `#include` 提到文件顶部。
3. `get_CURL()` 在非 `_CONSOLE` 下**没有实体函数体**，直接落到函数末尾的 `return false`（`weather.cpp:181-183`）。因此若只定义了 `DeskTop_yc`，任何仍调用 `get_CURL` 的代码都会静默返回 `false`。

## 5. 错误处理约定

### 5.1 先分清"返回值"与"异常"

这是本项目管理最需要理解的一点——**两层错误机制并存**：

| 层次 | 机制 | 行为 |
| --- | --- | --- |
| 网络层 `get_CURL` / `get_HttpClient` | 返回值 | 失败 `return false`。`weather.cpp:167` 有明确注释说明**故意不抛异常**（避免内存泄漏与不确定的返回路径） |
| 业务层 `get_weather` | 返回值 + 空 `catch` | 内部 `try/catch` 块捕获后**什么都不做**，仅返回 `false` |
| 解析层 `analysis` | **异常** | 解析失败会 `throw ycresult::ycresult(...)` |
| 聚合层 `get_weather_kit` | 返回值 | 捕获 `analysis` 的异常、写日志、返回 `false` |
| 基础设施 `readFileIntoString` / `getRoamingAppDataPath` | 异常 | 前者抛 `ycresult`（`file` / `content_too_long`），后者抛 **`std::runtime_error`** |

实践建议：

- 调用 `get_weather_kit()` 时**用返回值判断**，不需要 `try/catch`（它会自行消化 `ycresult`）。
- 调用 `analysis()`、`get_weather()`、`readFileIntoString()` 等更底层接口时，**必须** `try/catch`。
- 初始化阶段（`CheckIfDataFileExists`）要额外准备接住 `std::exception`。

### 5.2 异常类型与处理策略

`ycresult` 有两个维度：**类型**（`ExcptType`）说明"错在哪"，**处理方式**（`AddressMethod`）说明"该怎么办"。

| `ExcptType` | 含义 |
| --- | --- |
| `content_not_found` | 未找到所需字段（解析时最常见） |
| `content_too_long` | 内容超长 |
| `invalid_formatting` | 响应格式变化 / 头部异常 |
| `curl` | 网络错误（附 `curl_easy_strerror` 文本） |
| `file` | 文件打开/读取失败 |

| `AddressMethod` | 建议的处理 |
| --- | --- |
| `normal` | 正常情况 |
| `implicit` | 仅记录日志，不向用户展示 |
| `back` | 中止当前步骤、提示用户，程序**可继续** |
| `crash` | 无法恢复，应记录日志后退出 |

建议的处理骨架（参考 `云寸.cpp:35-48` 的写法）：

```cpp
try
{
	// 可能抛 ycresult 的调用
}
catch (const yc::ycresult::ycresult& e)
{
	Tools::log_write(Tools::Err, e.what(), "my_app");
	if (e.get_AddressMethod() == yc::ycresult::AddressMethod::back ||
		e.get_AddressMethod() == yc::ycresult::AddressMethod::crash)
	{
		Tools::ShutdownLogSystem();   // 崩溃前确保日志落盘
		// ... 中止或退出
	}
}
```

## 6. 集成者必读的坑

| # | 坑 | 症状 | 规避方式 |
| --- | --- | --- | --- |
| 6.1 | 忘记调用 `Tools::CheckIfDataFileExists()` | 缓存文件写到错误目录，或读文件失败抛 `file` 异常 | 在 `main` 一开始就调用，且**早于**任何天气请求 |
| 6.2 | 跨文件使用 `Position::getPosition()` / `getPositionCode()` | **静默返回 `"ERR"`**，不报错、不崩溃，但数据错 | 只读 `Weather` 的 `Location` / `LocationCode` 成员；详见 6.2 展开 |
| 6.3 | 忘记 `CURL_STATICLIB` | 链接期大量 `__imp_curl_easy_*` 未解析符号 | 预处理器定义必加 `CURL_STATICLIB` |
| 6.4 | 直接 `std::stoi(weather.CurrentAQI)` | 字段为空或含非数字字符时抛未捕获的 `std::invalid_argument` 而崩溃 | 先判空、用 `std::from_chars` 或 `try/catch` 包裹 |
| 6.5 | 忘记 `Tools::ShutdownLogSystem()` | 程序退出时最后一段日志丢失（异步队列未刷盘） | 所有 `return` 路径（含错误分支）之前都调用 |
| 6.6 | 忘记 `curl_global_cleanup()` | 进程退出时 libcurl 全局资源未释放 | 与 `curl_global_init` 配对（本项目自身也遗漏，见 [known_issues.md](known_issues.md)） |
| 6.7 | 选择与工程不匹配的 libcurl（x86/x64、Debug/Release） | `LNK2038` 运行库不匹配，或链接错误 | 按 3.1 的目录含义严格对应 |
| 6.8 | 未加 `/utf-8` | 源码中文字面量乱码，或 C4819 警告 | 命令行加 `/utf-8` |
| 6.9 | 语言标准低于 C++20 | `std::format`、`<source_location>`、`<stacktrace>` 编译失败 | 设为 `stdcpp20` 或更高 |
| 6.10 | 在 `weather.h` 之前的头里包含 `WebPage.h` 造成循环包含 | 编译错误或类型不完整 | 遵守 `Tools.h:5` 的警告："不得包含 `WebPage.h`" |
| 6.11 | 使用 `Type::forecast` | 请求返回失败，无任何结果 | 只用 `instant` / `location` / `warnings` |
| 6.12 | 多次调用 `analysis()` 或反复 `get_weather_kit()` | 内存增长（`get_weather_kit` 异常路径泄漏 `WebPage`） | 见 [known_issues.md](known_issues.md) 第 3、4 条；避免在长驻进程中高频轮询 |

### 6.2 展开：`Position` 为什么"能跑但设计错误"

`weather.h:52-64`：

```cpp
namespace Position {
	static std::string location = "ERR", code = "ERR";
	static void  setPosition(std::string lct, std::string cd) { location = lct; code = cd; }
	static std::string getPosition()     { return location; }
	static std::string getPositionCode() { return code; }
}
```

**命名空间作用域下的 `static` 具有内部链接（internal linkage）**，因此每一个 `#include "weather.h"` 的**翻译单元（.cpp）都会得到一份完全独立的 `location` / `code` 副本**。

后果：

- `Weather::analysis()` 在 `weather.cpp:46` 调用 `Position::setPosition()` 时，修改的是 **`weather.cpp` 自己那一份**；
- 你的 `my_app.cpp` 里调用 `Position::getPosition()`，读到的是**你自己那份、从未被赋值、恒为 `"ERR"` 的副本**；
- 全程**没有报错，也不会崩溃**，只是数据静默错误——这是最难排查的一类缺陷。

本项目之所以"看起来正常"，是因为 `Position` 的**写入（`weather.cpp:46`）与读取（`weather.cpp:330`、`394`）恰好都在 `weather.cpp` 同一个翻译单元内**，共享同一份副本。`云寸.cpp` 读取的是 `Weather` 的成员 `Location`，而不是 `Position`，所以也没暴露问题。

**结论：集成时把 `Position` 视为"库内部私有实现"，不要在自己的代码里访问它。**

> 该问题也已登记在 [known_issues.md](known_issues.md) 第 6 条（严重程度：**高**），完整的影响分析与四种修复方向见该条目。

## 7. 相关文档

| 主题 | 文档 |
| --- | --- |
| 整体架构与数据流 | [Overview.md](Overview.md) |
| 核心业务 `Weather` 细节 | [weather.md](weather.md) |
| 响应内容封装 `WebPage` | [WebPage.md](WebPage.md) |
| 基础设施与异步日志 | [Tools.md](Tools.md) |
| 异常体系 | [WeatherException.md](WeatherException.md) |
| 平台开关与预编译头 | [pch.md](pch.md) |
| 工程与构建配置 | [build_config.md](build_config.md) |
| libcurl / colored-cout 说明 | [third_party.md](third_party.md) |
| 已知风险与改造方向 | [known_issues.md](known_issues.md) |
