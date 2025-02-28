#include "pch.h"
#include "WebPage.h"


namespace yc::WebPage {
	
	void WebPage::read(std::string _fileName, Type _contentType, DocType _doctype, std::string _url, Encoding _encoding)
	{
		//encoding未实现
		makeContent(_fileName);

		this->fileName = _fileName;
		this->doctype = _doctype;
		this->contenttype = _contentType;
		this->encoding = _encoding;
		this->url = _url;

		has_content_ = 1;
	}

	void WebPage::make(std::string* source, Type pageType, DocType _doctype, std::string _url, Encoding _encoding)
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
		catch (const ycresult::ycresult& excpt)
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
		catch (const ycresult::ycresult& excpt)
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
}