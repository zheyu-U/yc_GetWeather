#pragma once
#ifndef _WEATHER_H_
#define _WEATHER_H_

#ifdef _CONSOLE
#include "curl/curl.h"//curl https://blog.csdn.net/weixin_44122235/article/details/128969128
#endif // _CONSOLE

#include "WebPage.h"
#include "Tools.h"
#include "WeatherException.h"
#include <chrono>
#include <format>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <windows.h>
#include <expected>

namespace yc {

	class Weather
	{
	public:
		Weather() :Time{ Tools::String_GetCurrentTime() } { setHeaders(); };
		Weather(Type ct, std::string location, std::string locationCode) : contentType{ ct }, Location{ location }, LocationCode{ locationCode }, Time{ Tools::String_GetCurrentTime() } { setHeaders(); };
		Weather(Type ct) :contentType{ ct }, Time{ Tools::String_GetCurrentTime() } { setHeaders(); };

		Type contentType;
		std::string Location, LocationCode, PublicIP, CurrentTem, MaxTem, MinTem, UpdateTime, CurrentDate, CurrentWeekday, CurrentAQI, CurrentWind_Direction, CurrentWind_speed, CurrentWeather, CurrentRainfall, Rainfall24, Warnings;
		std::vector<std::string> Headers;


		void analysis(WebPage::WebPage* page);
		std::expected<bool, Tools::SimpleError> get_CURL(std::string url_cURL, std::string _FName) noexcept;   // 规定 10 文件错误，20 网络错误，40 分析错误
		std::expected<bool, Tools::SimpleError> get_HttpClient(std::string url_cURL, std::string* res) noexcept;
		std::expected<bool, Tools::SimpleError> get_weather(Type type, WebPage::WebPage* page) noexcept ; // 规定 -10 为调用错误，其余同上
		std::expected<bool, Tools::SimpleError> get_weather_kit(Type type) noexcept;

		void setupTime() { Time = Tools::String_GetCurrentTime(); }

		/* 
			Accessors for base URLs. This allows tailored modification of the base URLs. 
			 仍在开发中，因为在weather.cpp里面，URL还要被拼接。定制URL的功能暂时不完善。  2026-10-22记
		*/
		std::string get_URL_Get_1d_Weather() { return URL_Get_1d_Weather_Base; }
		void set_URL_Get_1d_Weather(std::string url) { URL_Get_1d_Weather_Base = url; }
		std::string get_URL_Get_Instant_Weather() { return URL_Get_Instant_Weather_Base; }
		void set_URL_Get_Instant_Weather(std::string url) { URL_Get_Instant_Weather_Base = url; }
		std::string get_URL_Get_LocationCode() { return URL_Get_LocationCode_Base; }
		void set_URL_Get_LocationCode(std::string url) { URL_Get_LocationCode_Base = url; }
		std::string get_URL_Get_Warnings() { return URL_Get_Warnings_Base; }
		void set_URL_Get_Warnings(std::string url) { URL_Get_Warnings_Base = url; }

	private:

		void setHeaders();  //构造时自动设置
		bool ReadQuoteContent(std::string* FromWhat, std::string What, std::string* ToWhat);

		std::string Time;


		std::string URL_Get_1d_Weather_Base = "https://www.weather.com.cn/weather1d/101200109.shtml";
		std::string URL_Get_Instant_Weather_Base = "http://d1.weather.com.cn/sk_2d/";
		std::string URL_Get_LocationCode_Base = "http://wgeo.weather.com.cn/ip/?_=";
		std::string URL_Get_Warnings_Base = "http://d1.weather.com.cn/dingzhi/";
		// at observe24h_data
	};



	/* 位置 */
	namespace Position {

		//static std::string location, code;
		static std::string location = "ERR", code = "ERR";


		static void Position(std::string lct, std::string cd) { location = lct; code = cd; }

		static void setPosition(std::string m_location, std::string m_code) { location = m_location; code = m_code; }
		static std::string getPosition() { return location; }
		static std::string getPositionCode() { return code; }
	}
}
#endif // !_WEATHER_H_