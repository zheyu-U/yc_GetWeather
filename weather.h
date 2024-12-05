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
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <windows.h>

class Weather
{
public:
	Weather() :Time{ Tools::String_GetCurrentTime() } { setHeaders(); };
	Weather(WebPage::ContentType ct, std::string location, std::string locationCode) : contentType{ ct }, Location{ location }, LocationCode{ locationCode }, Time{ Tools::String_GetCurrentTime() } { setHeaders(); };
	Weather(WebPage::ContentType ct) :contentType{ ct }, Time{ Tools::String_GetCurrentTime() } { setHeaders(); };

	WebPage::ContentType contentType;
	std::string Location, LocationCode, PublicIP, CurrentTem, MaxTem, MinTem, UpdateTime, CurrentDate, CurrentWeekday, CurrentAQI, CurrentWind_Direction, CurrentWind_speed, CurrentWeather, CurrentRainfall, Rainfall24, Warnings;
	std::vector<std::string> Headers;


	void analysis(class WebPage* page);
	bool get_CURL(std::string url_cURL, std::string _FName);
	bool get_HttpClient(std::string url_cURL, std::string *res);
	bool get_weather(WebPage::ContentType type, WebPage* page);
	bool get_weather_kit(WebPage::ContentType type);

	void setupTime() { Time = Tools::String_GetCurrentTime(); }

	/* 位置 */
	class Position
	{
	public:

		Position() = default;
		Position(std::string lct, std::string cd) { location = lct; code = cd; }

		static void setPosition(std::string m_location, std::string m_code) { location = m_location; code = m_code; }
		static std::string getPosition() { return location; }
		static std::string getPositionCode() { return code; }

	private:

		static std::string location, code;

	};

private:

	void setHeaders();  //构造时自动设置
	bool ReadQuoteContent(std::string* FromWhat, std::string What, std::string* ToWhat);

	std::string Time;
};

#endif // !_WEATHER_H_