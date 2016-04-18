#pragma once
#ifndef NETEVENTLOOP_H
#define NETEVENTLOOP_H

// 开启asio 日志
#define BOOST_ASIO_ENABLE_HANDLER_TRACKING
#include <thread>
#include <memory>
#include <vector>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include "../Shared/Singleton.h"
using namespace boost::asio;


/*
* 网络IO
*	使用boost::asio, linux使用单线程处理, windows下使用多线程
*	具体socket操作，放到session中执行
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
	// 初始化
	void init();

	// 关闭
	void close();
	
	// 投递事件
	bool push_event(std::function<void()> fun);

	// 取得io
	io_service& get_service(){ return m_service; };

	// 监视关闭事件
	void push_watch_close(close_handler watch_close);

private:
	// 注册关闭时间
	void do_wait_stop();

	// 关闭loop
	void close_loop();

	// 取得cpu 数量
	int get_cpu_num();

private:
	EState m_state;
	io_service m_service;
	std::shared_ptr<io_service::work> m_work;
	thread_sptr_vector m_threads; 
	session_sptr_vector m_sessions;
	std::vector<close_handler> m_watch_close;	// 关闭时，通知其他模块
	boost::asio::signal_set m_signals;			// 监控关机事件
};

#define  g_loop TSingleton<NetEventLoop>::GetInstance()

#endif

