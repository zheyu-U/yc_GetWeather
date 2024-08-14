#pragma once
#ifndef _WEBPAGE_
#define _WEBPAGE_
#include <string>
#include "Tools.h"
#include "WeatherException.h"

class WebPage
{
public:

	/* 储存类型 */
	class ContentType {
	public:
		/* Constructors */
		 ContentType() : id{ count } { count++; }


		 /* Operators */
		bool operator==(const ContentType& type) const { return this->id == type.id; }	//判断ContentType静态常量是否相等
		

		/* Standard */
		static const WebPage::ContentType instant, forecast, location, warnings;	//ContentType--静态常量 声明


	private:

		short id;
		static inline short count = 0;

	};
	


	/* 文件格式 */
	class DocType 
	{
	public:
		/* Constructors */
		DocType() : id{ count } { count++; }

		/* Operators */
		bool operator==(const DocType& type) const { return this->id == type.id; }


		/* Standard */
		static const WebPage::DocType html, JavaScript;


	private:

		short id = 0;
		static inline short count = 0;

	};


	/* 编码格式 未实现 */
	class Encoding 
	{
	public:
		/* Constructors */
		Encoding() : id{ count } { count++; }


		/* Operators */
		bool operator==(const Encoding& type) const { return this->id == type.id; }	


		/* Standard */
		static const WebPage::Encoding utf8, gbk;


	private:

		short id;
		static inline short count = 0;
	}; 



//	void storeWebPage(WebPage* sourcePage);
	void readWebPage(std::string fileName, ContentType contentType, DocType doctype, std::string url, Encoding encoding = Encoding::utf8);
//	void makeWebPage(WebPage* Page, std::string source, ContentType pageType, DocType doctype, std::string url);
	
	void makeContent();
	void makeContent(std::string fileName);
	void freeContent();

	/* Accessors */
	std::string get_url() const { return url; };
	ContentType get_contenttype() const { return contenttype; }
	Encoding get_encoding() const { return encoding; }
	std::string* get_content_p();
	bool has_content() const { return has_content_; }

private:
	std::string fileName;
	std::string url;
	ContentType contenttype;
	DocType doctype;
	Encoding encoding;
	std::string content;
	bool has_content_  = 0;
};


//定义静态常量
//constexpr WebPage::ContentType instant, forecast, location, warnings;
#endif // !_WEBPAGE_

