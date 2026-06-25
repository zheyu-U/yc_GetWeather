# 云寸  
一个控制台程序。  
get_weather的全新版本。使用C++重写。
版本信息参见 Release.md

## 功能  
输出天气信息。  
通过爬取中国天气网([weather.com.cn](http://weather.com.cn))的天气数据，并加以分析。  
程序通过您当前**IP**地址判断地理位置，然后爬取对应城市信息。  

## 使用
### Release直接运行
仅支持Windows平台，建议Windows10以上。  
从**Releases**中下载版本，直接运行即可。  

### 当作库使用
将除了 ``` 云寸.cpp ``` 的文件包含在项目中。对于 *Windows App SDK* 或 *UWP* ，可以直接编译。**对于其他平台，请确保链接cURL库**。
  
## 参考
**爬虫** 采用curl库 ->  **[cURL](https://github.com/curl/curl)**  
**彩色输出** 采用colored-cout -> **[colored-cout](https://github.com/yurablok/colored-cout/)**


