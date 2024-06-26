﻿#include <iostream>
#include <tchar.h>
#include <stdexcept>
#include "weather.h"
#include "colored_cout.h"//https://github.com/yurablok/colored-cout/



int main()
{
    weather Weather;
    SetConsoleOutputCP(CP_UTF8);                                         //PREPARATIONS
    system("chcp 65001");                                                //cout控制台中文乱码问题 在visual studio的项目属性 -> 配置属性 -> c/c++ -> 命令行 -> 其它选项 中填写 /utf-8  ref->https://learn.microsoft.com/zh-cn/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=msvc-170
    curl_global_init(CURL_GLOBAL_ALL);                                   //初始化cURL
    Weather.CheckIfDataFileExists();                                     //Check whether %AppData%/yc exists and create it.
    std::string StringCurrentTime = Weather.String_GetCurrentTime();     //gat time函数
    Weather.Store_Headers_Getdata(&Weather.Headers);                 //MAKE Headers()
    
 
    {//PART THREE : GET LOCATIONS
    std::string URL_GetLocationCode = "http://wgeo.weather.com.cn/ip/?_=" + StringCurrentTime;

    if (Weather.Get_CURL(URL_GetLocationCode, "Locatioin.txt", &Weather.Headers) == 0)
    {
        Weather.log_write(weathern::Err, "Failed to get location!");
        /*std::cout << clr::red << "[ERROR]" << clr::reset << "Failed to get location!\n";*/
        exit(1);
    }
    
    Weather.AnalyseWeatherFile_try("\\Locatioin.txt", weathern::_Location_, &Weather); 
    Weather.log_write(weathern::Info, "Location code:" + Weather.LocationCode + " Location:" + Weather.Location);
    }

    std::string URL_Content = "http://d1.weather.com.cn/sk_2d/" + Weather.LocationCode + ".html?_=" + StringCurrentTime;//MAKE CONTENT URL       
                    /*
                            http://d1.weather.com.cn/sk_2d/101270101.html?_=1706879114687
                                                                       ==>`<=== there is a '_'!
                     */

    Weather.log_write(weathern::Info, "Get content.URL is: " + URL_Content );
    bool is_sud = Weather.Get_CURL(URL_Content, Weather.LocationCode + "." + StringCurrentTime + ".txt", &Weather.Headers );
    if (!is_sud){
        Weather.log_write(weathern::Err, "Failed to get!");
        exit(1);
    }

    {
        std::string URL_Warnings = "http://d1.weather.com.cn/dingzhi/" + Weather.LocationCode + ".html?_=" + StringCurrentTime;//MAKE CONTENT URL       
        /*
                http://d1.weather.com.cn/dingzhi/101270101.html?_=1717853928121
                                                             ==>`<=== there is a '_'!
         */

        Weather.log_write(weathern::Info, "Get warnings.URL is: " + URL_Warnings);
        bool is_sud = Weather.Get_CURL(URL_Warnings, "w" + Weather.LocationCode + "." + StringCurrentTime + ".txt", &Weather.Headers);
        if (!is_sud) {
            Weather.log_write(weathern::Err, "Failed to get!");
            exit(1);
        }
    }
    
    //ANALYSE AND PRINT
    Weather.AnalyseWeatherFile_try("\\" + Weather.LocationCode + "." + StringCurrentTime + ".txt", weathern::_Content_, &Weather);
    Weather.AnalyseWeatherFile_try("\\w" + Weather.LocationCode + "." + StringCurrentTime + ".txt", weathern::_Warnings_, &Weather);
    
    {
        system("pause");
        system("cls");
        std::cout << "更新日期:" << clr::white << Weather.CurrentDate << " " << Weather.UpdateTime << clr::reset << std::endl;
        std::cout << "城市:" << clr::green <<Weather.Location << clr::reset << "\n实时精准气温:" << clr::green << Weather.CurrentTem << "℃    " << clr::reset;
        std::cout << "实时天气:" << clr::green << Weather.CurrentWeather << clr::reset << std::endl;
        if (Weather.CurrentWind_speed == "0级")
        {
            Weather.CurrentWind_Direction = "无风";
        }
        std::cout << "风向:" << clr::green << Weather.CurrentWind_Direction << clr::reset << "  风速:" << clr::green << Weather.CurrentWind_speed << clr::reset << std::endl;
        std::cout << "AQI指数:";
        if (std::stoi(Weather.CurrentAQI) <= 50)std::cout << clr::green << Weather.CurrentAQI << " 优" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) <= 100)std::cout << clr::yellow << Weather.CurrentAQI << " 良" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) <= 150)std::cout << clr::magenta << Weather.CurrentAQI << " 轻度污染" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) <= 200)std::cout << clr::red << Weather.CurrentAQI << " 中度污染" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) <= 300)std::cout << clr::on_magenta << Weather.CurrentAQI << " 重度污染" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) > 300)std::cout << clr::on_red << Weather.CurrentAQI << " 严重污染" << clr::reset << "\n\n";
        if (!Weather.Warnings.empty()) std::cout << clr::white << Weather.Warnings << clr::reset << std::endl << std::endl;
        std::cout << "更多请访问 " << clr::cyan << "http://www.weather.com.cn/\n" << clr::reset;
    }
    system("pause");
    return 0;
}











// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
