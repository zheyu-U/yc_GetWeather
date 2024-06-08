#include "weather.h"

using namespace weathern;
//获取当前时间
std::string weather::String_GetCurrentTime()
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

//log_write在开发中
std::string weather::log_write(const int lvl,const std::string writelog)
{
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
    std::cout << colorShow << loglvl << clr::reset;
    std::cout << writelog << std::endl;
    return loglvl + writelog;
}

std::string weather::Get_DataLocation()
{
    return DataLocation;
}

//Check whether %AppData%/yc exists and create it.
void weather::CheckIfDataFileExists() {
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
        //std::cout << clr::red << "[ERROR]" << clr::reset << "无法储存数据文件。创建%AppData%/yc失败。甚至std::filesystem::create_directories()没有报错。是用户名含中文名吗？\n";
        log_write(Err, "无法储存数据文件。创建%AppData%/yc失败。甚至std::filesystem::create_directories()没有报错。是用户名含中文名吗？");
        exit(0);
    }
}


void weather::AnalyseWeatherFile(std::string FileName, int GetMethod, weather* weather_store)
{
    if (!std::filesystem::exists(DataLocation + FileName))		//必须先检测目录是否存在才能使用文件入口.
        throw std::invalid_argument("NOT EXIST");

    std::ifstream F_in;
    F_in.open(DataLocation + FileName, std::ios::in);
    if (F_in.is_open() == false) {
        throw std::invalid_argument("NOT OPEN");
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
            throw std::invalid_argument("NOT OPEN");
        }
    }
    F_in.close();

    //分析开始

    if (WeatherSource[0] != 'v' || WeatherSource.empty()) throw std::invalid_argument("INVALID CONTENT");
    size_t SourceLength = WeatherSource.size();

    if (GetMethod == _Content_)
    {
        if (!ReadQuoteContent(&WeatherSource, "cityname", &(*weather_store).Location)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "temp\"", &(*weather_store).CurrentTem)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "WD", &(*weather_store).CurrentWind_Direction)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "WS", &(*weather_store).CurrentWind_speed)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "weather\"", &(*weather_store).CurrentWeather)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "aqi\"", &(*weather_store).CurrentAQI)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "time\"", &(*weather_store).UpdateTime)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "date\"", &(*weather_store).CurrentDate)) throw std::invalid_argument("NOT FOUND");

    }
    if (GetMethod == _Location_)
    {
        if (!ReadQuoteContent(&WeatherSource, "ip", &(*weather_store).PublicIP)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "id", &(*weather_store).LocationCode)) throw std::invalid_argument("NOT FOUND");
        if (!ReadQuoteContent(&WeatherSource, "addr", &(*weather_store).Location)) throw std::invalid_argument("NOT FOUND");
    }
    if (GetMethod == _Warnings_)
    {
        std::string rawWarnings;
        if (!ReadQuoteContent(&WeatherSource, "\"w\"", &rawWarnings)) throw std::invalid_argument("NOT FOUND");
        if (rawWarnings.empty()) (*weather_store).Warnings = "";
        else {
            if (!ReadQuoteContent(&rawWarnings, "w9", &(*weather_store).Warnings)) throw std::invalid_argument("NOT FOUND");
        }
    }
}

void weather::AnalyseWeatherFile_try(std::string FileName, int GetMethod, weather* weather_store)
{
    try { AnalyseWeatherFile(FileName, GetMethod, weather_store); }
    catch (std::invalid_argument& excpt_AnA)
    {
        std::string err = excpt_AnA.what();
        if (err == "NOT EXIST")  log_write(Err, "File does not exists.");
        if (err == "NOT OPEN")    log_write(Err, "File can not open.");
        if (err == "INVALID CONTENT")   log_write(Err, "Cann't understand....There might be something wrong with the headers.");
        if (err == "NOT FOUND")   log_write(Err, "Cann't find the weather infomation....");
        exit(1);
    }
}

//获取 LocalAppDataPath    >>Reference:https://zhuanlan.zhihu.com/p/529344650
//<shlobj.h>
std::string weather::getRoamingAppDataPath() {
    std::string strLocalAppDataPath = "";
    WCHAR* strPath = nullptr;
    if (SUCCEEDED(::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &strPath))) {
        strLocalAppDataPath = WCHAR2String(strPath);
        CoTaskMemFree(strPath);
    }

    return strLocalAppDataPath;
}

// WCHAR 转换为 std::string    >>Reference:https://blog.csdn.net/kingkee/article/details/98115024
std::string weather::WCHAR2String(LPCWSTR pwszSrc)
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

//读文件 ->https://blog.csdn.net/luxpity/article/details/116809954
std::string weather::readFileIntoString(std::string filename)
{
    std::ifstream ifile(filename);
    if (!ifile.is_open()) {
        throw std::invalid_argument("NOT OPEN");
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


//int* FromWhere, std::string* FromWhat, std::string What, std::string* ToWhat
//FromWhat开始的地方  FromWhat的地址     查找相符的那个字符串     接收找到的字符串
//查找"abc":"str" 输入what(是abc) 输出ToWhat (是str)
//原理：数引号
bool weather::ReadQuoteContent(std::string* FromWhat, std::string What, std::string* ToWhat)
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


//https://blog.csdn.net/MOU_IT/article/details/96457666
//https://everything.curl.dev/examples/get
//URL ,file name, headers
bool weather::Get_CURL(std::string url_cURL, std::string _FName, std::vector<std::string>* Headers_fun)
{
    std::string FileDAN = Get_DataLocation() + "\\" + _FName;
    FILE* fcw;
    if (fopen_s(&fcw, FileDAN.c_str(), "wt+"))
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
            //std::cout << clr::green << "[INFO]" << clr::reset << "Headers -> " << (*Headers_fun)[headres_i] << std::endl;
            log_write(Info, "Headers -> " + (*Headers_fun)[headres_i]);
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
                //std::cout << clr::green << "[INFO]" << clr::reset << "IP is: ";
                std::string MyIP_s = MyIP;
                log_write(Info, "IP is: " + MyIP_s);
                //std::cout << MyIP << std::endl;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            fclose(fcw);
            return TRUE;
        }
    }
    //std::cout << clr::red << "[ERROR]" << clr::reset << "curlcode is :" << curl << std::endl;
    log_write(Err, "curlcode is " + res_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    fclose(fcw);

    return FALSE;
}

void weather::Store_Headers_Getdata(std::vector<std::string>* Headers_Getdata_Fun) {

    (*Headers_Getdata_Fun).push_back("Accept: */*");
    //   (*Headers_Getdata_Fun).push_back("User - Agent: Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 121.0.0.0 Safari / 537.36 Edg / 121.0.0.0");
   //    (*Headers_Getdata_Fun).push_back("Cookie: f_city=%E6%88%90%E9%83%BD%7C101270101%7C");
    (*Headers_Getdata_Fun).push_back("User-Agent:cURL yc_GetWeather/1.0.0");
    (*Headers_Getdata_Fun).push_back("Referer:http://www.weather.com.cn/");
    (*Headers_Getdata_Fun).push_back("Connection: keep-alive");
    (*Headers_Getdata_Fun).push_back("Accept-Language: zh - CN, zh; q = 0.9, en; q = 0.8, en - GB; q = 0.7, en - US; q = 0.6");
}