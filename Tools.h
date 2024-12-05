#pragma once
#ifndef _TOOLS
#define _TOOLS

//警告 不得包含 WebPage.h

#ifdef _CONSOLE
#include "colored_cout.h"//https://github.com/yurablok/colored-cout/
#endif // _CONSOLE

#include "WeatherException.h"
#include <iostream>
#include <string>
#include <shlobj.h>//AppData
#include <filesystem>
#include <fstream>
namespace Tools
{
	static std::string DataLocation;//set in void CheckIfDataFileExists()

	static const int Err{ 10 }, Warn{ 11 }, Info{ 12 };
	std::string getRoamingAppDataPath();  //获取 data储存目录
	void readFileIntoString(std::string filename, std::string* str);
	void log_write(int lvl, std::string writelog);
	std::string Get_DataLocation();
	void CheckIfDataFileExists();
	std::string String_GetCurrentTime();

}

#endif // !_TOOLS