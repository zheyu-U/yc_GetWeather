#pragma once
#ifndef _TOOLS
#define _TOOLS

//警告 不得包含 WebPage.h

#define YC_GETWEATHER_CORE_VERSION  "1.3.6.0"

#ifdef _CONSOLE
#include "colored_cout.h"    //https://github.com/yurablok/colored-cout/
#endif // _CONSOLE

#include "WeatherException.h"
#include <iostream>
#include <string>
#include <shlobj.h>               //AppData
#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>
#include <iomanip>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <initializer_list>
#include <vector>

namespace Tools
{
	static std::string DataLocation;
	// path to store data. set in void CheckIfDataFileExists()
	// 必须使用 Get_DataLocation()获取。
	// e.g. C:\Users\username\AppData\Roaming\yc   末尾不带斜杠

	static const int Err{ 10 }, Warn{ 11 }, Info{ 12 }, FErr{ 13 };


	std::string getRoamingAppDataPath();                                           // 获取 data储存目录
	void readFileIntoString(std::string filename, std::string* str);        // 读取文件
	std::string Get_DataLocation();                                                        // 返回 DataLocation
	void CheckIfDataFileExists();                                                            // 检查运行数据文件夹是否存在并创建它。应在程序开始时调用。在这里设置DataLocation
	std::string String_GetCurrentTime();


	// ========== 日志系统 ==========
	// 
	class Detail
	{
	private:
		std::string name, key;
	public:
		Detail(std::string n, std::string k) : name{ n }, key{ k } {}
		std::string getName() const { return name; }
		std::string getKey() const { return key; }
	};

	class InfoSet
	{
	public:
		InfoSet(std::initializer_list<Detail> info);
		~InfoSet();

		// 返回单行格式的 info 内容，例如: "name1:key1; name2:key2"
		std::string ToString() const;
		std::string GetEachString(int len) const noexcept;
		size_t size() const noexcept { return details_.size(); }

	private:
		std::vector<Detail> details_;

	};

	void log_write(int lvl, std::string writelog, std::string region, InfoSet info, std::vector<std::string> call);
	void log_write(int lvl, std::string writelog, std::string region, InfoSet info);
	void log_write(int lvl, std::string writelog,std::string region);
	//void __IN_log_write(int lvl, std::string writelog);
	// log_write 在开发中

	// ========== 异步日志文件持久化 ==========
	// 启动异步日志系统：创建 %AppData%/yc/log/ 目录，按启动时间生成日志文件，
	// 并启动后台刷写线程。应在 CheckIfDataFileExists() 之后调用。
	void InitLogSystem();

	// 停止异步日志系统：通知后台线程退出，写入结束标记，等待最终刷写完成并关闭文件。
	// 应在程序退出前（或 return 前）调用。
	// 若未调用 InitLogSystem() 则此函数为空操作。
	void ShutdownLogSystem();
}

#endif // !_TOOLS