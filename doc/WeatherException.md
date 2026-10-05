# WeatherException — 自定义异常体系

> 源文件：`WeatherException.h` / `WeatherException.cpp`　|　所属层：基础设施
> 职责概述：用两个枚举描述"出了什么问题"（`ExcptType`）与"该怎么处理"（`AddressMethod`），并提供异常类 `ycresult`。

## 1. 概述

`yc::ycresult` 命名空间定义了一套轻量异常体系，统一全项目的错误表达与处理策略。

## 2. 文件与来源

| 项 | 内容 |
| --- | --- |
| 源文件 | `WeatherException.h`（78 行）/ `WeatherException.cpp`（18 行） |
| 是否参与编译 | 是（`ClCompile` / `ClInclude`） |
| 是否第三方 | 否 |
| 依赖 | C++ 标准库 `<exception>`、`<string>` |

## 3. 关键定义

### 3.1 `enum class ExcptType`（异常类型）

```cpp
enum class ExcptType { content_too_long, content_not_found, invalid_formatting, curl, file };
```

| 值 | 含义 | `return_string_ExcptType()` 返回 |
| --- | --- | --- |
| `content_too_long` | 内容超长 | The content is out of length. |
| `content_not_found` | 找不到所需内容 | Cannot find the required content. |
| `invalid_formatting` | 格式未知 | Unknown formatting. |
| `curl` | 网络错误 | Curl error. |
| `file` | 文件错误 | File error. |

### 3.2 `enum class AddressMethod`（处理方式）

```cpp
enum class AddressMethod { normal, implicit, back, crash };
```

| 值 | 含义 |
| --- | --- |
| `normal` | 正常情况 |
| `implicit` | 记录异常但不显示（配合 `!_debug`） |
| `back` | 退出当前步骤并提示 |
| `crash` | 崩溃 |

### 3.3 类 `ycresult`

继承 `std::exception`，私有成员为 `message`、`type`、`method`。

## 4. 主要接口

| 接口 | 说明 |
| --- | --- |
| `explicit ycresult(const ExcptType type, const AddressMethod method, const std::string_view message = "We've been trapped in trouble! ")` | 构造 |
| `const char* what() const noexcept override` | 返回 `message.c_str()` |
| `AddressMethod get_AddressMethod() const noexcept` | 返回处理方式枚举 |
| `std::string return_string_ExcptType() const noexcept` | 返回异常类型的字符串描述 |
| `std::string return_string_AddressMethod() const noexcept` | 返回处理方式的字符串描述 |

## 5. 工作流程 / 调用关系

### 抛出端（以 `Weather::analysis` 为例）

```cpp
throw ycresult::ycresult(ET::content_not_found, AM::back, "temp\"");
```

### 捕获端（以 `main`、`get_weather_kit` 为例）

```cpp
catch (const ycresult::ycresult& e) {
	log_write(..., e.what(), ...);
	if (e.get_AddressMethod() == ycresult::AddressMethod::back || ...) { /* 处理 */ }
}
```

`main` 中当处理方式为 `back` / `crash` 时，会写致命日志、`ShutdownLogSystem()` 并 `exit(1)`。

## 6. 依赖关系

- **被依赖**：`weather.cpp`、`WebPage.cpp`、`Tools.cpp`、`云寸.cpp`（全项目抛出/捕获）。
- **依赖**：无（仅标准库）。

## 7. 注意事项 / 已知问题

- `WeatherException.cpp` 的内容是**全部注释掉的旧版实现**（静态常量定义方式），无实际代码，属从"静态常量枚举"重构为"scoped enum"时留下的痕迹，可删除。
- 枚举使用 `enum class`，但类名与命名空间同名（`yc::ycresult::ycresult`），可读性略差但合法。
- `message` 以 `std::string_view` 传入后拷贝到 `std::string` 存储，避免悬垂。
