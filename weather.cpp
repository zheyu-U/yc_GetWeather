#include "pch.h"
#include "weather.h"

using namespace Tools;


namespace yc {

	// 会抛出异常
	void Weather::analysis(WebPage::WebPage* page)
	{
		using ET = ycresult::ExcptType;
		using AM = ycresult::AddressMethod;

		Type GetMethod = page->get_contenttype();

		//分析开始
		std::string* WeatherSource = page->get_content_p();

		if ((*WeatherSource)[0] != 'v' || WeatherSource->empty()) throw ycresult::ycresult(ET::invalid_formatting, AM::back, "Formatting changed or wrong headers.");

		//size_t SourceLength = WeatherSource->size();

		this->contentType = GetMethod;
		if (GetMethod == Type::instant)
		{
			if (!ReadQuoteContent(WeatherSource, "cityname", &this->Location)) throw ycresult::ycresult(ET::content_not_found, AM::back, "cityname");
			if (!ReadQuoteContent(WeatherSource, "temp\"", &this->CurrentTem)) throw ycresult::ycresult(ET::content_not_found, AM::back, "temp\"");
			if (!ReadQuoteContent(WeatherSource, "WD", &this->CurrentWind_Direction)) throw ycresult::ycresult(ET::content_not_found, AM::back, "WD");
			if (!ReadQuoteContent(WeatherSource, "WS", &this->CurrentWind_speed)) throw ycresult::ycresult(ET::content_not_found, AM::back, "WS");
			if (!ReadQuoteContent(WeatherSource, "weather\"", &this->CurrentWeather)) throw ycresult::ycresult(ET::content_not_found, AM::back, "weather\"");
			if (!ReadQuoteContent(WeatherSource, "aqi\"", &this->CurrentAQI)) throw ycresult::ycresult(ET::content_not_found, AM::back, "aqi\"");
			if (!ReadQuoteContent(WeatherSource, "time\"", &this->UpdateTime)) throw ycresult::ycresult(ET::content_not_found, AM::back, "time\"");
			if (!ReadQuoteContent(WeatherSource, "date\"", &this->CurrentDate)) throw ycresult::ycresult(ET::content_not_found, AM::back, "date\"");
			
			if (CurrentWind_speed == "0级")
			{
				CurrentWind_Direction = "无风";
			}
		}
		if (GetMethod == Type::location)
		{
			if (!ReadQuoteContent(WeatherSource, "ip", &this->PublicIP)) throw ycresult::ycresult(ET::content_not_found, AM::back, "ip");
			if (!ReadQuoteContent(WeatherSource, "id", &this->LocationCode)) throw ycresult::ycresult(ET::content_not_found, AM::back, "id");
			if (!ReadQuoteContent(WeatherSource, "addr", &this->Location)) throw ycresult::ycresult(ET::content_not_found, AM::back, "addr");
			Position::setPosition(this->Location, this->LocationCode);              //set for later use
			Tools::log_write(Tools::Info, "Location code:" + Position::getPositionCode() + " Location:" + Position::getPosition(), "Weather::analysis");
		}
		if (GetMethod == Type::warnings)
		{
			std::string rawWarnings;
			if (!ReadQuoteContent(WeatherSource, "\"w\"", &rawWarnings)) throw ycresult::ycresult(ET::content_not_found, AM::back, "\"w\"");
			if (rawWarnings.empty()) {
				this->Warnings = "";
				log_write(Info, "No warning found.", "Weather::analysis");
			}
			else {
				if (!ReadQuoteContent(&rawWarnings, "w9", &this->Warnings)) throw ycresult::ycresult(ET::content_not_found, AM::back, "w9");
			}
		}
	}




	//int* FromWhere, std::string* FromWhat, std::string What, std::string* ToWhat
	//FromWhat开始的地方  FromWhat（地址）     查找相符的那个字符串     接收找到的字符串
	//查找"abc":"str" 输入 what(是 abc) 输出ToWhat (是 str)
	//原理：数引号
	bool Weather::ReadQuoteContent(std::string* FromWhat, std::string What, std::string* ToWhat)
	{
		size_t text_start = (*FromWhat).find(What);
		if (text_start == std::string::npos) return FALSE;
		size_t equal_mark = (*FromWhat).find_first_of(":=", text_start);
		if (equal_mark == std::string::npos) return FALSE;
		size_t value_start = (*FromWhat).find_first_of("\"[{", equal_mark) + 1;
		size_t value_end = std::string::npos;
		if ((*FromWhat)[value_start - 1] == '{') value_end = (*FromWhat).find_first_of("}", value_start);
		else if ((*FromWhat)[value_start - 1] == '\"') value_end = (*FromWhat).find_first_of("\"", value_start);
		else value_end = (*FromWhat).find_first_of("]", value_start);
		if (value_start == std::string::npos || value_end == std::string::npos) return FALSE;
		//std::cout << text_start << " :" << equal_mark << " :" << value_start << " :" << value_end << std::endl;
		(*ToWhat).clear();
		(*ToWhat).append((*FromWhat), value_start, value_end - value_start);
		return true;
	}

	//docs about CURL
	//https://blog.csdn.net/MOU_IT/article/details/96457666
	//https://everything.curl.dev/examples/get
	//URL ,file name, headers
	bool Weather::get_CURL(std::string url_cURL, std::string _FileName)
	{

		std::string Path = Get_DataLocation() + "\\" + _FileName;
		//get file path

		FILE* file;
		bool fileOpen_code = fopen_s(&file, Path.c_str(), "w");
		if (file == NULL || fileOpen_code != 0)
		{
			log_write(Err, "Create file (" + Path + ") failed. At function \"get_CURL\" when preparing to store Weather source", "Weather::get_CURL");
			return FALSE;
		}
		//prepare to store weather data


		// 控制台使用
#ifdef _CONSOLE


		CURL* curl = curl_easy_init();

		struct curl_slist* headers = NULL;
		size_t headers_size = this->Headers.size();
		if (headers_size != 0) {
			for (size_t headers_i = 0; headers_i < headers_size; headers_i += 2) {
				headers = curl_slist_append(
					headers,
					(this->Headers[headers_i] + ':' + this->Headers[headers_i + 1]).c_str()
				);

				log_write(Info, "Headers -> " + this->Headers[headers_i] + ':' + this->Headers[headers_i + 1], "Weather::get_CURL");
			}
		}
		//set headers

		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url_cURL.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

			CURLcode res_code = curl_easy_perform(curl);

			char* LocalIP, * TargetIP;

			CURLcode res_IP_L = curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &LocalIP);
			CURLcode res_IP_T = curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &TargetIP);//get IP
			if (res_code == CURLE_OK)
			{
				std::string LocalIP_s{}, TargetIP_s{};
				LocalIP_s.append(LocalIP); TargetIP_s.append(TargetIP);
				if (res_IP_L == 0) log_write(Info, "Local IP is: " + LocalIP_s, "Weather::get_CURL");
				if (res_IP_T == 0) log_write(Info, "Target IP is: " + TargetIP_s, "Weather::get_CURL");//get IP

				curl_slist_free_all(headers);
				curl_easy_cleanup(curl);
				fclose(file);
				/* Always clean up ! */
				return TRUE;
			}
			else
			{
				//curl failed!
				log_write(Warn, "\"curl_easy_perform()\"  failed.  More info below.", "Weather::get_CURL", {
					{"URL",  url_cURL },
					{ "cURL",  std::string(curl_easy_strerror(res_code)) }
					});
				//log_write(Warn, "...more from curl: " + std::string(curl_easy_strerror(res_code)), "Weather::get_CURL");
				using WE = ycresult::ExcptType;
				using WA = ycresult::AddressMethod;
				ycresult::ycresult e(WE::curl, WA::back, "Network error.\n" + std::string(curl_easy_strerror(res_code)));
				curl_slist_free_all(headers);
				curl_easy_cleanup(curl);
				fclose(file);
				// throw e;                     //  作为一个有返回值的函数，不能抛出异常。并且异常可能导致内存泄露、出现没有返回值等情况，导致外部引用出出现不确定bug。
				return false;
			}
		}
		else
		{
			log_write(Err, "Curl setup error. Please try again.", "Weather::get_CURL");
		}

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		fclose(file);
		/* Always clean up ! */

#endif // _CONSOLE

		return false;
	}

	bool Weather::get_HttpClient(std::string url_cURL, std::string* res)
	{

#ifdef DeskTop_yc
#include <winrt/Windows.Web.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
		using namespace winrt::Windows::Web::Http;

		HttpClient client;

		Headers::HttpRequestHeaderCollection headers = client.DefaultRequestHeaders();

		size_t headers_size = this->Headers.size();
		if (headers_size != 0) {
			for (size_t headers_i = 0; headers_i < headers_size; headers_i += 2) {

				headers.Append(
							winrt::to_hstring((this->Headers)[headers_i]),
							winrt::to_hstring((this->Headers)[headers_i + 1])
						);   // are supposed to use try-catch

						log_write(Info, "Headers -> " + this->Headers[headers_i] + ':' + this->Headers[headers_i + 1], "Weather::get_HttpClient");
			}
		}


		HttpResponseMessage resMsg;


		try
		{
			resMsg = client.GetAsync(
				winrt::Windows::Foundation::Uri(
					winrt::to_hstring(url_cURL)
				)
			).get();

			resMsg.EnsureSuccessStatusCode();
			*res = winrt::to_string(resMsg.Content().ReadAsStringAsync().get());
		}
		catch (winrt::hresult_error const& ex)
		{
			*res = winrt::to_string(ex.message());
		}

		if (resMsg.IsSuccessStatusCode()) return true;
		else {
			log_write(Tools::Warn, "\"get_HttpClient()\"  failed. URL: " + url_cURL + "\nResponse ReasonPhrase:" + winrt::to_string(resMsg.ReasonPhrase()), "Weather::get_HttpClient");
			/*
			throw ycresult(
				ycresult::ExcptType::curl,
				ycresult::AddressMethod::back,
				"Response ReasonPhrase:" + winrt::to_string(resMsg.ReasonPhrase())
			);
			*/                            //  作为一个有返回值的函数，不能抛出异常。并且异常可能导致内存泄露、出现没有返回值等情况，导致外部引用出出现不确定bug。
			return false;
		}

#endif // DeskTop_yc

		return false;
	}




	bool Weather::get_weather(Type type, WebPage::WebPage* page)
	{
		if (!page) return false;         //nullptr

		using CT = Type;


		/* ==================
				location
		   ================== */
		if (type == CT::location)
		{

			Tools::log_write(Tools::Info, "Try getting location.", "Weather::get_weather");

			std::string URL_GetLocationCode = "http://wgeo.weather.com.cn/ip/?_=" + Time;

			bool res1 = 0;
			std::string resContent;

			try
			{
#ifdef _CONSOLE
				res1 = get_CURL(URL_GetLocationCode, "Location.txt");
#endif // _CONSOLE

#ifdef DeskTop_yc
				res1 = get_HttpClient(URL_GetLocationCode, &resContent);
#endif // DeskTop_yc
			}
			catch (const ycresult::ycresult& e)
			{
				// needed to deal with. needed nomal handler of ycresult::ycresults
			}




			// get

			if (!res1)
			{
				Tools::log_write(Tools::Warn, "Failed to get location!", "Weather::get_weather");
			}
			else
			{

#ifdef _CONSOLE
				page->read("Location.txt",
					Type::location,
					WebPage::DocType::JavaScript,
					URL_GetLocationCode);
#endif // _CONSOLE

#ifdef DeskTop_yc
				page->make(&resContent,
					Type::location,
					WebPage::DocType::JavaScript,
					URL_GetLocationCode);
#endif // DeskTop_yc

				// make page


				return true;
			}
		}


		/* ==================
		current weather infomation
		   ================== */
		if (type == CT::instant)
		{

			Tools::log_write(Tools::Info, "Try getting weather infomation.", "Weather::get_weather");

			std::string URL_Content = "http://d1.weather.com.cn/sk_2d/" + Position::getPositionCode() + ".html?_=" + Time;     //MAKE URL

			Tools::log_write(Tools::Info, "Get content. URL is: " + URL_Content, "Weather::get_weather");

			std::string* c_fileName = new std::string;
			*c_fileName = Position::getPositionCode() + "_" + Time + ".txt";


			bool res2 = 0;
			std::string resContent;

			try
			{
#ifdef _CONSOLE
				res2 = get_CURL(URL_Content, *c_fileName);
#endif // _CONSOLE

#ifdef DeskTop_yc
				res2 = get_HttpClient(URL_Content, &resContent);
#endif // DeskTop_yc
				// get
			}
			catch (const ycresult::ycresult& e)
			{
				// needed to deal with. needed nomal handler of ycresult::ycresults
			}


			if (!res2) {
				Tools::log_write(Tools::Warn, "Failed to get weather infomation!", "Weather::get_weather");
			}
			else
			{
#ifdef _CONSOLE
				page->read(*c_fileName,
					Type::instant,
					WebPage::DocType::JavaScript,
					URL_Content);
#endif // _CONSOLE

#ifdef DeskTop_yc
				page->make(&resContent,
					Type::instant,
					WebPage::DocType::JavaScript,
					URL_Content);
#endif // DeskTop_yc


				Tools::log_write(Tools::Info, "Getting weather information succeeds.", "Weather::get_weather");
				return true;
			}

		}


		/* ==================
			   get warnings
		   ================== */
		if (type == CT::warnings)
		{
			/* ==    get warnings    == */

			Tools::log_write(Tools::Info, "Try getting warnings.", "Weather::get_weather");

			std::string URL_Warnings = "http://d1.weather.com.cn/dingzhi/" + Position::getPositionCode() + ".html?_=" + Time;             //MAKE URL       
			std::string* w_fileName = new std::string;
			*w_fileName = "warnings_" + Position::getPositionCode() + "_" + Time + ".txt";

			Tools::log_write(Tools::Info, "Get warnings. URL is: " + URL_Warnings, "Weather::get_weather");

			bool res3 = 0;
			std::string resContent;

			try
			{
#ifdef _CONSOLE
				res3 = get_CURL(URL_Warnings, *w_fileName);
#endif

#ifdef DeskTop_yc
				res3 = get_HttpClient(URL_Warnings, &resContent);
#endif
			}
			catch (const ycresult::ycresult& e)
			{
				// needed to deal with. needed nomal handler of ycresult::ycresults
			}


			if (!res3) {
				Tools::log_write(Tools::Warn, "Failed to get warnings!", "Weather::get_weather");
			}
			else
			{
#ifdef _CONSOLE
				page->read(*w_fileName,
					Type::warnings,
					WebPage::DocType::JavaScript,
					URL_Warnings);
#endif // _CONSOLE

#ifdef DeskTop_yc
				page->make(&resContent,
					Type::warnings,
					WebPage::DocType::JavaScript,
					URL_Warnings);
#endif // DeskTop_yc


				Tools::log_write(Tools::Info, "Getting warnings succeeds.", "Weather::get_weather");
				return true;
			}
		}

		Tools::log_write(Tools::Warn, "\"get_weather\" failed.", "Weather::get_weather");
		return false;
	}

	bool Weather::get_weather_kit(Type type)
	{
		// std::unique_ptr<WebPage::WebPage> pg = std::make_unique<WebPage::WebPage>(WebPage::WebPage());
		WebPage::WebPage* pg = new WebPage::WebPage;
		bool res = get_weather(type, pg);
		if (res)
		{

			try
			{
				analysis(pg);
			}
			catch (const ycresult::ycresult& e)
			{
				Tools::log_write(Warn, "\"analysis\" failed. (" + return_string(type) + ")", "Weather::analysis");
				std::string what = e.what();
				std::string detail = std::format("{0}  {1}  {2}", e.return_string_ExcptType(), e.return_string_AddressMethod(), what);
				Tools::log_write(Warn, "detail: " + detail, "Weather::analysis");
				return false;
			}
			

			Tools::log_write(Info, "\"get_weather_kit\" succeeded. (" + return_string(type) + ")", "Weather::get_weather_kit");
			delete pg;
			return true;
		}
		else
		{
			Tools::log_write(Warn, "\"get_weather_kit\" failed. (" + return_string(type) + ")", "Weather::get_weather_kit");
		}
		delete pg;
		return false;
	}

	void Weather::setHeaders() {

		this->Headers.push_back("Accept");
		this->Headers.push_back("*/*");

		//   this->Headers.push_back("User - Agent: Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 121.0.0.0 Safari / 537.36 Edg / 121.0.0.0");
		//   this->Headers.push_back("Cookie: f_city=%E6%88%90%E9%83%BD%7C101270101%7C");
		this->Headers.push_back("User-Agent");
		this->Headers.push_back("yc_GetWeather_get/" + std::string(YC_GETWEATHER_CORE_VERSION));

		this->Headers.push_back("Referer");
		this->Headers.push_back("http://www.Weather.com.cn/");

		this->Headers.push_back("Connection");
		this->Headers.push_back("keep-alive");

		this->Headers.push_back("Accept-Language");
		this->Headers.push_back("zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6");
	}


}