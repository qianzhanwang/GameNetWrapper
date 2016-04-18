#pragma once
#ifndef TCPSESSION_H
#define TCPSESSION_H

#include "../MsgPackQueue/MsgPackQueue.h"
#include "google\protobuf\stubs\common.h"
#include <functional>
#include <boost/asio/ip/tcp.hpp>
using namespace boost::asio::ip;


/*
* �Ự����
*	1��socket����
*	2��loopע��
*	3���շ���Ϣ����
*/
class NetEventLoop;
class TcpSession : public std::enable_shared_from_this<TcpSession>
{
public:
	enum SessionState
	{
		ss_stop = 0,	// �ر�״̬
		ss_connecting,	// ������
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
	// ����
	void start();

	// �ر�
	void stop();

	// ����Ŀ��
	bool async_connect(tcp::endpoint& ep);

	// ����Э��
	bool async_send(int msgid, google::protobuf::MessageLite* pmsg);

	// ȡͷЭ��
	MsgPacket pop_recv();

	// ȡ��sid
	std::uint32_t sid(){ return m_sessionid; };

private:
	// Ͷ�ݶ�����
	void push_read_task();

	// Ͷ��д����
	void push_write_task();

	// ���ܵ�����
	void on_get_data();

	// ��֯��������
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
