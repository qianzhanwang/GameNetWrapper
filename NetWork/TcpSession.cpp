#include "TcpSession.h"
#include "NetEventLoop.h"
#include "../Shared/GSAssert.h"
#include <algorithm>

TcpSession::TcpSession(NetEventLoop& loop, std::uint32_t id, 
	error_handler ev_err, connect_handler ev_con, 
	disconnect_handler ev_dicon, readable_handler ev_con_read)
	: m_socket(loop.get_service())
	, m_loop(loop)
	, m_sessionid(id)
	, m_cb_conn(ev_con)
	, m_cb_disconn(ev_dicon)
	, m_cb_error(ev_err)
	, m_cb_readable(ev_con_read)
	, m_send_tmp(4*1024)
	, m_recv_tmp(4*1024)
	, m_lastrecvtime(0)
{

}

TcpSession::~TcpSession()
{
	stop();
}

// ����
void TcpSession::start()
{
	// Ͷ�ݶ��¼�
	push_read_task();

	// Ͷ��д�¼�
	push_write_task();
}

// �ر�
void TcpSession::stop()
{
	if (m_state == ss_run)
	{
		m_socket.shutdown(socket_base::shutdown_both);
		m_socket.close();
	}
	m_state = ss_stop;
}

// ����Ŀ��
bool TcpSession::async_connect(tcp::endpoint& ep)
{
	_Assert(m_state == ss_stop, "���ڿ���״̬");
	_Assert(m_socket.is_open() == false, "socket�Ѿ�����");
	if (m_state == ss_stop || m_socket.is_open() == false)
	{
		return false; 
	}
	m_state = ss_connecting;

	//
	m_socket.async_connect(ep, [&](boost::system::error_code er){
		if (er)
		{
			if (m_cb_error)
			{
				m_cb_error(shared_from_this(), std::move(er.message()) );
			}
			return;
		}

		// ���ӳɹ�
		m_state = ss_run;
		if (m_cb_conn)
		{
			m_cb_conn(shared_from_this());
		}
	});

	return true;
}

// ����Э��
bool TcpSession::async_send(int msgid, google::protobuf::MessageLite* pmsg)
{
	return true;
}

// ȡͷЭ��
MsgPacket TcpSession::pop_recv()
{
	MsgPacket res;
	return std::move(res);
}

void TcpSession::push_read_task()
{
	if (m_state != ss_run)
	{
		assert(false);
		return;
	}

	char* pbuf = m_recv_tmp.data_writeable();
	std::size_t len = m_recv_tmp.writeable_size();
	if (len == 0)
	{
		assert(false);
		return;
	}

	m_socket.async_read_some(buffer(pbuf, len), [&, this](boost::system::error_code er, std::size_t len){
		if (er)
		{
			if (m_cb_error)
			{
				m_cb_error(shared_from_this(), std::move(er.message()) );
			}

			if (er != boost::asio::error::operation_aborted)
			{
				if (m_cb_disconn)
				{
					m_cb_disconn(shared_from_this());
				}
			}
		}
		else
		{
			if (len == 0)
			{
				if (m_cb_disconn)
				{
					m_cb_disconn(shared_from_this());
				}
				return;
			}

			// �����ѻ�ȡ
			m_recv_tmp.append_outside(len, [](char*, std::size_t){; });

			// ��ӵ�Э�����
			on_get_data();

			// Ͷ���µĶ�ȡ����
			push_read_task();
		}
	});
}

// Ͷ��д����
void TcpSession::push_write_task()
{
	if (m_state != ss_run)
	{
		assert(false);
		return;
	}

	// ������Ҫ���͵�����
	pre_send_data();
	const char* pbuf = m_send_tmp.data_readable();
	std::size_t len = m_send_tmp.readable_size();
	if (len == 0)
	{
		// 50ms���ӳټ��
		deadline_timer t(m_loop.get_service(), boost::posix_time::milliseconds(50));
		t.async_wait([&](boost::system::error_code er){
			push_write_task();
		});
		return;
	}

	// ��������
	m_socket.async_write_some(buffer(pbuf, len), [&, this](boost::system::error_code er, std::size_t len){
		if (er)
		{
			if (m_cb_error)
			{
				m_cb_error(shared_from_this(),er.message());
			}

			if (er != boost::asio::error::operation_aborted)
			{
				if (m_cb_disconn)
				{
					m_cb_disconn(shared_from_this());
				}
			}
		}
		else
		{
			// �ѷ�������
			m_send_tmp.pop_front(len);

			// Ͷ�ݷ����¼�
			push_write_task();
		}
	});
}


// ���ܵ�����
void TcpSession::on_get_data()
{
	for (;;)
	{
		const char* psrc = m_recv_tmp.data_readable();
		std::size_t len = m_recv_tmp.readable_size();
		if (len == 0)
		{
			break;
		}

		bool push_new = false;
		std::size_t len_push = len;
		bool push = m_RecvQueue.PushData(psrc, len_push, push_new);
		if (push)
		{
			m_recv_tmp.pop_front(len_push);

			// ��Э��ڵ�ɶ�
			if (push_new && m_cb_readable)
			{
				m_cb_readable(shared_from_this());
			}
		}
		else
		{
			break;
		}
	}
}

// ��֯��������
void TcpSession::pre_send_data()
{
	for (;;)
	{
		MsgSptr msg = m_SendQueue.ReadableFront();
		if (!msg)
		{
			break;;
		}

		std::size_t dst_len = m_send_tmp.writeable_size();
		std::size_t push_len = (std::min)(dst_len, msg->m_buf.readable_size());
		if (push_len == 0)
		{
			_Assert(push_len == 0, "���ͻ����þ�");
			break;
		}
		m_send_tmp.append(msg->m_buf.data_readable(), push_len);
		msg->m_buf.pop_front(push_len);
		msg->m_curlen += push_len;
	}
}