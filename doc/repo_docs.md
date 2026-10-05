# repo_docs — 仓库级文档与子模块

> 源文件：`README.md` / `Release.md` / `LICENSE` / `.gitmodules`　|　所属层：仓库说明
> 职责概述：记录仓库根目录级文档文件与第三方子模块信息。

## 1. 概述

本页记录仓库根目录的说明类文件、许可与 Git 子模块（第三方库）信息。各源代码文件的详细文档见本目录其它分篇，第三方库的简短说明见 `third_party.md`。

## 2. 文件与来源

| 文件 | 说明 |
| --- | --- |
| `README.md` | 项目介绍 |
| `Release.md` | 版本更新史 |
| `LICENSE` | 许可证（EPL-2.0） |
| `.gitmodules` | 子模块声明 |
| `.gitignore` / `.gitattributes` | 忽略规则 / 行尾属性 |

## 3. 关键内容

### 3.1 `README.md`

- 项目名"云寸"，是 `get_weather` 的全新 C++ 重写版；版本信息见 `Release.md`。
- 功能：爬取中国天气网（weather.com.cn）数据，按当前 IP 定位城市后输出天气。
- 使用方式：
  - Release 直接运行（仅 Windows，建议 Win10+）；
  - 当作库使用：排除 `云寸.cpp`，Windows App SDK / UWP 可直接编译，其他平台需链接 cURL。
- 引用：cURL（爬虫）、colored-cout（彩色输出）。

> 注：源码为 UTF-8，某些环境按错误编码查看会出现乱码。

### 3.2 `Release.md`

版本更新史：

| 版本 | 日期 | 更新 |
| --- | --- | --- |
| 1.3.0 | 2024-06-08 | 增加预警功能 |
| 1.2.1 | 2024-05-12 | 增加 `weather` 类，分离业务，为 `yc_UI` 做技术准备 |
| 1.1.2 | 2024-03-29 | 增加异常处理、优化彩色输出与风向判断 |

测试平台：Windows 11 / Windows 10 19045.3803 / Windows 7 均可用（Win7 需管理员权限）。

### 3.3 `LICENSE`

EPL-2.0（与 `云寸.rc` 中 LegalCopyright 一致）。

### 3.4 `.gitmodules`（子模块）

| 路径 | 仓库 | 用途 |
| --- | --- | --- |
| `curl/` | https://github.com/curl/curl.git | libcurl 完整源码，提供控制台网络请求 |
| `colored-cout/` | https://github.com/yurablok/colored-cout.git | 彩色输出库（上游完整版） |

> 注意：`colored-cout/` 子模块内含自己的 `README.md`、`sample.png`、`UNLICENSE`，与根目录精简版 `colored_cout.h/.cpp` 并存，二者内容不同，维护时勿混淆。详见 `third_party.md`。

## 4. 主要接口

（文档类文件，无代码接口。）

## 5. 工作流程 / 调用关系

无代码调用关系；`.gitmodules` 在 `git submodule update --init` 时拉取 `curl/` 与 `colored-cout/`。

## 6. 依赖关系

- `README.md` / `Release.md` 描述项目整体；
- `.gitmodules` 关联第三方子模块（见 `third_party.md`）。

## 7. 注意事项 / 已知问题

- `.gitattributes` 设 `* text=auto`，但仓库内不同文件仍存在 CRLF/LF 差异（编辑时需保留各文件原有换行符）。
- 生成物（勿提交/可忽略）：`.vs/`（VS 缓存）、`RCa15300`（VS 临时文件）、`Debug/`、`Release/`、`x64/`（编译输出目录）。
