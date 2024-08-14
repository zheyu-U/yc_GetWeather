#include "WebPage.h"

/* Definations */

const WebPage::ContentType
WebPage::ContentType::instant,
WebPage::ContentType::forecast, 
WebPage::ContentType::location, 
WebPage::ContentType::warnings;	
//ContentType--静态常量 定义

const WebPage::DocType WebPage::DocType::html, WebPage::DocType::JavaScript;
//DocType--静态常量 定义

const WebPage::Encoding WebPage::Encoding::utf8, WebPage::Encoding::gbk;
//Encoding--静态常量 定义



void WebPage::readWebPage(std::string fileName, ContentType contentType, DocType doctype, std::string url, Encoding encoding)
{
	//encoding未实现
	makeContent(fileName);

	this->fileName = fileName;
	this->doctype = doctype;
	this->contenttype = contentType;
	this->encoding = encoding;
	this->url = url;

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


void WebPage::makeContent(std::string fileName)
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
