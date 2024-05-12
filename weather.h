#pragma once
#ifndef _WEATHER_H_
#define _WEATHER_H_

#include "colored_cout.h"//https://github.com/yurablok/colored-cout/
#include "curl/curl.h"//curl https://blog.csdn.net/weixin_44122235/article/details/128969128
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <shlobj.h>//AppData
#include <windows.h>

namespace weathern {
    const int _Headers_{ 30 }, _Location_{ 31 }, _Content_{ 32 };             //get函数methods
}

class weather
{
public:
        std::string Location, LocationCode, PublicIP, CurrentTem, Maxtem, MinTem, UpdateTime, CurrentDate, CurrentWeekday, CurrentAQI, CurrentWind_Direction, CurrentWind_speed, CurrentWeather, CurrentRainful, Rainful24, Warnings;
        std::vector<std::string> Headers;
        
        std::string Get_DataLocation();
        void CheckIfDataFileExists();
        std::string String_GetCurrentTime();
        void AnalyseWeatherFile(std::string FileName, int GetMethod, weather* weather_store);
        std::string getRoamingAppDataPath();
        std::string WCHAR2String(LPCWSTR pwszSrc);
        std::string readFileIntoString(std::string filename);
        bool ReadQuoteContent(std::string* FromWhat, std::string What, std::string* ToWhat);
        bool Get_CURL(std::string url_cURL, std::string _FName, std::vector<std::string>* Headers_fun);
        void Store_Headers_Getdata(std::vector<std::string>* Headers_Getdata_Fun);
        
private:
        std::string DataLocation;//set in void CheckIfDataFileExists()
};

#endif // !_WEATHER_H_