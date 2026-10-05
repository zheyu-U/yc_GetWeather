# Tools — 通用工具与异步日志系统

> 源文件：`Tools.h` / `Tools.cpp`　|　所属层：基础设施
> 职责概述：提供通用工具（数据目录、读文件、取时间）与一套生产者-消费者模型的异步日志系统。

## 1. 概述

`namespace Tools` 提供两类能力：

1. **通用工具**：数据目录定位、读文件、取当前时间；
2. **异步日志系统**：日志队列 + `std::jthread` 后台线程批量落盘，支持彩色控制台输出、`InfoSet` 结构化信息与调用栈。

> `Tools.h` 顶部注释强调：**不得包含 `WebPage.h`**（避免循环依赖）。

## 2. 文件与来源

| 项 | 内容 |
| --- | --- |
| 源文件 | `Tools.h`（94 行）/ `Tools.cpp`（517 行） |
| 是否参与编译 | 是（`ClCompile` / `ClInclude`） |
| 是否第三方 | 否 |
| 依赖 | `WeatherException.h`、`colored_cout.h`（`_CONSOLE`）、Win32 `shlobj.h`、C++ 标准库（`<filesystem>`、`<thread>`、`<stacktrace>` 等） |

## 3. 关键定义

### 3.1 版本宏

```cpp
#define YC_GETWEATHER_CORE_VERSION "1.3.6.0"
```

被 `weather.cpp::setHeaders()` 用作 User-Agent。

### 3.2 日志级别

```cpp
static const int Err{12}, Warn{11}, Info{10}, FErr{13};
```

`lvl >= 12`（即 `Err` / `FErr`）的日志会在没有显式调用栈时自动附加 `std::stacktrace::current()`。

### 3.3 `Detail` / `InfoSet`

- `Detail`：键值对，含 `name`（键名）与 `key`（值）。
- `InfoSet`：一组 `Detail`，提供 `ToString()`（单行 `name:key; ...`）与 `GetEachString(i)`（对齐单条）、`size()`。

### 3.4 日志系统内部状态（`Tools.cpp` 匿名命名空间）

| 名称 | 类型 | 作用 |
| --- | --- | --- |
| `logMutex` | `std::mutex` | 保护 `logQueue` |
| `logCV` | `std::condition_variable` | 队列有数据或停止时唤醒后台线程 |
| `logQueue` | `std::queue<std::string>` | 生产者到消费者的数据通道 |
| `logFileStream` | `std::ofstream` | 日志文件流 |
| `logFlusherThread` | `std::jthread` | 后台刷写线程（C++20，析构自动 `request_stop()` + `join()`） |
| `logSystemRunning` | `std::atomic<bool>` | 系统是否已初始化的标志 |

## 4. 主要接口

### 4.1 通用工具

| 接口 | 说明 |
| --- | --- |
| `std::string getRoamingAppDataPath()` | 用 `SHGetKnownFolderPath(FOLDERID_RoamingAppData)` 取 `%AppData%` 路径 |
| `void CheckIfDataFileExists()` | 创建 `%AppData%\yc`，失败回退到 `\yc`，再失败 `exit(0)`；设置全局 `DataLocation` |
| `std::string Get_DataLocation()` | 返回 `DataLocation`（末尾无斜杠） |
| `void readFileIntoString(std::string Path, std::string* str)` | 读文件到字符串；打不开抛 `file`，超长抛 `content_too_long` |
| `std::string String_GetCurrentTime()` | 返回 Unix 毫秒时间戳字符串 |

### 4.2 日志接口

| 接口 | 说明 |
| --- | --- |
| `log_write(lvl, writelog, region = 函数名)` | 基本写日志 |
| `log_write(lvl, writelog, region, InfoSet info)` | 附带结构化信息 |
| `log_write(lvl, writelog, region, InfoSet info, std::vector<std::string> call)` | 附带调用栈 |
| `void InitLogSystem()` | 启动异步日志系统 |
| `void ShutdownLogSystem()` | 停止异步日志系统并落盘 |

三个 `log_write` 重载统一走内部 `log_write_impl()`。

### 4.3 关键内部实现

| 名称 | 说明 |
| --- | --- |
| `logFlusherFunc(std::stop_token)` | 后台刷写线程主函数 |
| `log_write_impl(...)` | 统一实现：格式化时间戳 + 级别标签，写文件队列 + 控制台输出 |
| `enqueueLogLine(...)` | 入队并通知（**当前未被调用**，见 `known_issues.md` 第 11 条） |

## 5. 工作流程 / 调用关系

### 5.1 日志写入（生产者）

```
log_write(...)  →  log_write_impl(...)
	  ├─ 格式化：time_ + 级别标签 → StampOutline
	  ├─ 若 logSystemRunning：加锁，按顺序 push 主行 → [INFOSET] 行 → 各 Detail 行 → [CALLSTACK] 行；notify_one
	  └─ 控制台：按级别上色输出（Release 下 Info 级不显示）
```

### 5.2 后台刷写（消费者）

```
logFlusherFunc(stoken)：
  while (!stoken.stop_requested()):
	  加锁 wait_for(500ms, 队列非空 或 stop_requested)
	  批量 std::move 取出到 batch（锁内）
	  锁外写文件 ← 不阻塞生产者
	  flush()
  收到停止后：兜底写出队列残留 → 最终 flush
```

> 关键设计：文件 I/O 在锁外进行；`wait_for` 每 500ms 自动醒来一次，避免 `notify` 丢失导致日志不落盘。

### 5.3 生命周期

```
InitLogSystem()：
  1. 创建 %AppData%\yc\log（失败抛 crash 级 file 异常）
  2. 生成 yc_log_<时间>.log
  3. 以追加模式打开文件流
  4. 写 "Session Start (v...)" 并 flush
  5. 启动 logFlusherThread，置 logSystemRunning = true

ShutdownLogSystem()：
  未初始化则直接返回；
  否则 写 "Session End" → request_stop() → join() → 关文件 → logSystemRunning = false
```

### 5.4 `readFileIntoString` 流程

```
open(Path) → 失败抛 ycresult(file, back)
（size 检查，当前失效，见下）
clear() 目标串 → 逐字符 get() push_back
```

## 6. 依赖关系

- **依赖**：`WeatherException`（`ycresult`）、`colored_cout`（控制台上色）、Win32 `shlobj.h`、C++20 标准库。
- **被依赖**：`云寸.cpp`、`weather.cpp`、`WebPage.cpp`（全项目基础设施）。

## 7. 注意事项 / 已知问题

- 调用顺序约束：`InitLogSystem()` 必须在 `CheckIfDataFileExists()` 之后、任何 `log_write()` 之前调用。
- `readFileIntoString` 的大小检查失效（`gcount()` 在读取前调用，恒为 0），且逐字符读取效率低（详见 `known_issues.md` 第 5、11 条）。
- `enqueueLogLine()` 为死代码，未被任何地方调用（详见 `known_issues.md` 第 11 条）。
- `GetEachString` 使用 `{:>32}` 格式化，与 `log_write_impl` 中 `StampOutline` 的宽度约定耦合，改动需同步。
- 后台线程最长 500ms 刷写一次，崩溃时可能丢失最近日志（详见 `known_issues.md` 第 11 条）。
