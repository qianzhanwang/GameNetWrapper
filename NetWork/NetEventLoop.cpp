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

// ȡ��cpu ����
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
// ��ʼ��
void NetEventLoop::init()
{
	_Assert(m_state == es_stop, "״̬����");
	if (m_state != es_stop)
	{
		return;
	}

	int thread_num = 1;

#ifdef _WIN32
	// windows ��ʹ�ö��߳�
	thread_num = get_cpu_num();
#endif

	// ��Ӵ����߳�
	_Assert(m_threads.size() == 0, "�߳�Ϊ0");
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

	// ע��ر��¼�
	m_signals.add(SIGINT);
	m_signals.add(SIGTERM);
#if defined(SIGQUIT)
	m_signals.add(SIGQUIT);
#endif
	do_wait_stop();
}

// �ر�
void NetEventLoop::close()
{
	if (m_state != es_run)
	{
		return;
	}
	std::cout << "close..." << std::endl;

	// �����״̬
	m_state = es_stoping;

	// ִ�����йر��¼�
	for (auto cb_close : m_watch_close)
	{
		cb_close();
	}

	// �ر�loop
	close_loop();
}

// �ر�loop
void NetEventLoop::close_loop()
{
	_Assert(m_state == es_stoping, "�ظ��ر�loop");
	m_work.reset();
	if (m_service.stopped()==false)
	{
		m_service.stop();
	}
	m_state = es_stop;
}

// Ͷ���¼�
bool NetEventLoop::push_event(std::function<void()> fun)
{
	if (m_state != es_run)
	{
		_Assert(false, "Ͷ��ʱ��������״̬");
		return false;
	}
	m_service.post(fun);
	return true;
}

// ���ӹر��¼�
void NetEventLoop::push_watch_close(close_handler watch_close)
{
	m_watch_close.push_back(watch_close);
}

// ע��ر�ʱ��
void NetEventLoop::do_wait_stop()
{
	m_signals.async_wait([this](boost::system::error_code, int){
		close();
	});
}