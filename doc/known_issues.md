# known_issues — 已知问题清单与后续开发方向

> 所属层：全项目　|　来源：通读全部源码后的整理
> 职责概述：汇总设计风险、潜在缺陷与遗留问题，作为后续维护与二次开发的方向指引。

## 概述

本页汇总在通读全项目源码后发现的**设计风险、潜在缺陷与遗留问题**，作为后续维护与二次开发（含 `yc_GetWeather_UI` 复用）的方向指引。

- 每条记录包含：**问题描述、所在位置、影响、严重程度、建议修复方向**。
- 严重程度约定：
  - **高**：可能直接导致程序崩溃、功能失败或数据错误，应优先处理；
  - **中**：可稳定运行但存在隐患（内存泄漏、逻辑失效、可维护性差），应尽快修复；
  - **低**：代码整洁度、遗留文件、边缘情况，可择机处理。

## 问题总览

| # | 问题 | 位置 | 严重程度 |
| --- | --- | --- | --- |
| 1 | 自研解析器 `ReadQuoteContent` 格式脆弱 | `weather.cpp` | 高 |
| 2 | `std::stoi(CurrentAQI)` 未加防护 | `云寸.cpp` | 高 |
| 3 | `get_weather_kit` 解析异常路径泄漏 `WebPage` | `weather.cpp` | 中 |
| 4 | `get_weather` 中 `new std::string` 文件名未释放 | `weather.cpp` | 中 |
| 5 | `readFileIntoString` 文件大小检查失效 | `Tools.cpp` | 中 |
| 6 | `Position` 的 `static` 为内部链接，跨文件静默读到 `"ERR"` | `weather.h` | 高 |
| 7 | `WebPage::make` 未设置 `url` 成员 | `WebPage.cpp` | 中 |
| 8 | `WebPage::makeContent(fileName)` 自赋值错误 | `WebPage.cpp` | 低 |
| 9 | `Tools_.cpp` 遗留文件、`resource1.h` 缺失引用 | 工程 | 低 |
| 10 | `Type::forecast` 声明但未实现 | `WebPage.h` / `weather.cpp` | 低 |
| 11 | 其它代码整洁问题（未使用字段、死代码、裸指针风格等） | 多处 | 低 |
| 12 | 跨平台移植问题，使用curl必须宏定义 _console | `weather.cpp` `Intergration.md` ` | 低* |

---

## 1. 自研解析器 `ReadQuoteContent` 格式脆弱

- **位置**：`weather.cpp` → `Weather::ReadQuoteContent()`（约 70–86 行），调用点见 `Weather::analysis()`。相关说明见 `weather.md`。
- **描述**：该函数不是标准 JSON 解析，而是纯字符串匹配——先 `find(键名)`，再找 `:`/`=`，再按值起始字符（`"`、`{`、`[`）找对应结束符（`"`、`}`、`]`）截取中间内容。它依赖字段顺序、引号形式与转义规则，天气网响应格式一旦变化（字段重排、值含转义引号、嵌套结构变化）就会解析失败或取到错误值。
- **影响**：网页格式变动后程序大面积解析失败（`analysis` 抛 `content_not_found`），或静默取到错误的天气数据；代码自身也以 `invalid_formatting`（"Formatting changed"）提示该脆弱性。
- **严重程度**：高。
- **建议修复方向**：
  1. 引入标准 JSON 解析库（如 `nlohmann/json`），把 `analysis` 改为按字段名取值，彻底摆脱对文本顺序/引号的依赖；
  2. 若暂不引入依赖，至少用正则表达式替代手工数引号，并对转义字符做处理；
  3. 增加解析单元测试，用真实响应样本固定回归用例，格式变化时可第一时间发现。

## 2. `std::stoi(CurrentAQI)` 未加防护

- **位置**：`云寸.cpp` 输出块（约 64–69 行）。相关说明见 `云寸.md`。
- **描述**：AQI 分级直接调用 `std::stoi(weatherInstant->CurrentAQI)`，未做异常捕获或数字校验。若该字段解析为空串或含非数字字符（网页格式变化、字段缺失时容易出现），`stoi` 会抛出 `std::invalid_argument` / `std::out_of_range`。
- **影响**：该异常在 `main` 中**未被捕获**，将直接导致程序崩溃；且此时异步日志系统可能来不及落盘。
- **严重程度**：高。
- **建议修复方向**：
  1. 改用 `std::from_chars` 做无异常解析，解析失败给出默认显示（如"未知"）；
  2. 或对 `stoi` 做 `try/catch`，捕获后按"未知/无数据"处理；
  3. 更彻底的做法：在 `Weather::analysis` 中就校验 AQI 为纯数字，非法则记录日志并置空。

## 3. `get_weather_kit` 解析异常路径泄漏 `WebPage`

- **位置**：`weather.cpp` → `Weather::get_weather_kit()`（约 448–480 行）。
- **描述**：

  ```cpp
  WebPage::WebPage* pg = new WebPage::WebPage;
  bool res = get_weather(type, pg);
  if (res) {
	  try { analysis(pg); }
	  catch (const ycresult::ycresult& e) {
		  // ...记录日志...
		  return false;      // <-- 此处直接返回，未 delete pg
	  }
	  delete pg;
	  return true;
  }
  ```

  当 `analysis` 抛异常进入 `catch` 时，直接 `return false`，`pg` 未被释放。
- **影响**：解析失败（网页格式变化等场景）时每次泄漏一个 `WebPage` 对象；作为长驻库/UI 复用时泄漏会累积。
- **严重程度**：中。
- **建议修复方向**：用 `std::unique_ptr<WebPage::WebPage>` 管理（源码中已有被注释掉的 `make_unique` 写法可参考），或调整控制流保证所有分支都 `delete pg`。

## 4. `get_weather` 中 `new std::string` 文件名未释放

- **位置**：`weather.cpp` → `Weather::get_weather()`，`c_fileName`（约 334 行）、`w_fileName`（约 395 行）。
- **描述**：`std::string* c_fileName = new std::string;` / `std::string* w_fileName = new std::string;` 分配后仅用于拼文件名，函数返回前从未 `delete`。
- **影响**：每次抓取实时天气/预警都会泄漏一小段内存，长期运行累积。
- **严重程度**：中。
- **建议修复方向**：直接改为栈上的 `std::string c_fileName = ...;`（无需堆分配），传引用即可。

## 5. `readFileIntoString` 文件大小检查失效

- **位置**：`Tools.cpp` → `Tools::readFileIntoString()`（约 266–284 行）。相关说明见 `Tools.md`。
- **描述**：第 276 行 `size_t cnt = file.gcount();` 在**读取之前**调用。`gcount()` 返回上一次无格式输入操作提取的字符数，此时尚未读取，恒为 0，因此后续 `if (cnt >= 655'360)` 的长度限制永远不会触发。
- **影响**：预期的"文件过大即拒绝"防护形同虚设；读取超大文件会持续 `push_back` 直到内存压力。
- **严重程度**：中。
- **建议修复方向**：
  1. 打开文件后先用 `std::filesystem::file_size` 判断大小；
  2. 或用 `seekg(0, std::ios::end)` / `tellg()` 获取长度；
  3. 再按大小一次性读取（顺带替换逐字符 `push_back` 的低效实现）。

## 6. `Position` 的 `static` 为内部链接，跨文件静默读到 `"ERR"`

- **位置**：`weather.h` → `namespace Position`（约 52–64 行）。相关说明见 `weather.md`。
- **描述**：定位结果（城市名、城市代码）通过 `weather.h` 中 `namespace Position` 内的 `static` 变量在多次 `get_weather_kit` 调用间传递；默认值为 `"ERR"`。同时存在功能重复的 `Position(...)` 与 `setPosition(...)`。
- **关键机制（易被忽略，实际影响远大于"全局可变状态"本身）**：`static` 写在**命名空间作用域**（而不是类作用域）时，得到的是**内部链接（internal linkage）**，即每个 `#include "weather.h"` 的翻译单元（TU）都会各自生成**一份独立的副本**：

  ```cpp
  namespace Position {
      static std::string location = "ERR", code = "ERR";  // 每个 .cpp 一份
      static void setPosition(std::string, std::string);   // 只写本 TU 的那一份
      static std::string getPositionCode();                // 只读本 TU 的那一份
  }
  ```

  因此 `weather.cpp` 里的 `setPosition(...)` 写入的只是 **`weather.cpp` 自己**的副本；**任何其它 `.cpp`** 调用 `Position::getPositionCode()` 读到的永远是**该文件新副本的初值 `"ERR"`**。

  本项目"看起来一切正常"纯属巧合：`weather.cpp` 中仅有的两处读取（约 330 行、394 行）恰好与 `setPosition` 处在**同一个翻译单元**内，所以能读到真实值。一旦有第二个文件去读，问题立刻暴露。
- **影响**：
  1. **跨文件静默失败（最严重）**：外部文件（UI 层、新拆分的辅助模块）调用 `getPositionCode()` **不报错、不崩溃、不抛异常**，直接拿到 `"ERR"`，进而可能拼出 `http://d1.weather.com.cn/sk_2d/ERR.html` 之类的非法 URL，最终表现为"天气数据取不到"或"取到错误城市" —— 属于无任何提示的错误结果，排查成本极高；
  2. **违反直觉、易误判修法**：加 `inline` 或把变量搬到头文件都不能修正语义，必须去掉 `static` 使其具备外部链接，或改为类成员；
  3. 全局可变状态本身在并发调用或 UI 复用场景下会导致数据竞争与结果串扰；
  4. 默认值 `"ERR"` 若被误用会拼出非法 URL。
- **严重程度**：高（原评为"中"，因发现**内部链接导致跨 TU 读取静默返回错误值**而上调）。
- **建议修复方向**：
  1. **首选**：将定位结果改为 `Weather` 对象成员（已有 `Weather::Location` / `Weather::LocationCode`），通过 getter 访问，彻底消除全局状态与链接问题；
  2. **次选**：若必须保留自由函数，删除 `static` 使其具外部链接，并把**定义**移入 `weather.cpp`，头文件只留**声明**（避免 ODR 问题）；
  3. 删除功能重复的 `Position(...)` 构造函数，保留语义清晰的 `setPosition` / `getPosition`；
  4. 在拼接 URL 前校验城市代码有效性（非空、非 `"ERR"`），把"静默错误"变成"显式失败"（抛 `ycresult` 或返回 `false`）。

## 7. `WebPage::make` 未设置 `url` 成员

- **位置**：`WebPage.cpp` → `WebPage::make()`（约 21–29 行）。相关说明见 `WebPage.md`。
- **描述**：`make()` 设置了 `content`、`contenttype`、`doctype`、`encoding`，但**遗漏了 `this->url = _url;`**（对比 `read()` 有设置）。参数 `_url` 被丢弃。
- **影响**：桌面/UWP（`DeskTop_yc`）分支构造的 `WebPage`，其 `get_url()` 返回空字符串；任何依赖 `url` 的诊断/日志会缺失信息。
- **严重程度**：中。
- **建议修复方向**：在 `make()` 中补上 `this->url = _url;`，与 `read()` 行为保持一致。

## 8. `WebPage::makeContent(fileName)` 自赋值错误

- **位置**：`WebPage.cpp` → `WebPage::makeContent(std::string _fileName)`（约 48–63 行）。相关说明见 `WebPage.md`。
- **描述**：函数体第 60 行为 `this->fileName = fileName;`，把成员 `fileName` 赋给自己（应为参数 `_fileName`）。参数 `_fileName` 只用于拼路径，未回写到成员。
- **影响**：`get_content_p()` 在内容被 `freeContent()` 后走 `makeContent()`（无参）分支时，会使用**过期的 `fileName`** 去读文件，可能读到旧文件或打开失败。
  > 注：常规路径 `read()` 在第 12 行显式设置了 `this->fileName = _fileName`，因此掩盖了该缺陷，问题仅在直接调用 `makeContent(fileName)` 时暴露。
- **严重程度**：低（当前调用路径未触发，但属明显笔误）。
- **建议修复方向**：改为 `this->fileName = _fileName;`。

## 9. `Tools_.cpp` 遗留文件与 `resource1.h` 缺失引用

- **位置**：仓库根目录 `Tools_.cpp`（见 `Tools_.md`）；`云寸.vcxproj` 的 `ClInclude` 列表（见 `build_config.md`）。
- **描述**：`Tools_.cpp` 为旧版实现（含缺陷），未加入编译列表；`resource1.h` 在工程中被引用但仓库中不存在。
- **影响**：不影响构建，但会造成维护困惑与 IDE 中的缺失项提示。
- **严重程度**：低。
- **建议修复方向**：确认无需后删除 `Tools_.cpp` 或将其移入 `legacy/` 并注明；从工程文件中清理 `resource1.h` 引用。

## 10. `Type::forecast` 声明但未实现

- **位置**：`WebPage.h` 定义 `enum class Type { instant, forecast, location, warnings };`；`weather.cpp` 的 `get_weather` 与 `analysis` 中无 `forecast` 分支。
- **描述**：`forecast`（预报）类型已进入枚举与 `return_string`，但没有对应的下载 URL 与解析逻辑。
- **影响**：调用 `get_weather_kit(Type::forecast)` 会落入 `get_weather` 末尾的失败日志并返回 `false`；功能缺失但不易被察觉。
- **严重程度**：低（属未完成特性）。
- **建议修复方向**：要么补齐预报数据的 URL 与解析实现，要么在文档/接口处明确标注"暂不支持"，或在 `get_weather` 中直接记录"未实现"日志。

## 11. 其它代码整洁问题

以下问题不直接影响功能，但会降低可读性与可维护性，建议在后续重构中一并处理：

| 问题 | 位置 | 说明 |
| --- | --- | --- |
| 未使用字段 | `weather.h` | `MaxTem`、`MinTem`、`CurrentWeekday`、`CurrentRainfall`、`Rainfall24`、`PublicIP` 等解析/保存后未参与输出 |
| 多余变量 | `云寸.cpp` | `if(res1) bool res2 = ...; if(res1) bool res3 = ...;` 中的 `res2`、`res3` 作用域受限且未被使用 |
| 死代码 | `Tools.cpp` | `enqueueLogLine()` 定义后未被调用（`log_write_impl` 直接操作队列） |
| 函数体内 `#include` | `weather.cpp` | `get_HttpClient` 在函数体内 `#include <winrt/...>`，非常规写法，建议移至 `pch.h`/文件头部并受宏保护 |
| 裸指针管理 | `weather.cpp` / `云寸.cpp` | 多处 `new`/`delete`（`Weather* weatherInstant`、`WebPage* pg`），异常路径易泄漏，建议统一用智能指针 |
| 日志落盘窗口 | `Tools.cpp` | 后台线程最长 500ms 才刷写一次，崩溃时可能丢失最近的日志（当前已有 flush，属可接受的权衡，记录备查） |
| 文本读取效率 | `Tools.cpp` | `readFileIntoString` 逐字符 `push_back`，大文件效率低，可改为整块读取 |

---

## 优先级建议

1. **立即处理（高）**：第 1 项（解析器）、第 2 项（`stoi` 崩溃点）。
2. **尽快处理（中）**：第 3、4、5、6、7 项。
3. **择机处理（低）**：第 8、9、10、11 项。

## 备注

- 本页基于当前代码状态（`YC_GETWEATHER_CORE_VERSION` = `1.3.6.0`）整理。
- 修复任一项时，建议同步更新对应的模块文档（`云寸.md`、`weather.md`、`WebPage.md`、`Tools.md`、`WeatherException.md` 等）与根目录 `Release.md`。
