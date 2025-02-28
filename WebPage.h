#pragma once
#ifndef _WEBPAGE_
#define _WEBPAGE_
#include <string>
#include "Tools.h"
#include "WeatherException.h"


namespace yc {

	/* 储存类型 */
	enum class Type { instant, forecast, location, warnings };

	namespace WebPage {

		/* 文件格式 */
		enum class DocType { html, JavaScript };

		/* 编码格式 未实现 */
		enum class Encoding { utf8, gbk };

		class WebPage
		{
		public:

			
			//	void storeWebPage(WebPage* sourcePage);
			void read(std::string fileName, Type contentType, DocType doctype, std::string url, Encoding encoding = Encoding::utf8);
			void make(std::string* source, Type pageType, DocType doctype, std::string url, Encoding _encoding = Encoding::utf8);

			void freeContent();

			// 只应在 freeContent() 后，需要内容时调用
			void makeContent();
			void makeContent(std::string fileName);

			/* Accessors */
			std::string get_url() const { return url; };
			Type get_contenttype() const { return contenttype; }
			Encoding get_encoding() const { return encoding; }
			std::string* get_content_p();
			bool has_content() const { return has_content_; }

		private:
			

			std::string fileName;
			std::string url;
			Type contenttype;
			DocType doctype;
			Encoding encoding;
			std::string content;
			bool has_content_ = 0;
		};

		// 在类外提供生成WebPage的接口
		//	void storeWebPage(WebPage* sourcePage);
		static void readWebPage(WebPage& webpage, std::string fileName, Type contentType, DocType doctype, std::string url, Encoding encoding = Encoding::utf8) {
			webpage.read(fileName, contentType, doctype, url, encoding);
		}
		static void makeWebPage(WebPage webpage, std::string* source, Type pageType, DocType doctype, std::string url, Encoding _encoding = Encoding::utf8) {
			webpage.make(source, pageType, doctype, url, _encoding);
		}

	}
}
//定义静态常量
//constexpr WebPage::Type instant, forecast, location, warnings;
#endif // !_WEBPAGE_

