#include "weather.h"

using namespace Tools;

void Weather::analysis(WebPage* page)
{
	using ET = WeatherException::ExcptType;
	using AM = WeatherException::AddressMethod;

	WebPage::ContentType GetMethod = page->get_contenttype();

	//分析开始
	std::string* WeatherSource = page->get_content_p();

	if ((*WeatherSource)[0] != 'v' || WeatherSource->empty()) throw WeatherException(ET::invalid_formatting, AM::back, "Formatting changed or wrong headers.");

	size_t SourceLength = WeatherSource->size();

	this->contentType = GetMethod;
	if (GetMethod == WebPage::ContentType::instant)
	{
		if (!ReadQuoteContent(WeatherSource, "cityname", &this->Location)) throw WeatherException(ET::content_not_found, AM::back);
		if (!ReadQuoteContent(WeatherSource, "temp\"", &this->CurrentTem)) throw WeatherException(ET::content_not_found, AM::back);
		if (!ReadQuoteContent(WeatherSource, "WD", &this->CurrentWind_Direction)) throw WeatherException(ET::content_not_found, AM::back);
		if (!ReadQuoteContent(WeatherSource, "WS", &this->CurrentWind_speed)) throw WeatherException(ET::content_not_found, AM::back);
		if (!ReadQuoteContent(WeatherSource, "weather\"", &this->CurrentWeather)) throw WeatherException(ET::content_not_found, AM::back);
		if (!ReadQuoteContent(WeatherSource, "aqi\"", &this->CurrentAQI)) throw WeatherException(ET::content_not_found, AM::back);
		if (!ReadQuoteContent(WeatherSource, "time\"", &this->UpdateTime)) throw WeatherException(ET::content_not_found, AM::back);
		if (!ReadQuoteContent(WeatherSource, "date\"", &this->CurrentDate)) throw WeatherException(ET::content_not_found, AM::back);

		if (CurrentWind_speed == "0级")
		{
			CurrentWind_Direction = "无风";
		}
	}
	if (GetMethod == WebPage::ContentType::location)
	{
		if (!ReadQuoteContent(WeatherSource, "ip", &this->PublicIP)) throw WeatherException(ET::content_not_found, AM::back);
		if (!ReadQuoteContent(WeatherSource, "id", &this->LocationCode)) throw WeatherException(ET::content_not_found, AM::back);
		if (!ReadQuoteContent(WeatherSource, "addr", &this->Location)) throw WeatherException(ET::content_not_found, AM::back);
		Position::setPosition(this->Location, this->LocationCode);              //set for later use
		Tools::log_write(Tools::Info, "Location code:" + Position::getPositionCode() + " Location:" + Position::getPosition());
	}
	if (GetMethod == WebPage::ContentType::warnings)
	{
		std::string rawWarnings;
		if (!ReadQuoteContent(WeatherSource, "\"w\"", &rawWarnings)) throw WeatherException(ET::content_not_found, AM::back);
		if (rawWarnings.empty()) {
			this->Warnings = "";
			log_write(Info, "No warning found.");
		}
		else {
			if (!ReadQuoteContent(&rawWarnings, "w9", &this->Warnings)) throw WeatherException(ET::content_not_found, AM::back);
		}
	}
}




//int* FromWhere, std::string* FromWhat, std::string What, std::string* ToWhat
//FromWhat开始的地方  FromWhat的地址     查找相符的那个字符串     接收找到的字符串
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
		log_write(Err, std::format("Create file ({}) failed. At function \"get_CURL\" when preparing to store Weather source", Path));
		return FALSE;
	}
	//prepare to store weather data

	CURL* curl = curl_easy_init();

	struct curl_slist* headers = NULL;
	size_t headers_size = this->Headers.size();
	if (headers_size != 0) {
		for (size_t headers_i = 0; headers_i < headers_size; headers_i++) {
			headers = curl_slist_append(headers, this->Headers[headers_i].c_str());
			log_write(Info, "Headers -> " + this->Headers[headers_i]);
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
			if (res_IP_L == 0) log_write(Info, "Local IP is: " + LocalIP_s);
			if (res_IP_T == 0) log_write(Info, "Target IP is: " + TargetIP_s);//get IP

			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);
			fclose(file);
			/* Always clean up ! */
			return TRUE;
		}
		else
		{
			//curl failed!
			log_write(Err, "\"curl_easy_perform()\"  failed.\n" + std::string(curl_easy_strerror(res_code)));

			using WE = WeatherException::ExcptType;
			using WA = WeatherException::AddressMethod;
			WeatherException e(WE::curl, WA::back, "Network error.\n" + std::string(curl_easy_strerror(res_code)) );
			throw e;
		}
	}
	else
	{
		log_write(Err, "Curl setup error. Please try again.");
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	fclose(file);
	/* Always clean up ! */

	return false;
}




bool Weather::get_weather(WebPage::ContentType type, WebPage* page)
{
	if (!page) return false;         //nullptr

	using CT = WebPage::ContentType;


	/* ==================
			location
	   ================== */
	if (type == CT::location)
	{

		Tools::log_write(Tools::Info, "Try getting location.");

		std::string URL_GetLocationCode = "http://wgeo.weather.com.cn/ip/?_=" + Time;

		bool res1 = get_CURL(URL_GetLocationCode, "Location.txt");
		// get

		if (!res1)
		{
			Tools::log_write(Tools::Err, "Failed to get location!");
		}
		else
		{
			page->readWebPage("Location.txt",
				WebPage::ContentType::location,
				WebPage::DocType::JavaScript,
				URL_GetLocationCode);
			// make page


			return true;
		}
	}


	/* ==================
	current weather infomation
	   ================== */
	if (type == CT::instant)
	{

		Tools::log_write(Tools::Info, "Try getting weather infomation.");

		std::string URL_Content = "http://d1.weather.com.cn/sk_2d/" + Weather::Position::getPositionCode() + ".html?_=" + Time;     //MAKE URL

		Tools::log_write(Tools::Info, "Get content. URL is: " + URL_Content);

		std::string* c_fileName = new std::string;
		*c_fileName = Weather::Position::getPositionCode() + "_" + Time + ".txt";


		bool res2 = get_CURL(URL_Content, *c_fileName);
		// get

		if (!res2) {
			Tools::log_write(Tools::Err, "Failed to get weather infomation!");
		}
		else
		{
			page->readWebPage(*c_fileName,
				WebPage::ContentType::instant,
				WebPage::DocType::JavaScript,
				URL_Content);

			Tools::log_write(Tools::Info, "Getting weather infomation succeeds.");
			return true;
		}

	}


	/* ==================
		   get warnings
	   ================== */
	if (type == CT::warnings)
	{
		/* ==    get warnings    == */

		Tools::log_write(Tools::Info, "Try getting warnings.");

		std::string URL_Warnings = "http://d1.weather.com.cn/dingzhi/" + Weather::Position::getPositionCode() + ".html?_=" + Time;             //MAKE URL       
		std::string* w_fileName = new std::string;
		*w_fileName = "warnings_" + Weather::Position::getPositionCode() + "_" + Time + ".txt";

		Tools::log_write(Tools::Info, "Get warnings. URL is: " + URL_Warnings);

		bool res3 = get_CURL(URL_Warnings, *w_fileName);
		if (!res3) {
			Tools::log_write(Tools::Err, "Failed to get warnings!");
		}
		else
		{
			page->readWebPage(*w_fileName,
				WebPage::ContentType::warnings,
				WebPage::DocType::JavaScript,
				URL_Warnings);

			Tools::log_write(Tools::Info, "Getting warnings succeeds.");
			return true;
		}
	}

	Tools::log_write(Tools::Err, "\"get_weather\" failed.");
	return false;
}

bool Weather::get_weather_kit(WebPage::ContentType type)
{
	WebPage* pg = new WebPage;
	bool res = get_weather(type, pg);
	if (res)
	{
		analysis(pg);

		Tools::log_write(Info, "\"get_weather_kit\" succeeded.");
		return true;
	}
	else
	{
		Tools::log_write(Err, "\"get_weather_kit\" failed.");
	}
	return false;
}

void Weather::setHeaders() {

	this->Headers.push_back("Accept: */*");
	//   this->Headers.push_back("User - Agent: Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 121.0.0.0 Safari / 537.36 Edg / 121.0.0.0");
	//   this->Headers.push_back("Cookie: f_city=%E6%88%90%E9%83%BD%7C101270101%7C");
	this->Headers.push_back("User-Agent:cURL yc_GetWeather/1.0.0");
	this->Headers.push_back("Referer:http://www.Weather.com.cn/");
	this->Headers.push_back("Connection: keep-alive");
	this->Headers.push_back("Accept-Language: zh - CN, zh; q = 0.9, en; q = 0.8, en - GB; q = 0.7, en - US; q = 0.6");
}

std::string Weather::Position::location = "ERR",
Weather::Position::code = "ERR";