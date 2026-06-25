#include "pch.h"
#include "Tools.h"


// ==================== 异步日志系统 — 内部基础设施 ====================
// 匿名命名空间：所有组件仅在 Tools.cpp 内可见，避免外部误操作
namespace {
	// ---- 线程同步原语 ----
	std::mutex              logMutex;            // 保护 logQueue 的互斥锁
	std::condition_variable logCV;               // 条件变量：队列有数据或系统停止时唤醒后台线程

	// ---- 数据通道 ----
	std::queue<std::string> logQueue;            // 生产者(log_write) → 消费者(logFlusherFunc) 的队列

	// ---- 文件输出 ----
	std::ofstream           logFileStream;       // 日志文件流（InitLogSystem 打开，ShutdownLogSystem 关闭）

	// ---- 后台线程 ----
	std::jthread            logFlusherThread;    // C++20 jthread：析构时自动 request_stop() + join()
	std::atomic<bool>       logSystemRunning{ false }; // 标记系统是否已初始化，防止未初始化时写队列

	// ==================== logFlusherFunc — 后台刷写线程函数 ====================
	// 职责：循环等待 logQueue 中有数据，然后批量取出写入文件。
	// 在收到 stop_token 停止信号后，完成最后一次刷写后退出。
	// 参数 stoken：std::jthread 自动传入的停止令牌，析构 / request_stop() 时触发。
	void logFlusherFunc(std::stop_token stoken)
	{
		// 批量缓冲区：一次性从队列中取出多条日志，减少加锁次数
		std::vector<std::string> batch;
		batch.reserve(64);

		// 主循环：只要没收到停止信号就一直运行
		while (!stoken.stop_requested())
		{
			{
				// ---- 加锁，等待数据或停止信号 ----
				std::unique_lock<std::mutex> lock(logMutex);

				// wait_for 的优势：即使没有被 notify，也会每 500ms 自动醒来检查一次，
				// 防止 notify_one() 丢失导致日志迟迟不刷写（双重保险）
				logCV.wait_for(lock, std::chrono::milliseconds(500),
					[&] {
						// 唤醒条件：队列非空 OR 外部请求停止
						return !logQueue.empty() || stoken.stop_requested();
					});

				// ---- 批量取出：一次性转移所有待写入条目 ----
				// 使用 std::move 避免字符串拷贝（队列中条目被掏空，所有权转给 batch）
				while (!logQueue.empty())
				{
					batch.push_back(std::move(logQueue.front()));
					logQueue.pop();
				}
			} // 锁在此处释放 —— 文件 I/O 在锁外进行，不阻塞生产者

			// ---- 无锁写入文件 ----
			for (const auto& line : batch)
			{
				if (logFileStream.is_open())
					logFileStream << line << '\n';
			}
			batch.clear();

			// 每次批量写入后 flush：保证日志在程序崩溃时已写出到磁盘
			if (logFileStream.is_open())
				logFileStream.flush();
		}

		// ---- 收到停止信号后的最终刷写 ----
		// 兜底：把可能残留在队列中的日志全部写出
		{
			std::unique_lock<std::mutex> lock(logMutex);
			while (!logQueue.empty())
			{
				if (logFileStream.is_open())
					logFileStream << logQueue.front() << '\n';
				logQueue.pop();
			}
		}
		// 最后一次 flush，确保所有数据落盘
		if (logFileStream.is_open())
			logFileStream.flush();
	}

} // anonymous namespace


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
	using namespace yc::ycresult;
	std::ifstream file;
	file.open(Path, std::ios::in);
	if (file.is_open() == false) {
		throw ycresult(ExcptType::file, AddressMethod::back, "Cannot open the file!");
	}
	//open

	size_t cnt = file.gcount();
	if (cnt >= 655'360) throw ycresult(ExcptType::content_too_long, AddressMethod::back, "file is too big!");
	//check size
	//ref https://cplusplus.com/reference/fstream/ifstream/

	(*str).clear();
	char buff;
	while (file.get(buff)) (*str).push_back(buff);
}

void Tools::log_write(int lvl, std::string writelog, std::string region, InfoSet info, std::vector<std::string> call) {
	Tools::log_write(lvl, writelog, region);
}

void Tools::log_write(int lvl, std::string writelog, std::string region, InfoSet info) {
	Tools::log_write(lvl,  writelog, region);
}

void Tools::log_write(int lvl, std::string writelog, std::string region)
{

	//auto flt = now.time_since_epoch().count() % (std::micro::den * 10);
	//auto time_ = std::format("{:%F %H:%M:%S}.{:01d} ", now_, flt / 1000000);

	auto now = std::chrono::floor<std::chrono::seconds>(
		std::chrono::system_clock::now());
	std::string time_ = std::format(
		"{:%F %H:%M:%S}",
		now);



	// ---- 级别标签（控制台和文件共用） ----
	std::string loglvl;
	switch (lvl)
	{
	case Err:   loglvl = "[ERROR]";       break;
	case Warn:  loglvl = "[WARNING]";     break;
	case Info:  loglvl = "[INFO]";        break;
	case FErr:  loglvl = "[FATAL ERROR]"; break;
	default:    loglvl = "[UNKNOWN]";     break;
	}
	loglvl.insert(0, time_);

#ifdef _CONSOLE  //控制台程序使用

	clr colorShow = clr::on_red;
	switch (lvl)
	{
	case Err:   colorShow = clr::red;     break;
	case Warn:  colorShow = clr::yellow;  break;
	case Info:  colorShow = clr::green;   break;
	case FErr:  colorShow = clr::on_red;  break;
	default:    colorShow = clr::blue;    break;
	}

#ifndef _DEBUG  //display when not debug 调试时才显示	
	if (colorShow != clr::green)
	{
		std::cout << colorShow << loglvl << clr::reset;
		if (!region.empty())
		{
			std::cout << clr::white << " |[" + region + "]| " << clr::reset;
		}
		std::cout << writelog << std::endl;
	}
#endif // !_DEBUG

#ifdef _DEBUG  //display when debug 调试时才显示	
	std::cout << colorShow << loglvl << clr::reset;
	if (!region.empty())
	{
		std::cout << clr::white << " |[" + region + "]| " << clr::reset;
	}
	std::cout << writelog << std::endl;
#endif // _DEBUG

#endif // _CONSOLE

	// ---- 文件写入（异步队列，独立于 _CONSOLE 宏） ----
	// logSystemRunning 由 InitLogSystem() 设为 true，未初始化时跳过，防止崩溃
	if (logSystemRunning.load(std::memory_order_acquire))
	{
		// 构建无颜色码的纯文本行，格式与控制台输出一致
		// 格式: "2025-03-29 14:30:15.3 [INFO] |[Region]| message"
		std::string fileLine = loglvl;   // 已含时间戳 + 级别标签
		if (!region.empty())
		{
			fileLine += " |[" + region + "]| ";
		}
		fileLine += writelog;

		// 入队：加锁 push + notify，不阻塞主逻辑
		{
			std::lock_guard<std::mutex> lock(logMutex);
			logQueue.push(std::move(fileLine));
		}
		logCV.notify_one();   // 唤醒后台线程准备刷写
	}

}


//Check whether %AppData%/yc exists and create it.
void Tools::CheckIfDataFileExists() {
	DataLocation = getRoamingAppDataPath() + "\\yc";
	std::filesystem::path Path_DataLocation = DataLocation;
	if (!std::filesystem::exists(Path_DataLocation)) {
		if (!std::filesystem::create_directories(Path_DataLocation)) {
			//std::cout << clr::red << "[ERROR]" << clr::reset << "无法储存数据文件。创建%AppData%/yc失败。\n";
			log_write(Err, "无法储存数据文件。创建%AppData%/yc失败。", "Tools::CheckIfDataFileExists");
			DataLocation = "\\yc";
			std::filesystem::create_directories(Path_DataLocation);
		}
	}

	if (!std::filesystem::exists(Path_DataLocation))
	{
		log_write(FErr, "无法储存数据文件。再尝试在本地目录创建\\yc失败。", "Tools::CheckIfDataFileExists");
		exit(0);
	}

	log_write(Info, "Data location is:" + DataLocation, "CheckIfDataFileExists");
}

std::string Tools::Get_DataLocation()
{
	return DataLocation;
}

// ==================== InitLogSystem — 启动异步日志系统 ====================
// 职责：
//   1. 在 %AppData%/yc/log/ 下创建日志目录（如不存在）
//   2. 以当前时间生成唯一日志文件名：yc_log_2025-03-29_14-30-15.log
//   3. 打开文件流（std::ofstream），写入会话启动标记
//   4. 启动后台刷写线程（std::jthread）
// 调用时机：CheckIfDataFileExists() 之后，任何 log_write() 调用之前。
// 异常：若目录创建或文件打开失败，抛出 ycresult::file 异常。
void Tools::InitLogSystem()
{
	using namespace yc::ycresult;

	// ---- 1. 确保日志子目录存在：DataLocation\log\ ----
	std::string logDir = DataLocation + "\\log";
	std::filesystem::path logPath{ logDir };
	if (!std::filesystem::exists(logPath))
	{
		if (!std::filesystem::create_directories(logPath))
		{
			// 目录创建失败属于严重问题，抛出异常交由上层处理
			throw ycresult(ExcptType::file, AddressMethod::back,
				"Failed to create log directory: " + logDir);
		}
	}

	// ---- 2. 生成时间戳文件名 ----
	// 精确到秒，每次运行唯一：yc_log_2025-03-29_14-30-15.log
	auto now       = std::chrono::system_clock::now();
	auto nowSec    = std::chrono::floor<std::chrono::seconds>(now);
	std::string timestampFileName = std::format("{:%F_%H-%M-%S}", nowSec);
	std::string logFilePath       = logDir + "\\yc_log_" + timestampFileName + ".log";

	// ---- 3. 打开文件（追加模式） ----
	logFileStream.open(logFilePath, std::ios::app);
	if (!logFileStream.is_open())
	{
		throw ycresult(ExcptType::file, AddressMethod::back,
			"Cannot open log file: " + logFilePath);
	}

	// ---- 4. 写入会话启动标记 ----
	// 方便在日志文件中区分不同运行实例
	std::string startLine = std::format(
		"{:%F %H:%M:%S}[SYSTEM] ======== Session Start (v{}) ========",
		nowSec, YC_GETWEATHER_CORE_VERSION);
	logFileStream << startLine << '\n';
	logFileStream.flush();  // 立即落盘，确保启动标记不丢失

	// ---- 5. 标记系统就绪，启动后台刷写线程 ----
	logFlusherThread = std::jthread(logFlusherFunc);
	logSystemRunning.store(true);

	// 直接输出到 cout（此时日志系统刚启动，用 log_write 也可，但避免对自身产生依赖）
	std::cout << "[SYSTEM] Log file: " << logFilePath << '\n';
}


// ==================== ShutdownLogSystem — 停止异步日志系统 ====================
// 职责：
//   1. 向队列中推入一条结束标记行
//   2. 通知后台线程退出（request_stop）
//   3. 等待后台线程完成最终刷写（join）
//   4. 关闭日志文件流
// 调用时机：程序退出前（return / exit 之前）。
// 若 InitLogSystem() 未被调用，则 logSystemRunning 为 false，函数立即返回。
void Tools::ShutdownLogSystem()
{
	if (!logSystemRunning.load())
		return;     // 未初始化，无需关闭，直接返回

	// ---- 1. 写入会话结束标记 ----
	{
		std::lock_guard<std::mutex> lock(logMutex);
		auto now = std::chrono::floor<std::chrono::seconds>(
			std::chrono::system_clock::now());
		std::string endLine = std::format(
			"{:%F %H:%M:%S}[SYSTEM] ======== Session End ========",
			now);
		logQueue.push(std::move(endLine));
	}
	logCV.notify_one();   // 唤醒后台线程，使其注意到结束标记
	logCV.notify_one();   

	// ---- 2. 请求后台线程停止 ----
	logFlusherThread.request_stop();   // 设置 stop_token 为已停止状态
	logCV.notify_one();                // 再次唤醒，确保线程从 wait_for 中退出

	// 等待后台线程完成最终刷写并退出。由于后台线程在退出前会把队列中
	// 的剩余日志写入文件并 flush，所以在 join 之后可以安全关闭文件。
	if (logFlusherThread.joinable()) {
		try {
			logFlusherThread.join();
		}
		catch (...) {
			// 在析构/退出阶段，吞掉任何异常以避免二次错误
		}
	}

	// ---- 3. 关闭文件 ----
	if (logFileStream.is_open())
		logFileStream.close();

	// ---- 4. 重置状态 ----
	logSystemRunning.store(false);
}


Tools::InfoSet::InfoSet(std::initializer_list<Detail> info)
{
	for (const auto& detail : info)
	{
		details_.emplace_back(detail);
	}
}

Tools::InfoSet::~InfoSet()
{}

std::string Tools::InfoSet::ToString() const
{
	std::string out;
	bool first = true;
	for (const auto& d : details_)
	{
		if (!first)
			out += "; ";
		first = false;
		out += d.getName();
		out += ":";
		out += d.getKey();
	}
	return out;
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
	log_write(Info, "Current Time is: " + STR_CurrentTime + " .", "String_GetCurrentTime");
	return STR_CurrentTime;

}

