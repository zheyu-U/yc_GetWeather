# Tools_ — 遗留旧版工具实现

> 源文件：`Tools_.cpp`　|　所属层：基础设施（遗留，未编译）
> 职责概述：`Tools.cpp` 的旧版备份，**不参与编译**，仅作历史记录保留。

## 1. 概述

`Tools_.cpp` 是 `Tools.cpp` 的**旧版本备份**，**未被 `云寸.vcxproj` 引用，不参与编译**。相较于现在的 `Tools.cpp`，它缺少若干改进，并存在缺陷。本文件保留说明以记录其存在，避免维护者误改旧版本。

## 2. 文件与来源

| 项 | 内容 |
| --- | --- |
| 源文件 | `Tools_.cpp`（412 行） |
| 是否参与编译 | **否**（不在 `ClCompile` 列表中） |
| 是否第三方 | 否 |
| 定位 | 遗留产物，建议删除或移出仓库 |

## 3. 关键定义

与现版 `Tools.cpp` 相比，存在 200+ 行差异（`Compare-Object` 统计约 217 处）。主要差异点见第 5 节。

## 4. 主要接口

旧版同样提供 `log_write` 三个重载，但实现退化为彼此递归调用（见第 5、7 节）；`InfoSet` 相关接口不完整。

## 5. 与当前版的主要差异

1. **无 `InfoSet` / `Detail` 结构化信息**：旧版 `log_write` 三个重载全部退化为调用基础 `log_write(lvl, writelog, region)`，丢弃 `InfoSet info` 与 `call` 参数。
2. **缺少 `GetEachString`**：旧版没有 `InfoSet::GetEachString`。
3. **日志级别标签不同**：`FErr` 输出为 `[FATAL ERROR]`（现版为 `[FTL ERROR]`）。
4. **文件写入逻辑耦合在 `log_write` 内**：旧版在 `log_write` 里直接加锁 push 队列；现版抽出了统一的 `log_write_impl`。
5. **`InitLogSystem` 用 `std::cout` 输出日志文件路径**（现版用 `log_write`）。

## 6. 依赖关系

（遗留文件，不参与编译，无有效依赖关系。）

## 7. 注意事项 / 已知问题

- 旧版 `log_write` 的三个重载会互相递归调用（如 `log_write(lvl,msg,region,info)` 调用 `log_write(lvl,msg,region)`，后者又可能继续调用……），存在**无限递归风险**。这正是它被现版取代的原因之一。
- 建议：该文件为遗留产物，可直接删除或移出仓库，避免维护者误改旧版本。
