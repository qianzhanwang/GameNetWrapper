#pragma once
#ifndef TCPSESSION_H
#define TCPSESSION_H

#include "../MsgPackQueue/MsgPackQueue.h"
#include "google\protobuf\stubs\common.h"
#include <functional>
#include <boost/asio/ip/tcp.hpp>
using namespace boost::asio::ip;


/*
* 会话链接
*	1、socket链接
*	2、loop注册
*	3、收发消息队列
*/
class NetEventLoop;
class TcpSession : public std::enable_shared_from_this<TcpSession>
{
public:
	enum SessionState
	{
		ss_stop = 0,	// 关闭状态
		ss_connecting,	// 链接中
		ss_run,			// 
	};

	typedef std::shared_ptr<TcpSession> TcpSessionSptr;
	typedef std::weak_ptr<TcpSession> TcpSessionWptr;
	typedef std::function<void(TcpSessionWptr, std::string err)> error_handler;
	typedef std::function<void(TcpSessionWptr)> connect_handler;
	typedef std::function<void(TcpSessionWptr)> disconnect_handler;
	typedef std::function<void(TcpSessionWptr)> readable_handler;

	TcpSession(NetEventLoop& loop, std::uint32_t id, error_handler ev_err, connect_handler ev_con, disconnect_handler ev_dicon, readable_handler ev_con_read);
	~TcpSession();
	TcpSession(const TcpSession&) = delete;
	TcpSession& operator=(const TcpSession&) = delete;

public:
	// 开启
	void start();

	// 关闭
	void stop();

	// 链接目标
	bool async_connect(tcp::endpoint& ep);

	// 发送协议
	bool async_send(int msgid, google::protobuf::MessageLite* pmsg);

	// 取头协议
	MsgPacket pop_recv();

	// 取得sid
	std::uint32_t sid(){ return m_sessionid; };

private:
	// 投递读任务
	void push_read_task();

	// 投递写任务
	void push_write_task();

	// 接受到数据
	void on_get_data();

	// 组织发送数据
	void pre_send_data();

private:
	std::uint32_t	m_sessionid; // session id
	SessionState	m_state;
	tcp::socket		m_socket;
	NetEventLoop&	m_loop;

	CMsgPackQueue	m_SendQueue; 
	CMsgPackQueue	m_RecvQueue;
	VectorBuffer	m_send_tmp;
	VectorBuffer	m_recv_tmp;

	error_handler	m_cb_error;
	connect_handler m_cb_conn;
	disconnect_handler	m_cb_disconn;
	readable_handler	m_cb_readable;

	std::uint32_t	m_lastrecvtime;
};



#endif
