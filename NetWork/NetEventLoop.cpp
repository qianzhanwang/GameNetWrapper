#include "NetEventLoop.h"
#include "..\Shared\GSAssert.h"
#include <iostream>
#include "TcpSession.h"

NetEventLoop::NetEventLoop()
	:m_state(es_stop)
	, m_signals(m_service)
{
	io_service::work* m_pwork = new io_service::work(m_service);
	m_work.reset(m_pwork);
}

NetEventLoop::~NetEventLoop()
{
	close();

	for (auto& thsptr : m_threads)
	{
		thsptr->join();
	}
	m_threads.clear();
}

// 取得cpu 数量
int NetEventLoop::get_cpu_num()
{
#ifdef _MSC_VER
	SYSTEM_INFO siSysInfo;
	::GetSystemInfo(&siSysInfo);
	return siSysInfo.dwNumberOfProcessors;
#else
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}
// 初始化
void NetEventLoop::init()
{
	_Assert(m_state == es_stop, "状态错误");
	if (m_state != es_stop)
	{
		return;
	}

	int thread_num = 1;

#ifdef _WIN32
	// windows 中使用多线程
	thread_num = get_cpu_num();
#endif

	// 添加处理线程
	_Assert(m_threads.size() == 0, "线程为0");
	m_threads.clear();

	//
	for (int i = 0;i!= thread_num;++i)
	{
		std::shared_ptr<std::thread> th_sptr = std::shared_ptr<std::thread>(
			new std::thread([&]{
			m_service.run();
		}));
		m_threads.push_back(th_sptr);
	}
	m_state = es_run;

	// 注册关闭事件
	m_signals.add(SIGINT);
	m_signals.add(SIGTERM);
#if defined(SIGQUIT)
	m_signals.add(SIGQUIT);
#endif
	do_wait_stop();
}

// 关闭
void NetEventLoop::close()
{
	if (m_state != es_run)
	{
		return;
	}
	std::cout << "close..." << std::endl;

	// 标记新状态
	m_state = es_stoping;

	// 执行所有关闭事件
	for (auto cb_close : m_watch_close)
	{
		cb_close();
	}

	// 关闭loop
	close_loop();
}

// 关闭loop
void NetEventLoop::close_loop()
{
	_Assert(m_state == es_stoping, "重复关闭loop");
	m_work.reset();
	if (m_service.stopped()==false)
	{
		m_service.stop();
	}
	m_state = es_stop;
}

// 投递事件
bool NetEventLoop::push_event(std::function<void()> fun)
{
	if (m_state != es_run)
	{
		_Assert(false, "投递时不在运行状态");
		return false;
	}
	m_service.post(fun);
	return true;
}

// 监视关闭事件
void NetEventLoop::push_watch_close(close_handler watch_close)
{
	m_watch_close.push_back(watch_close);
}

// 注册关闭时间
void NetEventLoop::do_wait_stop()
{
	m_signals.async_wait([this](boost::system::error_code, int){
		close();
	});
}