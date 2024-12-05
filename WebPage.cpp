#include "pch.h"
#include "WebPage.h"

/* Definations */

//const WebPage::ContentType
//WebPage::ContentType::instant,
//WebPage::ContentType::forecast,
//WebPage::ContentType::location,
//WebPage::ContentType::warnings;
//ContentType--静态常量 定义

const WebPage::DocType WebPage::DocType::html, WebPage::DocType::JavaScript;
//DocType--静态常量 定义

const WebPage::Encoding WebPage::Encoding::utf8, WebPage::Encoding::gbk;
//Encoding--静态常量 定义



void WebPage::readWebPage(std::string _fileName, ContentType _contentType, DocType _doctype, std::string _url, Encoding _encoding)
{
	//encoding未实现
	makeContent(_fileName);

	this->fileName = _fileName;
	this->doctype = _doctype;
	this->contenttype = _contentType;
	this->encoding = _encoding;
	this->url = _url;

}

void WebPage::makeWebPage(std::string* source, ContentType pageType, DocType _doctype, std::string _url, Encoding _encoding)
{
	this->content = *source;        // copy string!
	this->contenttype = pageType;
	this->doctype = _doctype;
	this->encoding = _encoding;

	has_content_ = 1;
}

void WebPage::makeContent()
{
	std::string Path = Tools::Get_DataLocation() + "\\" + fileName;
	try
	{
		Tools::readFileIntoString(Path, &(this->content));//read and store in WeatherSource
	}
	catch (const WeatherException& excpt)
	{
		throw excpt;
	}

	has_content_ = 1;

}


void WebPage::makeContent(std::string _fileName)
{
	std::string Path = Tools::Get_DataLocation() + "\\" + _fileName;
	try
	{
		Tools::readFileIntoString(Path, &(this->content));//read and store in WeatherSource
	}
	catch (const WeatherException& excpt)
	{
		throw excpt;
	}

	this->fileName = fileName;
	has_content_ = 1;

}

void WebPage::freeContent()
{
	content.clear();
	has_content_ = 0;
}

std::string* WebPage::get_content_p()
{
	if (has_content_)
	{
		return &content;
	}
	else
	{
		makeContent();
		return &content;
	}

}
