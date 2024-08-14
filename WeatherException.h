#pragma once
#ifndef WEATHER_EXCEPTION_
#define WEATHER_EXCEPTION_

#include <exception>
#include <string>

class WeatherException : std::exception
{
public:
	class ExcptType;
	class AddressMethod;


	/* Constructors */

	//const ExcptType , const AddressMethod , const std::string_view
	explicit WeatherException(const ExcptType type, const AddressMethod method, const std::string_view message = "We've been trapped in trouble! ")
	{
		this->method = method;
		this->message = message;
		this->type = type;
	}


	/* Accessors */
	const char* what() const noexcept override {
		return message.c_str();
	}

	const char* ExcptTypeExplanation() const noexcept {
		return type.view.c_str();
	}

	const std::string errView() const noexcept;

	AddressMethod get_AddressMethod() const noexcept {
		return method;
	}



	/* 异常类型 */
	class ExcptType {
	public:


		/* Constructors */
		ExcptType(std::string defaultView) : id{ count }, view{ defaultView } { count++; };
		ExcptType() = default;


		/* Operators */
		bool operator==(const ExcptType& type) const { return this->id == type.id; }	//判断ExcptType静态常量是否相等


		/* Standard */
		static const ExcptType content_too_long, content_not_found, invalid_formatting, curl, file;	//异常--静态常量 声明


		std::string view;


	private:
		short id = 0;	//识别ExcptType静态常量
		static inline short count = 0;
	};
	// 异常类型 END



	/* 处理方式 */
	class AddressMethod
	{
	public:
		/* Constructors */
		AddressMethod() : id{ count } { count++; }


		/* Operators */
		bool operator==(const AddressMethod& type) const { return this->id == type.id; }	//判断AddressMethod静态常量是否相等


		/* Standard */
		static const AddressMethod
			normal,        //正常情况
			implicit,      //记录异常但不显示  !_debug
			back,          //退出当前步骤并提示
			crash;         //崩溃


	private:
		short id = 0;	   //识别AddressMethod静态常量
		static inline short count = 0;

	};
	// 处理方式 END




private:

	std::string message;
	ExcptType type;
	AddressMethod method;

};

#endif // !WEATHER_EXCEPTION_
