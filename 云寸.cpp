#include <iostream>
#include <stdlib.h>
#include <tchar.h>

#include <array>
#include <vector>
#include <direct.h>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <shlobj.h>

#include <windows.h>

#include <stdexcept>

#include "curl/curl.h"//curl https://blog.csdn.net/weixin_44122235/article/details/128969128
#include "colored-cout/colored_cout.h"//https://github.com/yurablok/colored-cout/
#include "colored-cout/colored_cout.cpp"
struct WeatherInfo
{
    std::string Location, LocationCode, PublicIP, CurrentTem, Maxtem, MinTem, UpdateTime, CurrentDate, CurrentWeekday, CurrentAQI, CurrentWind_Direction, CurrentWind_speed, CurrentWeather, CurrentRainful, Rainful24, Warnings;
    std::vector<std::string> Headers;
}Weather;

const int _Headers_{ 30 }, _Location_{ 31 }, _Content_{ 32 };             //get函数methods
std::string DataLocation ;

std::string getRoamingAppDataPath();
std::string WCHAR2String(LPCWSTR pwszSrc);
std::string String_GetCurrentTime();
void CheckIfDataFileExists();
void AnalyseWeatherFile(std::string FileName, int GetMethod, WeatherInfo* weather_store);
std::string readFileIntoString(std::string filename);
bool Get_CURL(std::string url_cURL, std::string _FName, std::vector<std::string>* Headers_fun);
bool ReadQuoteContent(std::string* FromWhat, std::string What, std::string* ToWhat);
void Store_Headers_Getdata(std::vector<std::string>* Headers_Getdata_Fun);

int main()
{
    /*
    -------------PART ONE : PREPARATIONS---------------------------------
    */
    SetConsoleOutputCP(CP_UTF8);
                                                                         //system("chcp 65001");//cout控制台中文乱码问题                                                                //在visual studio的项目属性 -> 配置属性 -> c/c++ -> 命令行 -> 其它选项 中填写 /utf-8  ref->https://learn.microsoft.com/zh-cn/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=msvc-170
    curl_global_init(CURL_GLOBAL_ALL);                                   //初始化cURL
    CheckIfDataFileExists();                                             //Check whether %AppData%/yc exists and create it.
    std::string StringCurrentTime = String_GetCurrentTime();             //gat time函数
    


     //PART TWO : MAKE HEADERS---------------------------------
    {
        Store_Headers_Getdata(&Weather.Headers);    
    }


    //PART THREE : GET LOCATIONS---------------------------------
    {
    std::string URL_GetLocationCode = "http://wgeo.weather.com.cn/ip/?_=" + StringCurrentTime;
    if (Get_CURL(URL_GetLocationCode, "Locatioin.txt", &Weather.Headers) == 0)
    {
        std::cout << clr::red << "[ERROR]" << clr::reset << "Failed to get location!\n";
        exit(1);
    }
    try { 
        AnalyseWeatherFile("\\Locatioin.txt", _Location_, &Weather); 
    }
    catch (std::invalid_argument& excpt_AnA)
    {
        std::string err = excpt_AnA.what();
        if (err == "NOT EXIST")  std::cout << clr::red << "[ERROR]" << clr::reset << "File does not exists.\n";
        if (err == "NOT OPEN")    std::cout << clr::red << "[ERROR]" << clr::reset << "File can not open.\n";
        if (err == "INVALID CONTENT")   std::cout << clr::red << "[ERROR]" << clr::reset << "Cann't understand....\n";
        if (err == "NOT FOUND")   std::cout << clr::red << "[ERROR]" << clr::reset << "Cann't find the weather infomation....\n";
        exit(1);
    }
    std::cout << clr::green << "[INFO]" << clr::reset << "Location code:" << Weather.LocationCode << "\n" << clr::green << "[INFO]" << clr::reset << "Location:" << Weather.Location << std::endl;
    }
 

    /*
    -------------PART FOUR : MAKE CONTENT URL---------------------------------
    */

    std::string URL_Content = "http://d1.weather.com.cn/sk_2d/" + Weather.LocationCode + ".html?_=" + StringCurrentTime;
                           
    /*
                           
            http://d1.weather.com.cn/sk_2d/101270101.html?_=1706879114687
                                                       ==>`<=== there is a '_'!
     */;

    std::cout << clr::green << "[INFO]" << clr::reset << "URL is: " << URL_Content << "\n";
    bool is_sud = Get_CURL(URL_Content, StringCurrentTime + ".txt", &Weather.Headers);
    if (!is_sud){
        std::cout << clr::red << "[ERROR]" << clr::reset << "Failed to get!\n";
        exit(1);
    }
    
    
 //   Get(URL, "curl", StringCurrentTime + ".txt");

 //   system("pause");

    /*
    -------------PART FIVE :ANALYSE AND PRINT---------------------------------
    */
    bool isErr = false;
    try
    {
        AnalyseWeatherFile("\\" + StringCurrentTime + ".txt", _Content_, &Weather);
    }
    catch (std::invalid_argument& excpt_AnA)
    {
        std::string err = excpt_AnA.what();
        if (err == "NOT EXIST")  std::cout << clr::red << "[ERROR]" << clr::reset << "File does not exists.\n";
        if (err == "NOT OPEN")    std::cout << clr::red << "[ERROR]" << clr::reset << "File can not open.\n";
        if (err == "INVALID CONTENT")   std::cout << clr::red << "[ERROR]" << clr::reset << "Cann't understand....\n";
        if (err == "NOT FOUND")   std::cout << clr::red << "[ERROR]" << clr::reset << "Cann't find the weather infomation....\n";
        exit(1);
    }
    
    
    {
        system("pause");
        system("cls");
        std::cout << "更新日期:" << clr::white << Weather.CurrentDate << " " << Weather.UpdateTime << clr::reset << std::endl;
        std::cout << "城市:" << clr::green <<Weather.Location << clr::reset << "\n实时精准气温:" << clr::green << Weather.CurrentTem << "℃    " << clr::reset;
        std::cout << "实时天气:" << clr::green << Weather.CurrentWeather << clr::reset << std::endl;
        if (Weather.CurrentWind_speed == "0级")
        {
            Weather.CurrentWind_Direction = "无风";
        }
        std::cout << "风向:" << clr::green << Weather.CurrentWind_Direction << clr::reset << "  风速:" << clr::green << Weather.CurrentWind_speed << clr::reset << std::endl;
        std::cout << "AQI指数:";
        if (std::stoi(Weather.CurrentAQI) <= 50)std::cout << clr::green << Weather.CurrentAQI << " 优" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) <= 100)std::cout << clr::yellow << Weather.CurrentAQI << " 良" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) <= 150)std::cout << clr::magenta << Weather.CurrentAQI << " 轻度污染" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) <= 200)std::cout << clr::red << Weather.CurrentAQI << " 中度污染" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) <= 300)std::cout << clr::on_magenta << Weather.CurrentAQI << " 重度污染" << clr::reset << "\n\n";
        else if (std::stoi(Weather.CurrentAQI) > 300)std::cout << clr::on_red << Weather.CurrentAQI << " 严重污染" << clr::reset << "\n\n";
        std::cout << "更多请访问 " << clr::cyan << "http://www.weather.com.cn/\n" << clr::reset;
        

    }
    system("pause");
    return 0;
}


//Check whether %AppData%/yc exists and create it.
void CheckIfDataFileExists() {
    DataLocation = getRoamingAppDataPath() + "\\yc";
    std::cout << clr::green << "[INFO]" << clr::reset << "Data location is:" << DataLocation << std::endl;
    std::filesystem::path Path_DataLocation = DataLocation;
    if (!std::filesystem::exists(Path_DataLocation)) {
        if (!std::filesystem::create_directories(Path_DataLocation)) {
            std::cout << clr::red << "[ERROR]" << clr::reset << "无法储存数据文件。创建%AppData%/yc失败。\n";
            exit(0);
        }
    }
    if (!std::filesystem::exists(Path_DataLocation)) {
        std::cout << clr::red << "[ERROR]" << clr::reset << "无法储存数据文件。创建%AppData%/yc失败。甚至std::filesystem::create_directories()没有报错。是用户名含中文名吗？\n";
        exit(0);
    }
}



void AnalyseWeatherFile(std::string FileName, int GetMethod, WeatherInfo* weather_store)
{
    if (!std::filesystem::exists(DataLocation + FileName))		//必须先检测目录是否存在才能使用文件入口.
        throw std::invalid_argument( "NOT EXIST");

    std::ifstream F_in;
    F_in.open(DataLocation + FileName, std::ios::in);
    if (F_in.is_open() == FALSE) {
        throw std::invalid_argument( "NOT OPEN");
    }

    std::string WeatherSource;
    try
    {
        WeatherSource = readFileIntoString(DataLocation + FileName);//read and store in WeatherSource
    }
    catch (std::string excp)
    {
        if (excp == "NOT OPEN")
        {
            F_in.close();
            throw std::invalid_argument( "NOT OPEN");
        }
    }
    F_in.close();

    //分析开始

    if (WeatherSource[0] != 'v' || WeatherSource.empty()) throw std::invalid_argument("INVALID CONTENT");
    size_t SourceLength = WeatherSource.size();
    
    if (GetMethod == _Content_)
    {
        if (!ReadQuoteContent(&WeatherSource, "cityname", &(*weather_store).Location)) throw std::invalid_argument( "NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "temp\"", &(*weather_store).CurrentTem)) throw std::invalid_argument( "NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "WD", &(*weather_store).CurrentWind_Direction)) throw std::invalid_argument( "NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "WS", &(*weather_store).CurrentWind_speed)) throw std::invalid_argument( "NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "weather\"", &(*weather_store).CurrentWeather)) throw std::invalid_argument( "NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "aqi\"", &(*weather_store).CurrentAQI)) throw std::invalid_argument( "NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "time\"", &(*weather_store).UpdateTime)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "date\"", &(*weather_store).CurrentDate)) throw std::invalid_argument("NOT FOUND");

    }
    if (GetMethod == _Location_)
    {
        if (!ReadQuoteContent(&WeatherSource, "ip", &(*weather_store).PublicIP)) throw std::invalid_argument( "NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "id", &(*weather_store).LocationCode)) throw std::invalid_argument( "NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "addr", &(*weather_store).Location)) throw std::invalid_argument( "NOT FOUND");
    }
}

//int* FromWhere, std::string* FromWhat, std::string What, std::string* ToWhat
//FromWhat开始的地方  FromWhat的地址     查找相符的那个字符串     接收找到的字符串
//查找"abc":"str" 输入what(是abc) 输出ToWhat (是str)
//原理：数引号
bool ReadQuoteContent(std::string* FromWhat, std::string What, std::string* ToWhat)
{                                                             
    size_t text_start = (*FromWhat).find(What);
    if (text_start == std::string::npos) return FALSE;
    size_t equal_mark = (*FromWhat).find_first_of(":=", text_start);
    if (equal_mark == std::string::npos) return FALSE;
    size_t value_start = (*FromWhat).find("\"", equal_mark) + 1;
    size_t value_end = (*FromWhat).find("\"", value_start);
    if (value_start == std::string::npos || value_end == std::string::npos) return FALSE;
    //std::cout << text_start << " :" << equal_mark << " :" << value_start << " :" << value_end << std::endl;
    (*ToWhat).clear();
    (*ToWhat).append((*FromWhat), value_start, value_end - value_start);
    return true;
}


//读文件 ->https://blog.csdn.net/luxpity/article/details/116809954
std::string readFileIntoString(std::string filename)
{
    std::ifstream ifile(filename);
    if (!ifile.is_open()) {
        throw std::invalid_argument( "NOT OPEN");
    }
    std::ostringstream buf;
    std::string filestr;
    char ch;
    while (buf && ifile.get(ch)) {
        buf.put(ch);
    }
    filestr = buf.str();
    return filestr;
}

//获取 LocalAppDataPath    >>Reference:https://zhuanlan.zhihu.com/p/529344650
//<shlobj.h>
std::string getRoamingAppDataPath() {
    std::string strLocalAppDataPath = "";
    WCHAR* strPath = nullptr;
    if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &strPath))) {
        strLocalAppDataPath = WCHAR2String(strPath);
        CoTaskMemFree(strPath);
    }

    return strLocalAppDataPath;
}

// WCHAR 转换为 std::string    >>Reference:https://blog.csdn.net/kingkee/article/details/98115024
std::string WCHAR2String(LPCWSTR pwszSrc)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
    if (nLen <= 0)
        return std::string("");

    char* pszDst = new char[nLen];
    if (NULL == pszDst)
        return std::string("");

    WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
    pszDst[nLen - 1] = 0;

    std::string strTmp(pszDst);
    delete[] pszDst;

    return strTmp;
}

 //获取当前时间
std::string String_GetCurrentTime() {
    /*一种获取当前时间方法
    long long _T_CurrentTime = std::chrono::duration_cast<std::chrono::milliseconds>(_CurrentTime.time_since_epoch()).count();
    std::cout << _T_CurrentTime << std::endl;
    */
    std::chrono::system_clock::time_point _F_CurrentTime = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point _F_Time_0 = std::chrono::system_clock::time_point(std::chrono::seconds(0));
    std::chrono::duration<double> F_CurrentTimeGap = _F_CurrentTime - _F_Time_0;
    std::string STR_CurrentTime = std::to_string(long long(F_CurrentTimeGap.count() * 1000));
    std::cout << clr::green << "[INFO]" << clr::reset << "Current Time is: " << STR_CurrentTime << " .\n";
    return STR_CurrentTime;
}

//https://blog.csdn.net/MOU_IT/article/details/96457666
//https://everything.curl.dev/examples/get
//URL ,file name, headers
bool Get_CURL(std::string url_cURL, std::string _FName, std::vector<std::string>* Headers_fun)
{ 
    std::string FileDAN = DataLocation + "\\" + _FName;
    FILE* fcw;
    if (fopen_s(&fcw, FileDAN.c_str(), "wt+") )
    {
        return FALSE;
    }

    CURL* curl = curl_easy_init();
    CURLcode res_code;

    //--headers
    struct curl_slist* headers = NULL;
    if ((*Headers_fun).size() != 0) {
        for (size_t headres_i = 0; headres_i < (*Headers_fun).size(); headres_i++) {
            headers = curl_slist_append(headers, (*Headers_fun)[headres_i].c_str());
        std:: cout << clr::green << "[INFO]" << clr::reset << "Headers -> " << (*Headers_fun)[headres_i] << std::endl;
        }
    }

    if (curl)
    {
        char* MyIP;
        curl_easy_setopt(curl, CURLOPT_URL, url_cURL.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//        curl_easy_setopt(curl, CURLOPT_HEADERDATA, fcw);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fcw);
        
        res_code = curl_easy_perform(curl);      
        CURLcode res_IP = curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &MyIP);
        if (res_code == 0)
        {
            if (res_IP == 0)
            {
                std::cout << clr::green << "[INFO]" << clr::reset << "IP is: ";               
                std::cout << MyIP << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            fclose(fcw);
            return TRUE;
        }
    }
    std::cout << clr::red << "[ERROR]" << clr::reset << "curlcode is :" << curl << std::endl;
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    fclose(fcw);
    
    return FALSE;
}


void Store_Headers_Getdata(std::vector<std::string>* Headers_Getdata_Fun) {

    (*Headers_Getdata_Fun).push_back("Accept: */*");
 //   (*Headers_Getdata_Fun).push_back("User - Agent: Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 121.0.0.0 Safari / 537.36 Edg / 121.0.0.0");
//    (*Headers_Getdata_Fun).push_back("Cookie: f_city=%E6%88%90%E9%83%BD%7C101270101%7C");
    (*Headers_Getdata_Fun).push_back("User-Agent:cURL yc_GetWeather/1.0.0");
    (*Headers_Getdata_Fun).push_back("Referer:http://www.weather.com.cn/");
    (*Headers_Getdata_Fun).push_back("Connection: keep-alive");
    (*Headers_Getdata_Fun).push_back("Accept-Language: zh - CN, zh; q = 0.9, en; q = 0.8, en - GB; q = 0.7, en - US; q = 0.6");
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
