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
	// ��������
	bool start(std::uint32_t port);

	// �߼��߳�
	void porcess();

	// �߳�����
	void kick_session(std::uint32_t sid);

	// ����Э��
	void send_msg(std::uint32_t sid, google::protobuf::MessageLite* pmsg);

private:
	// ����
	bool listen(std::uint32_t port);

	// ����������
	void accept();

	// �¼����������ر�
	void event_close();

	// �����session
	void event_new_session(tcp::socket& sk);

	// ��һ��ID
	std::uint32_t next_session_id();

	// session ����
	void event_session_error(TcpSession::TcpSessionWptr session, std::string str_er);

	// session �Ͽ�����
	void event_session_disconnect(TcpSession::TcpSessionWptr session);

	// session �ɶ�
	void event_readable(TcpSession::TcpSessionWptr session);

//////////////////////////////////////////////////////////////////////////
// ���̵߳����¼�
protected:
	// ������Ϣ�������־
	virtual void fun_error(const std::string& str_er);

	// �����ӽ���
	virtual void fun_session_new(TcpSession::TcpSessionWptr session);

	// ���ӶϿ��¼�
	virtual void fun_session_kick(std::uint32_t sid);

	// �ɶ��¼�
	virtual void fun_session_readable(std::uint32_t sid);
	
private:
	NetEventLoop& m_loop;
	tcp::acceptor m_accept;
	tcp::socket m_socket;
	STcpSessionMap m_conns;			// ��������<sid, session>������IO�߳�
	WTcpSessionMap m_map_session;	// session�б�,�����߼��߳�<sid, session>

	std::vector<std::function<void()>> m_events;	// �߼��̣߳��¼�����
	std::mutex m_mtx_ev;							// ���ӻ��滥����
};

#endif