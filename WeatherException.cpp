#include "WeatherException.h"

const WeatherException::ExcptType
WeatherException::ExcptType::content_too_long{ "The content is out of length." },
WeatherException::ExcptType::content_not_found{ "Cannot find the required content." },
WeatherException::ExcptType::file{ "file error." },
WeatherException::ExcptType::curl{ "curl error." },
WeatherException::ExcptType::invalid_formatting{ "Unknown formatting" };
//异常类型--静态常量 定义


const WeatherException::AddressMethod
WeatherException::AddressMethod::normal,        //正常情况
WeatherException::AddressMethod::implicit,      //记录异常但不显示  !_debug
WeatherException::AddressMethod::back,          //退出当前步骤并提示
WeatherException::AddressMethod::crash;         //崩溃
//异常处理方式--静态常量 定义

const std::string WeatherException::errView() const noexcept
{
	return "WeatherException:\nExcptType:"
		+ std::string(ExcptTypeExplanation())
		+ "\nError message:"
		+ std::string(what());
}
