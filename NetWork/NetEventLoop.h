#pragma once
#ifndef NETEVENTLOOP_H
#define NETEVENTLOOP_H

// ����asio ��־
#define BOOST_ASIO_ENABLE_HANDLER_TRACKING
#include <thread>
#include <memory>
#include <vector>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include "../Shared/Singleton.h"
using namespace boost::asio;


/*
* ����IO
*	ʹ��boost::asio, linuxʹ�õ��̴߳���, windows��ʹ�ö��߳�
*	����socket�������ŵ�session��ִ��
*/
class TcpSession;
class NetEventLoop
{
public:
	typedef std::shared_ptr<std::thread> thread_sptr;
	typedef std::vector<thread_sptr> thread_sptr_vector;
	typedef std::shared_ptr<TcpSession> session_sptr;
	typedef std::vector<session_sptr> session_sptr_vector;
	typedef std::function<void()> close_handler;

	enum EState
	{
		es_stop = 0,
		es_run,
		es_stoping,
	};

	NetEventLoop();
	~NetEventLoop();

public:
	// ��ʼ��
	void init();

	// �ر�
	void close();
	
	// Ͷ���¼�
	bool push_event(std::function<void()> fun);

	// ȡ��io
	io_service& get_service(){ return m_service; };

	// ���ӹر��¼�
	void push_watch_close(close_handler watch_close);

private:
	// ע��ر�ʱ��
	void do_wait_stop();

	// �ر�loop
	void close_loop();

	// ȡ��cpu ����
	int get_cpu_num();

private:
	EState m_state;
	io_service m_service;
	std::shared_ptr<io_service::work> m_work;
	thread_sptr_vector m_threads; 
	session_sptr_vector m_sessions;
	std::vector<close_handler> m_watch_close;	// �ر�ʱ��֪ͨ����ģ��
	boost::asio::signal_set m_signals;			// ��عػ��¼�
};

#define  g_loop TSingleton<NetEventLoop>::GetInstance()

#endif

