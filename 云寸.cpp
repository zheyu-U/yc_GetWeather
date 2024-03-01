#include <iostream>
#include <stdlib.h>
#include <tchar.h>


//#include "json\json.h"
#include <direct.h>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <shlobj.h>

#include <windows.h>

#include "curl/curl.h"//https://blog.csdn.net/weixin_44122235/article/details/128969128
struct WeatherInfo
{
    bool is_ERROE = FALSE;
    std::string Location, CurrentTem, Maxtem, MinTem, UpdateTime, CurrentDate, CurrentWeekday, CurrentAQI, CurrentWind_Direction, CurrentWind_speed, CurrentWeather, CurrentRainful, Rainful24, Warnings;
};

std::string URL,DataLocation ;

void Get(std::string _url, std::string _Command, std::string _FName);//爬
std::string getRoamingAppDataPath();
std::string WCHAR2String(LPCWSTR pwszSrc);
std::string String_GetCurrentTime();
void CheckIfDataFileExists();
WeatherInfo AnalyseWeatherFile(std::string FileName, int GetMethod);
std::string readFileIntoString(std::string filename);
bool is_stringC(std::string Origin, std::string a, size_t q);
int Where_cntr(std::string Source_in, size_t i_in);
bool Get_CURL(std::string url_cURL, std::string _FName);
void ReadQuoteContent(int* FromWhere, std::string* FromWhat, std::string What, std::string* ToWhat);


int main()
{
    SetConsoleOutputCP(CP_UTF8);
    //system("chcp 65001");//cout控制台中文乱码问题
    //在visual studio的项目属性 -> 配置属性 -> c/c++ -> 命令行 -> 其它选项 中填写 /utf-8
    CheckIfDataFileExists();//Check whether %AppData%/yc exists and create it.
    
    std::string StringCurrentTime = String_GetCurrentTime();//gat time函数
    
    URL = "http://d1.weather.com.cn/sk_2d/101270101.html?_=" + StringCurrentTime;
                            /*
                            http://d1.weather.com.cn/sk_2d/101270101.html?_=1706879114687
                                                                       ==>`<=== there is a '_'!
                            */;

    std::cout << "[INFO]URL is: " << URL << "\n";
    
    bool is_sud = Get_CURL(URL, StringCurrentTime + ".txt");
    if (!is_sud)
    {
        std::cout << "[ERROR]Failed to get!\n";
        exit(1);
    }
 //   Get(URL, "curl", StringCurrentTime + ".txt");

 //   system("pause");
    WeatherInfo Current_Weather_INFO = AnalyseWeatherFile("\\" + StringCurrentTime + ".txt", 1);
    if (Current_Weather_INFO.is_ERROE == TRUE)
    {
        std::cout << "[ERROR]Analyse error\n";
    }
    else
    {
        system("pause");
        system("cls");
        std::cout << "城市:" << Current_Weather_INFO.Location << "\n实时精准气温:" << Current_Weather_INFO.CurrentTem << "℃    ";
        std::cout << "实时天气:" << Current_Weather_INFO.CurrentWeather << std::endl;
        std::cout << "风向:" << Current_Weather_INFO.CurrentWind_Direction << "  风速:" << Current_Weather_INFO.CurrentWind_speed<< std::endl;
        std::cout << "AQI指数:" << Current_Weather_INFO.CurrentAQI << "\n\n";

    }
    system("pause");
    return 0;
}


//Check whether %AppData%/yc exists and create it.
void CheckIfDataFileExists() {
    DataLocation = getRoamingAppDataPath() + "\\yc";
    std::cout << "[INFO]Data location is:" << DataLocation << std::endl;
    std::filesystem::path Path_DataLocation = DataLocation;
    if (!std::filesystem::exists(Path_DataLocation)) {
        if (!std::filesystem::create_directories(Path_DataLocation)) {
            std::cout << "无法储存数据文件。创建%AppData%/yc失败。\n";
            exit(0);
        }
    }
    if (!std::filesystem::exists(Path_DataLocation)) {
        std::cout << "无法储存数据文件。创建%AppData%/yc失败。甚至std::filesystem::create_directories()没有报错。是用户名含中文名吗？\n";
        exit(0);
    }
}


// 下载jsoncpp  -》https://github.com/open-source-parsers/jsoncpp or directly from https://github.com/Kitware/CMake/releases/download/v3.29.0-rc1/cmake-3.29.0-rc1-windows-x86_64.msi
//运行 amalgamate.py 以安装，把dist文件夹里的include文件夹拷到 右侧解决方案资源管理器中 解决方案-引用-外部依赖项文件夹中
//https://blog.csdn.net/luxpity/article/details/116809954
WeatherInfo AnalyseWeatherFile(std::string FileName, int GetMethod)
{
    WeatherInfo weather_ana;
    weather_ana.is_ERROE = FALSE;
    if (!std::filesystem::exists(DataLocation + FileName))		//必须先检测目录是否存在才能使用文件入口.
        weather_ana.is_ERROE = TRUE;

    std::ifstream F_in;
    F_in.open(DataLocation + FileName, std::ios::in);
    if (F_in.is_open() == FALSE) {
        weather_ana.is_ERROE = TRUE;
    }

    std::string WeatherSource = readFileIntoString(DataLocation + FileName);//read and store in WeatherSource
    if (WeatherSource == "ERROR") weather_ana.is_ERROE = TRUE;
    F_in.close();

    //分析开始

    if (WeatherSource[0] != 'v' && WeatherSource[0] != '{') weather_ana.is_ERROE = TRUE;
    size_t SourceLength = WeatherSource.size();

    for (int i = 0; i < SourceLength; i++)
    {
        ReadQuoteContent(&i, &WeatherSource, "cityname", &weather_ana.Location);
        ReadQuoteContent(&i, &WeatherSource, "temp\"", &weather_ana.CurrentTem);
        ReadQuoteContent(&i, &WeatherSource, "WD", &weather_ana.CurrentWind_Direction);
        ReadQuoteContent(&i, &WeatherSource, "WS", &weather_ana.CurrentWind_speed);
        ReadQuoteContent(&i, &WeatherSource, "weather\"", &weather_ana.CurrentWeather);
        ReadQuoteContent(&i, &WeatherSource, "aqi\"", &weather_ana.CurrentAQI);
    }
    return weather_ana;
}

//int* FromWhere, std::string* FromWhat, std::string What, std::string* ToWhat
//FromWhat开始的地方  FromWhat的地址     查找相符的那个字符串     接收找到的字符串
//查找"abc":"str" 输入what(是abc) 输出ToWhat (是str)
//原理：数引号
void ReadQuoteContent(int* FromWhere, std::string* FromWhat, std::string What, std::string* ToWhat)
{
    if (is_stringC(*FromWhat, What, *FromWhere))                      //          ~<-- where{i + tcntrl - 2}
    {                                                                 //          0 1 2 3 4 5<- = Where_cntr(WeatherSource, i + tcntrl - 2)
        int tcntrl = Where_cntr(*FromWhat, *FromWhere);               //c i t y " : " b c " , ". .
                                                                      //0 1 2 3 4 5 6 7 8 9 
        //                                                 ~because-->                ~<-- = tcntrl & where{i + tcntrcl}
        for (int j = tcntrl; j < tcntrl + Where_cntr(*FromWhat, *FromWhere + tcntrl - 2) - 3; j++)
        {
            *ToWhat += (*FromWhat)[*FromWhere + j];
        }
        *FromWhere += Where_cntr(*FromWhat, *FromWhere + tcntrl - 2);
        
    }
    
}

//在analise中判断数据开始的地方(by count the number of \")
// 初始值：如temp的t所在的i值
int Where_cntr(std::string Source_in, size_t i_in)
{
    int l_q = 0,cntr = 0;
    while (l_q < 2) {
        if (Source_in[i_in + cntr + 1] == '\"')//本位是第0位，从1位开检
        {
            l_q++;
        }
        cntr++;
    }
    return (cntr + 1);//返回引号后的一位
}



//原始数据，需要数据，i（原始数据开始-1）。
bool is_stringC(std::string Origin, std::string a, size_t q)
{
    //cout<<a;
    int len = a.size();
    int m = 0;
    for (size_t r = q; r < (q + len); r++) {

        if (Origin[r] != a[m])return 0;
        m++;
    }
    return 1;
}

//读文件 ->https://blog.csdn.net/luxpity/article/details/116809954
std::string readFileIntoString(std::string filename)
{
    std::ifstream ifile(filename);
    if (!ifile.is_open()) {
        return "ERROR";
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
    std::cout << "[INFO]Current Time is: " << STR_CurrentTime << " .\n";
    return STR_CurrentTime;
}

//https://blog.csdn.net/MOU_IT/article/details/96457666
//https://everything.curl.dev/examples/get
bool Get_CURL(std::string url_cURL, std::string _FName)
{
    FILE* fcw;
    std::string FileDAN = DataLocation + "\\" + _FName;
    if (fopen_s(&fcw, FileDAN.c_str(), "wt+") )
    {
        return FALSE;
    }

    CURL* curl = curl_easy_init();
    CURLcode res_code;
    curl_global_init(CURL_GLOBAL_ALL);

    struct curl_slist* headers = NULL;
    std::string headers_Accept_Str = "Accept: */*";
    std::string headers_UserAgent_Str = "User - Agent: Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 121.0.0.0 Safari / 537.36 Edg / 121.0.0.0"; 
    std::string headers_Cookie_Str = "Cookie: f_city=%E6%88%90%E9%83%BD%7C101270101%7C";
    std::string headers_Referer_Str = "Referer: http://www.weather.com.cn/";
    std::string headers_Connection_Str = "Connection: keep - alive";
    std::string headers_AcceptLan_Str = "Accept - Language: zh - CN, zh; q = 0.9, en; q = 0.8, en - GB; q = 0.7, en - US; q = 0.6";
    headers = curl_slist_append(headers, headers_AcceptLan_Str.c_str());
    headers = curl_slist_append(headers, headers_Accept_Str.c_str());
    headers = curl_slist_append(headers, headers_UserAgent_Str.c_str());
//    headers = curl_slist_append(headers, headers_Cookie_Str.c_str());
    headers = curl_slist_append(headers, headers_Referer_Str.c_str());
    headers = curl_slist_append(headers, headers_Connection_Str.c_str());
    if (curl)
    {
        char* MyIP;
        curl_easy_setopt(curl, CURLOPT_URL, url_cURL.c_str());
        curl_easy_setopt(curl, CURLOPT_COOKIE, headers_Cookie_Str.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//        curl_easy_setopt(curl, CURLOPT_HEADERDATA, fcw);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fcw);
        
        res_code = curl_easy_perform(curl);      
        CURLcode res_IP = curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &MyIP);
        if (res_code == 0)
        {
            if (res_IP == 0)
            {
                std::cout << "[INFO]IP is: ";
                
                std::cout << MyIP << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            fclose(fcw);
            return TRUE;
        }
        
        
    }
    std::cout << "[ERROR]curlcode is :" << curl << std::endl;
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    fclose(fcw);
    
    return FALSE;
}

//爬虫板块，使用cmd的curl
//参数：网址，命令（只有curl），输出文件名（要加后缀，如a.txt）
//abandoned!
void Get(std::string _url, std::string _Command, std::string _FName) {
    std::string TryCommand = "/c curl \"" + _url + "\" "  
        + "-H \"Accept: */"
        + "* \"-H \"Accept - Language: zh - CN, zh; q = 0.9, en; q = 0.8, en - GB; q = 0.7, en - US; q = 0.6\" "
        + "-H \"Connection: keep - alive\" "
        + "-H \"Cookie: f_city=%E6%88%90%E9%83%BD%7C101270101%7C\" "
        + "-H \"Referer: http://www.weather.com.cn/\" "
        + "-H \"User - Agent: Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 121.0.0.0 Safari / 537.36 Edg / 121.0.0.0\""
        + "--compressed --insecure "
        + ">> " + DataLocation + "\\" + _FName;
    //节选自Cookie，没有也能爬--> ; Hm_lvt_080dabacb001ad3dc8b9b9049b36d43b=1705818063,1706689152,1706779743,1706878760; Hm_lpvt_080dabacb001ad3dc8b9b9049b36d43b=1706879055
    std::cout << "[INFO]Command is --> (cmd) " + TryCommand << std::endl;

    //ShellExecuteA(NULL, "runas", "cmd", "/c ipconfig >> D:\\disk.txt", NULL, SW_SHOWNORMAL);
    ShellExecuteA(NULL, "open", "cmd", TryCommand.c_str(), NULL, SW_MAX);

    //system("pause");
    
 //   curl "http://d1.weather.com.cn/sk_2d/101270101.html?_=1706879114687" ^
 // -H "Accept: */*" ^
 /*
    -H "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6" ^
      -H "Connection: keep-alive" ^
      -H "Cookie: f_city=^%^E6^%^88^%^90^%^E9^%^83^%^BD^%^7C101270101^%^7C; Hm_lvt_080dabacb001ad3dc8b9b9049b36d43b=1705818063,1706689152,1706779743,1706878760; Hm_lpvt_080dabacb001ad3dc8b9b9049b36d43b=1706879055" ^
      -H "Referer: http://www.weather.com.cn/" ^
      -H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36 Edg/121.0.0.0" ^
      --compressed ^
      --insecure
    */
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
