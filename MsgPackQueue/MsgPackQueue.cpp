#include "MsgPackQueue.h"
#include "../ProtoMsg/ProtoMsg.h"
#include "../MsgDef/MsgBuild.h"
#include "../shared/GSAssert.h"
#include <algorithm>
#include <cstdint>

#pragma warning( push )
#pragma warning (disable : 4127)

CMsgPackQueue::CMsgPackQueue(void)
{
    m_nCurSize = 0;
    m_bError = false;
    m_curmsg.reset();
}


CMsgPackQueue::~CMsgPackQueue(void)
{
}


bool CMsgPackQueue::PushMsg(unsigned int msgid, ::google::protobuf::MessageLite* pMsgBody)
{
    if (msgid == 0 || pMsgBody == NULL)
    {
        return false;
    }

    // 
    std::uint32_t len_pro = pMsgBody->ByteSize();
	std::uint32_t len_id_pro = len_pro + sizeof(unsigned short);
    MsgSptr msg(new SMsgPackNode());
    msg->m_msgid = msgid;
    msg->m_sumlen = len_pro + 2 * sizeof(unsigned short);
    msg->m_buf.append((const char*)&len_id_pro, sizeof(unsigned short));
    msg->m_buf.append((const char*)&msgid, sizeof(unsigned short));
    msg->m_buf.append_outside(len_pro, [&](char* buf_new, size_t len_new)
    {
        _Assert(len_new >= len_pro,("�ڴ����ʧ��"));
        pMsgBody->SerializeToArray(buf_new, len_new);
    });
    msg->m_curlen = 0;

    // lock push 
    std::unique_lock<std::mutex> ul(m_mutex);
    m_lstMsg.push_back(msg);
    m_nCurSize = m_lstMsg.size();
    ul.unlock();
    msg.reset();
    return true;
}

// �����������
bool CMsgPackQueue::PushData(const char* pProtoBuf, unsigned int& nInOutMsgLength, bool& push_new)
{
	push_new = false;
    if (NULL == pProtoBuf || nInOutMsgLength==0)
    {
        return false;
    }

    if (!m_curmsg)
    {        
        // no len
        if (nInOutMsgLength < sizeof(unsigned short) * 2)
        {
            return false;
        }

        // init
        std::uint32_t len = *(unsigned short*)pProtoBuf;
        pProtoBuf += sizeof(unsigned short);
		std::uint32_t id = *(unsigned short*)(pProtoBuf);
        pProtoBuf += sizeof(unsigned short);
		_Assert(len != 0 && len < 64 * 1024, ("Э�鳤�ȴ��� 0 or >64k"));
        m_curmsg = MsgSptr(new SMsgPackNode());
        m_curmsg->m_sumlen = len - sizeof(unsigned short);
        m_curmsg->m_msgid = id;
        m_curmsg->m_curlen = 0;
        nInOutMsgLength -= 2 * sizeof(unsigned short);
    }

    unsigned int need = m_curmsg->m_sumlen - m_curmsg->m_curlen;
    int cp_len = need >nInOutMsgLength ? nInOutMsgLength : need;
    m_curmsg->m_buf.append(pProtoBuf, cp_len);
    m_curmsg->m_curlen += cp_len;
    nInOutMsgLength -= cp_len;

    // lock push
    if (m_curmsg->m_curlen != 0 && m_curmsg->m_curlen == m_curmsg->m_sumlen)
    {
        std::unique_lock<std::mutex> ul(m_mutex);
        m_lstMsg.push_back(m_curmsg);
        m_curmsg.reset();
        m_nCurSize = m_lstMsg.size();
		push_new = true;
    }
    return true;
}

// ȡ�ÿɶ����׽ڵ�
MsgSptr& CMsgPackQueue::ReadableFront()
{
    if (!m_curmsg || m_curmsg->m_curlen == m_curmsg->m_sumlen)
    {
        // û��Э��ɶ�
        m_curmsg.reset();
        if (m_nCurSize == 0)
        {
            return m_curmsg;
        }

        // ȡ��һ��
        std::unique_lock<std::mutex> ul(m_mutex);
        std::list<MsgSptr>::iterator iter = m_lstMsg.begin();
        if (iter != m_lstMsg.end())
        {
            m_curmsg = *iter;
            m_lstMsg.pop_front();
            m_nCurSize = m_lstMsg.size();
            ul.unlock();
        }
    }
    return m_curmsg;
}

MsgSptr CMsgPackQueue::PopFront()
{
    MsgSptr pmsg;
    std::unique_lock<std::mutex> ul(m_mutex);
    std::list<MsgSptr>::iterator iter = m_lstMsg.begin();
    if (iter != m_lstMsg.end())
    {
        pmsg = *iter;
        m_lstMsg.pop_front();
        m_nCurSize = m_lstMsg.size();
    }
    ul.unlock();
    return std::move(pmsg);
}

// �����쳣��ֹͣ����
void CMsgPackQueue::SetError()
{
    m_bError = true;
}

bool CMsgPackQueue::IsError()
{
    return m_bError == true;
}

// Э����г���
size_t CMsgPackQueue::GetSize()
{
    return m_nCurSize;
}

#pragma warning (pop)