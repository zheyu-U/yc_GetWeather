# resource — 资源脚本（图标与版本信息）

> 源文件：`resource.h` / `云寸.rc`　|　所属层：构建资源
> 职责概述：为可执行文件提供图标与版本信息。

## 1. 概述

Windows 资源脚本，为生成的可执行文件提供程序图标与版本信息（`VS_VERSION_INFO`）。

## 2. 文件与来源

| 项 | 内容 |
| --- | --- |
| 源文件 | `resource.h`（16 行）/ `云寸.rc` |
| 是否参与编译 | 是（`ResourceCompile Include="云寸.rc"`；`resource.h` 在 `ClInclude`） |
| 是否第三方 | 否 |
| 编码 | `云寸.rc` 以 **UTF-16 LE（含 BOM）** 存储 |

## 3. 关键定义

`resource.h` 定义资源 ID：

```cpp
#define IDI_ICON1 101
```

`云寸.rc` 结构：

1. `#include "resource.h"` + `#include "winres.h"`；
2. `LANGUAGE LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED`（简体中文）；
3. `TEXTINCLUDE`（资源编辑器用）；
4. **Icon**：`IDI_ICON1 ICON "Assets\\icon256.ico"`；
5. **Version**：`VS_VERSION_INFO`，`FILEVERSION/PRODUCTVERSION 1,3,6,0`。

### 版本信息字段（StringFileInfo）

| 键 | 值 |
| --- | --- |
| CompanyName | zheyu-U |
| FileDescription | 云寸 (To view more on github: https://github.com/zheyu-U/yc_GetWeather) |
| FileVersion / ProductVersion | 1.3.6.0 |
| InternalName / OriginalFilename | 云寸.exe |
| LegalCopyright | Copyright (C) EPL-2.0 license |
| ProductName | yc_GetWeather |

## 4. 主要接口

（资源脚本，无函数接口。）

## 5. 工作流程 / 调用关系

构建时由资源编译器（`rc.exe`）处理 `云寸.rc`，把图标与版本块写入最终 exe；`resource.h` 中 `IDI_ICON1` 供资源脚本与工程引用。

## 6. 依赖关系

- **依赖**：`resource.h`、`winres.h`、`Assets\icon256.ico`。
- **被依赖**：`云寸.vcxproj` 的 `ResourceCompile`。

## 7. 注意事项 / 已知问题

- 版本号 `1.3.6.0` 与 `Tools.h` 中的 `YC_GETWEATHER_CORE_VERSION` 保持一致，修改版本时需同步。
- `云寸.rc` 为 UTF-16 LE，用 ANSI/UTF-8 查看会乱码，属正常现象。
- 关联文件：`Assets\icon256.ico`（图标，约 270 KB，多尺寸）、`Assets\icon_small.psd`（图标源文件，不编译进程序）。
- `云寸.vcxproj` 引用了 `resource1.h`，但仓库中**不存在**该文件，属残留引用（详见 `build_config.md` 与 `known_issues.md` 第 9 条）。
