# 云寸 开发指南  

## 目录

 **[ 云寸介绍 ### ](#yc)**  
 **[ Weather 介绍 ### ](#Weather)**  
 **[ WebPage 介绍 ### ](#WebPage)**  
 **[ WeatherException 介绍 ### ](#1)**  
 **[ Tools 介绍 ### ](#1)**  


## <a id="yc">云寸介绍</a>  


## <a id="WebPage">WebPage介绍</a> 

###  WebPage类 
WebPage 是用于储存网页文件的类。   
(默认构造函数)

#### 方法  

#####  
   	void readWebPage(std::string fileName, Type contentType, DocType doctype, std::string url, Encoding encoding = Encoding::utf8);  

从文件中读取，转化为WebPage  
**fileName**: 文件相对于 DataPath(...\yc\)的路径  
**contentType**: 文档类型(参见[Type](#Type))  
**doctype**: 文档格式(参见[DocType](#DocType))  
**url**: 网页网址  
**encoding**: 文件编码格式  

#####  
	void makeWebPage(std::string* source, Type pageType, DocType doctype, std::string url, Encoding _encoding = Encoding::utf8);  

从string中读取，转化为WebPage  

#####  
	void makeContent();  
	void makeContent(std::string fileName); 
	
只应在 freeContent() 释放内存后，需要内容时调用。

#####  
	void freeContent();   
清空网页内容，释放内存。  
*注意*：又需要WebPage内容时请调用 makeContent()。  