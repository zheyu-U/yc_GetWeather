#include <iostream>
#include <tchar.h>
#include <stdexcept>
#include "weather.h"
#include "colored_cout.h"//https://github.com/yurablok/colored-cout/



int main()
{
	SetConsoleOutputCP(CP_UTF8);                                         //PREPARATIONS           cout控制台中文乱码问题 在 visual studio的项目属性 -> 配置属性 -> c/c++ -> 命令行 -> 其它选项 中填写 /utf-8  ref->https://learn.microsoft.com/zh-cn/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=msvc-170
	curl_global_init(CURL_GLOBAL_ALL);                                   //初始化cURL
	Tools::CheckIfDataFileExists();                                     //Check whether %AppData%/yc exists and create it.
	std::string StringCurrentTime = Tools::String_GetCurrentTime();     //gat time函数
	Weather* weatherInstant = new Weather(WebPage::ContentType::instant);

	try
	{
		/*-- get location --*/
		bool res1 = weatherInstant->get_weather_kit(WebPage::ContentType::location);

		/*-- get weather infomation --*/
		bool res2 = weatherInstant->get_weather_kit(WebPage::ContentType::instant);

		/*-- get warnings --*/
		bool res3 = weatherInstant->get_weather_kit(WebPage::ContentType::warnings);
	}
	catch (const WeatherException& e)
	{
		Tools::log_write(Tools::Err, e.errView());

		using A = WeatherException::AddressMethod;
		if (e.get_AddressMethod() == A::back || e.get_AddressMethod() == A::crash )
		{
			Tools::log_write(Tools::Err, "Now crash.");
			exit(1);
		}
	}




	/* print */
	{
#ifdef _DEBUG
		system("pause");
		system("cls");
#endif // _DEBUG
		std::cout << "更新日期:" << clr::white << weatherInstant->CurrentDate << " " << weatherInstant->UpdateTime << clr::reset << std::endl;
		std::cout << "城市:" << clr::green << weatherInstant->Location << clr::reset << "\n实时精准气温:" << clr::green << weatherInstant->CurrentTem << "℃    " << clr::reset;
		std::cout << "实时天气:" << clr::green << weatherInstant->CurrentWeather << clr::reset << std::endl;
		std::cout << "风向:" << clr::green << weatherInstant->CurrentWind_Direction << clr::reset << "  风速:" << clr::green << weatherInstant->CurrentWind_speed << clr::reset << std::endl;
		std::cout << "AQI指数:";
		if (std::stoi(weatherInstant->CurrentAQI) <= 50)std::cout << clr::green << weatherInstant->CurrentAQI << " 优" << clr::reset << "\n\n";
		else if (std::stoi(weatherInstant->CurrentAQI) <= 100)std::cout << clr::yellow << weatherInstant->CurrentAQI << " 良" << clr::reset << "\n\n";
		else if (std::stoi(weatherInstant->CurrentAQI) <= 150)std::cout << clr::magenta << weatherInstant->CurrentAQI << " 轻度污染" << clr::reset << "\n\n";
		else if (std::stoi(weatherInstant->CurrentAQI) <= 200)std::cout << clr::red << weatherInstant->CurrentAQI << " 中度污染" << clr::reset << "\n\n";
		else if (std::stoi(weatherInstant->CurrentAQI) <= 300)std::cout << clr::on_magenta << weatherInstant->CurrentAQI << " 重度污染" << clr::reset << "\n\n";
		else if (std::stoi(weatherInstant->CurrentAQI) > 300)std::cout << clr::on_red << weatherInstant->CurrentAQI << " 严重污染" << clr::reset << "\n\n";
		if (!weatherInstant->Warnings.empty()) std::cout << clr::white << weatherInstant->Warnings << clr::reset << std::endl << std::endl;
		std::cout << "更多请访问 " << clr::cyan << "http://www.weather.com.cn/\n" << clr::reset;
	}
	system("pause");
	return 0;
}











// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
