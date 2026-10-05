# WebPage — 网页内容封装

> 源文件：`WebPage.h` / `WebPage.cpp`　|　所属层：内容封装
> 职责概述：表示"一个网页 / 一次 HTTP 响应"的内容及其元数据，是 `Weather` 与原始数据之间的抽象层。

## 1. 概述

`yc::WebPage::WebPage` 封装一次 HTTP 响应的正文与元数据。分工为：`Weather::get_weather()` 负责把数据装进 `WebPage`，`Weather::analysis()` 再从 `WebPage` 读出内容解析。

`WebPage.h` 同时定义了全局的 `yc::Type` 枚举（供全项目使用）。

## 2. 文件与来源

| 项 | 内容 |
| --- | --- |
| 源文件 | `WebPage.h`（79 行）/ `WebPage.cpp`（85 行） |
| 是否参与编译 | 是（`ClCompile` / `ClInclude`） |
| 是否第三方 | 否 |
| 依赖 | `Tools.h`（读文件）、`WeatherException.h` |

## 3. 关键定义

### 3.1 `enum class Type`（位于 `yc` 命名空间）

```cpp
enum class Type { instant, forecast, location, warnings };
```

配套自由函数 `return_string(Type)` 返回枚举对应的字符串，用于日志。

### 3.2 `enum class DocType` / `enum class Encoding`（位于 `yc::WebPage`）

```cpp
enum class DocType  { html, JavaScript };   // 文档格式
enum class Encoding { utf8, gbk };          // 编码（未实现，仅预留）
```

### 3.3 类成员

| 成员 | 说明 |
| --- | --- |
| `std::string fileName` | 控制台模式下对应的本地缓存文件名 |
| `std::string url` | 来源 URL |
| `Type contenttype` | 内容类型 |
| `DocType doctype` | 文档格式 |
| `Encoding encoding` | 编码（当前未使用） |
| `std::string content` | 实际内容 |
| `bool has_content_ = 0` | 是否已持有内容 |

## 4. 主要接口

| 接口 | 说明 |
| --- | --- |
| `void read(fileName, contentType, doctype, url, encoding)` | 从磁盘文件加载内容并填充元数据，供 `_CONSOLE` 分支使用 |
| `void make(source, pageType, doctype, url, encoding)` | 从内存字符串拷贝构造，供 `DeskTop_yc` 分支使用 |
| `void makeContent()` | 按成员 `fileName` 读取 `%AppData%\yc\<fileName>` 到 `content` |
| `void makeContent(fileName)` | 按参数 `fileName` 读取文件到 `content` |
| `void freeContent()` | 清空 `content` 并置 `has_content_ = 0` |
| `std::string* get_content_p()` | 返回 `&content`；若尚无内容则先 `makeContent()` |
| `std::string get_url() const` | 取来源 URL |
| `Type get_contenttype() const` | 取内容类型 |
| `Encoding get_encoding() const` | 取编码 |
| `bool has_content() const` | 是否已持有内容 |

类外还提供两个静态包装：`readWebPage(...)`、`makeWebPage(...)`（详见第 7 节）。

## 5. 工作流程 / 调用关系

```
Weather::get_weather()
  ├─ _CONSOLE   ：page->read(fileName, type, DocType::JavaScript, url)
  │                 └─ makeContent(fileName) → Tools::readFileIntoString(%AppData%\yc\fileName) → content
  └─ DeskTop_yc ：page->make(&resContent, type, DocType::JavaScript, url)
					└─ content = *source（拷贝）

Weather::analysis(page)
  └─ page->get_content_p() → 解析 content 中的字段
```

`get_content_p()` 的惰性加载：若 `has_content_` 为假，会先调 `makeContent()`（使用成员 `fileName`）再返回。

## 6. 依赖关系

- **依赖**：`Tools`（`Get_DataLocation()`、`readFileIntoString()`）、`WeatherException`（`ycresult`）。
- **被依赖**：`Weather`（持有并消费 `WebPage`）。

## 7. 注意事项 / 已知问题

- **`make()` 未设置 `url`**：与 `read()` 不同，`make()` 遗漏了 `this->url = _url;`（详见 `known_issues.md` 第 7 条）。
- **`makeContent(fileName)` 自赋值笔误**：函数内为 `this->fileName = fileName;`（应为参数 `_fileName`），导致成员未被正确更新（详见 `known_issues.md` 第 8 条）。
- **类外包装无效**：`makeWebPage(WebPage webpage, ...)` 按值传参，修改的是副本，调用无效；建议改用成员函数或引用/指针。
- `Encoding` 编码转换**未实现**，`read` / `make` 均忽略该参数。
- 文件尾部注释 `//constexpr WebPage::Type ...` 为遗留残留，可删。
