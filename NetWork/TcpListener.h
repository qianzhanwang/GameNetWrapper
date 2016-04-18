#pragma once

#ifndef TCPLISTENER_H
#define TCPLISTENER_H


#include "TcpSession.h"
#include <boost/asio/ip/tcp.hpp>
using namespace boost::asio::ip;

class NetEventLoop;
class TcpListener
{
public:
	typedef std::map<std::uint32_t, TcpSession::TcpSessionSptr> STcpSessionMap;
	typedef std::map<std::uint32_t, TcpSession::TcpSessionWptr> WTcpSessionMap;

	TcpListener(NetEventLoop& loop);
	~TcpListener();

public:
	// 开启监听
	bool start(std::uint32_t port);

	// 逻辑线程
	void porcess();

	// 踢出链接
	void kick_session(std::uint32_t sid);

	// 发送协议
	void send_msg(std::uint32_t sid, google::protobuf::MessageLite* pmsg);

private:
	// 监听
	bool listen(std::uint32_t port);

	// 处理新链接
	void accept();

	// 事件：服务器关闭
	void event_close();

	// 添加新session
	void event_new_session(tcp::socket& sk);

	// 下一个ID
	std::uint32_t next_session_id();

	// session 报错
	void event_session_error(TcpSession::TcpSessionWptr session, std::string str_er);

	// session 断开连接
	void event_session_disconnect(TcpSession::TcpSessionWptr session);

	// session 可读
	void event_readable(TcpSession::TcpSessionWptr session);

//////////////////////////////////////////////////////////////////////////
// 主线程调用事件
protected:
	// 错误信息输出到日志
	virtual void fun_error(const std::string& str_er);

	// 新链接建立
	virtual void fun_session_new(TcpSession::TcpSessionWptr session);

	// 链接断开事件
	virtual void fun_session_kick(std::uint32_t sid);

	// 可读事件
	virtual void fun_session_readable(std::uint32_t sid);
	
private:
	NetEventLoop& m_loop;
	tcp::acceptor m_accept;
	tcp::socket m_socket;
	STcpSessionMap m_conns;			// 所有连接<sid, session>，用于IO线程
	WTcpSessionMap m_map_session;	// session列表,用于逻辑线程<sid, session>

	std::vector<std::function<void()>> m_events;	// 逻辑线程，事件队列
	std::mutex m_mtx_ev;							// 链接缓存互斥锁
};

#endif