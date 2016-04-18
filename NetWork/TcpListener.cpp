#include "TcpListener.h"
#include "NetEventLoop.h"
#include "../Shared/GSAssert.h"
#include <iostream>
#include <fstream>
#include "..\MsgDef\MsgBuild.h"

TcpListener::TcpListener(NetEventLoop& loop)
	: m_loop(loop)
	, m_accept(loop.get_service())
	, m_socket(loop.get_service())
{
	// 注册关闭事件
	loop.push_watch_close(std::bind(&TcpListener::event_close, this));
}


TcpListener::~TcpListener()
{
}

// 开启监听
bool TcpListener::start(std::uint32_t port)
{
	if (listen(port) == false)
	{
		_Assert(false, "监听失败");
		return false;
	}

	accept();
	return true;
}

// 逻辑线程
void TcpListener::porcess()
{
	// 取得缓存的链接
	std::unique_lock<std::mutex> ul(m_mtx_ev);
	std::vector<std::function<void()>> tmp_events;
	tmp_events.swap(m_events);
	ul.unlock();

	// 执行事件队列
	for (auto& fun : tmp_events)
	{
		if (fun)
		{
			fun();
		}
	}
}

// 处理新链接
void TcpListener::accept()
{
	m_accept.async_accept(m_socket, [&, this](const boost::system::error_code& er){
		if (er)
		{
			fun_error(er.message());
		}

		if (!m_accept.is_open())
		{
			return;
		}

		if (!er)
		{
			event_new_session(m_socket);
		}
		accept();
	});
}

// 监听
bool TcpListener::listen(std::uint32_t port)
{
	try
	{
		boost::asio::ip::tcp::acceptor acceptor(m_loop.get_service());
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
		m_accept.open(endpoint.protocol());
		m_accept.bind(endpoint);
		m_accept.listen();
	}
	catch (boost::system::system_error er)
	{
		std::cout << "Error:" << er.what() << std::endl;
		return false;
	}
	return true;
}

// 下一个ID
std::uint32_t TcpListener::next_session_id()
{
	static std::uint32_t ids = 0;
	return ++ids;
}

// 添加新session
void TcpListener::event_new_session(tcp::socket& sk)
{
	TcpSession::TcpSessionSptr session = std::make_shared<TcpSession>(m_loop, next_session_id(),
		std::bind(&TcpListener::event_session_error, this, std::placeholders::_1, std::placeholders::_2),
		nullptr,
		std::bind(&TcpListener::event_session_disconnect, this, std::placeholders::_1),
		std::bind(&TcpListener::event_readable, this, std::placeholders::_1));

	// 添加到列表
	bool succ = m_map_session.insert(std::make_pair(session->sid(), session)).second;
	if (succ == false)
	{
		session->stop();
		_Assert(false, "添加新session失败");
		return;
	}

	// 开启session
	session->start();

	// 添加到事件列表
	std::unique_lock<std::mutex> ul(m_mtx_ev);
	m_events.push_back(std::bind(&TcpListener::fun_session_new, this, session));
	ul.unlock();
}

// session 报错
void TcpListener::event_session_error(TcpSession::TcpSessionWptr session, std::string str_er)
{
	// 添加到事件列表
	std::unique_lock<std::mutex> ul(m_mtx_ev);
	m_events.push_back(std::bind(&TcpListener::fun_error, this, str_er));
	ul.unlock();
}

// session 断开连接
void TcpListener::event_session_disconnect(TcpSession::TcpSessionWptr session)
{
	auto sptr = std::move(session.lock());
	if (!sptr)
	{
		return;
	}

	// 关闭，删除
	std::uint32_t sid = sptr->sid();
	sptr->stop();
	m_conns.erase(sid);

	// 添加到事件列表
	std::unique_lock<std::mutex> ul(m_mtx_ev);
	m_events.push_back(std::bind(&TcpListener::fun_session_kick, this, sid));
	ul.unlock();
}

// session 可读
void TcpListener::event_readable(TcpSession::TcpSessionWptr session)
{
	auto sptr = std::move(session.lock());
	if (!sptr)
	{
		return;
	}
	std::uint32_t sid = sptr->sid();

	// 添加到事件列表
	std::unique_lock<std::mutex> ul(m_mtx_ev);
	m_events.push_back(std::bind(&TcpListener::fun_session_readable, this, sid));
	ul.unlock();
}

// 事件：服务器关闭
void TcpListener::event_close()
{
	std::cout << "event_close" << std::endl;
	std::ofstream ofs("1.txt");
	ofs << "hello " << std::endl;
	ofs.flush();
	ofs.close();
}

// 错误信息输出到日志
void TcpListener::fun_error(const std::string&)
{

}

// 新链接建立
void TcpListener::fun_session_new(TcpSession::TcpSessionWptr)
{

}

// 链接断开
void TcpListener::fun_session_kick(std::uint32_t sid)
{
}

// 可读事件
void TcpListener::fun_session_readable(std::uint32_t sid)
{

}


// 踢出链接
void TcpListener::kick_session(std::uint32_t sid)
{
	m_loop.push_event([&]{
		auto iter = m_conns.find(sid);
		if (iter == m_conns.end())
		{
			return;
		}
		event_session_disconnect(iter->second);
	});
}

// 发送协议
void TcpListener::send_msg(std::uint32_t sid, google::protobuf::MessageLite* pmsg)
{
	auto iter = m_map_session.find(sid);
	if (iter == m_map_session.end())
	{
		return;
	}
	TcpSession::TcpSessionSptr sptr = std::move(iter->second.lock());
	if (!sptr)
	{
		return;
	}

	unsigned int id = (unsigned int)TSingleton<MsgBuildTool>::GetInstance().GetMsgID(pmsg);
	if (id == 0)
	{
		_Assert(false, "无效的协议");
		return;
	}
	sptr->async_send(id, pmsg);
}