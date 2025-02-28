#pragma once
#ifndef WEATHER_EXCEPTION_
#define WEATHER_EXCEPTION_

#include <exception>
#include <string>

namespace yc::ycresult {

	/* 异常类型 */
	enum class ExcptType { content_too_long, content_not_found, invalid_formatting, curl, file };	//异常--静态常量 声明

	/* 处理方式 */
	enum class AddressMethod
	{
		normal,        //正常情况
		implicit,      //记录异常但不显示  !_debug
		back,          //退出当前步骤并提示
		crash          //崩溃
	};

	class ycresult : public std::exception
	{
	public:

		/* Constructors */

		//const ExcptType , const AddressMethod , const std::string_view
		explicit ycresult(const ExcptType type, const AddressMethod method, const std::string_view message = "We've been trapped in trouble! ")
		{
			this->method = method;
			this->message = message;
			this->type = type;
		}


		/* Accessors */
		const char* what() const noexcept override {
			return message.c_str();
		}


		AddressMethod get_AddressMethod() const noexcept {
			return method;
		}

	private:

		std::string message;
		ExcptType type;
		AddressMethod method;

	};
}

#endif // !WEATHER_EXCEPTION_
