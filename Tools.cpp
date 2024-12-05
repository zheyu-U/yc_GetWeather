#include "pch.h"
#include "Tools.h"



//获取 LocalAppDataPath    >>Reference:https://zhuanlan.zhihu.com/p/529344650
//<shlobj.h>
std::string Tools::getRoamingAppDataPath() {

#ifdef _CONSOLE
	PWSTR strPath = nullptr;
	std::filesystem::path appDataPath;

	if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &strPath))) {
		appDataPath = strPath;
		//		strLocalAppDataPath = WCHAR2String(strPath);
		CoTaskMemFree(strPath);
	}
	else {
		std::filesystem::filesystem_error::runtime_error e("Unable to store data");
		throw e;
	}
	return appDataPath.string();
#endif // _CONSOLE


#ifdef DeskTop_yc
#include <winrt/Windows.Storage.h>
	winrt::Windows::Storage::UserDataPaths c = winrt::Windows::Storage::UserDataPaths::GetDefault();
	return winrt::to_string(c.RoamingAppData());
#endif


}



//读文件
void Tools::readFileIntoString(std::string Path, std::string* str)
{
	std::ifstream file;
	file.open(Path, std::ios::in);
	if (file.is_open() == false) {
		throw WeatherException(WeatherException::ExcptType::file, WeatherException::AddressMethod::back, "Cannot open the file!");
	}
	//open

	size_t cnt = file.gcount();
	if (cnt >= 655'360) throw WeatherException(WeatherException::ExcptType::content_too_long, WeatherException::AddressMethod::back, WeatherException::ExcptType::content_too_long.view);
	//check size
	//ref https://cplusplus.com/reference/fstream/ifstream/

	(*str).clear();
	char buff;
	while (file.get(buff)) (*str).push_back(buff);
}


//log_write在开发中
void Tools::log_write(const int lvl, const std::string writelog)
{

#ifdef _CONSOLE  //控制台程序使用

	std::string loglvl;
	clr colorShow = clr::on_red;
	switch (lvl)
	{
	case Err:
		loglvl = "[ERROR]";
		colorShow = clr::red;
		break;
	case Warn:
		loglvl = "[WARNING]";
		colorShow = clr::yellow;
		break;
	case Info:
		loglvl = "[INFO]";
		colorShow = clr::green;
		break;
	}

#ifndef _DEBUG  //display when debug 调试时才显示	
	if (colorShow != clr::green)
	{
		std::cout << colorShow << loglvl << clr::reset;
		std::cout << writelog << std::endl;
	}
#endif // !_DEBUG

#ifdef _DEBUG  //display when debug 调试时才显示	
	std::cout << colorShow << loglvl << clr::reset;
	std::cout << writelog << std::endl;
#endif // _DEBUG

#endif // _CONSOLE
}


//Check whether %AppData%/yc exists and create it.
void Tools::CheckIfDataFileExists() {
	DataLocation = getRoamingAppDataPath() + "\\yc";
	//std::cout << clr::green << "[INFO]" << clr::reset << "Data location is:" << DataLocation << std::endl;
	log_write(Info, "Data location is:" + DataLocation);
	std::filesystem::path Path_DataLocation = DataLocation;
	if (!std::filesystem::exists(Path_DataLocation)) {
		if (!std::filesystem::create_directories(Path_DataLocation)) {
			//std::cout << clr::red << "[ERROR]" << clr::reset << "无法储存数据文件。创建%AppData%/yc失败。\n";
			log_write(Err, "无法储存数据文件。创建%AppData%/yc失败。");
			exit(0);
		}
	}
	if (!std::filesystem::exists(Path_DataLocation)) {
		//std::cout << clr::red << "[ERROR]" << clr::reset << "无法储存数据文件。创建%AppData%/yc失败。甚至 std::filesystem::create_directories()没有报错。是用户名含中文名吗？\n";
		log_write(Err, "无法储存数据文件。创建%AppData%/yc失败。甚至 std::filesystem::create_directories()没有报错。是用户名含中文名吗？");
		exit(0);
	}
}

std::string Tools::Get_DataLocation()
{
	return DataLocation;
}

std::string Tools::String_GetCurrentTime()
{

	/*一种获取当前时间方法
	long long _T_CurrentTime = std::chrono::duration_cast<std::chrono::milliseconds>(_CurrentTime.time_since_epoch()).count();
	std::cout << _T_CurrentTime << std::endl;
	*/
	std::chrono::system_clock::time_point _F_CurrentTime = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point _F_Time_0 = std::chrono::system_clock::time_point(std::chrono::seconds(0));
	std::chrono::duration<double> F_CurrentTimeGap = _F_CurrentTime - _F_Time_0;
	std::string STR_CurrentTime = std::to_string(long long(F_CurrentTimeGap.count() * 1000));
	//std::cout << clr::green << "[INFO]" << clr::reset << "Current Time is: " << STR_CurrentTime << " .\n";
	log_write(Info, "Current Time is: " + STR_CurrentTime + " .");
	return STR_CurrentTime;

}

