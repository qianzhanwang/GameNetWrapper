#ifndef _H_MSGPACKQUEUE_H_
#define _H_MSGPACKQUEUE_H_

#include "../Shared/Singleton.h"
#include "../MsgDef/MsgBase.h"
#include "../Shared/VectorBuffer.h"
#include <list>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

/*
* ���нڵ�
*/
struct SMsgPackNode /*: public TObject_pool<SMsgPackNode, 500>, ʹ��tcmalloc/jemalloc���*/
{
    SMsgPackNode()
    {
        m_msgid     = 0;
        m_sumlen    = 0;
        m_curlen    = 0;
        m_buf.clear();
    }

    unsigned int        m_msgid;            // Э��ID
    unsigned int        m_sumlen;           // �ܳ���
    unsigned int        m_curlen;           // ��ǰ����
    VectorBuffer        m_buf;              // ���� sumlen[uint32] + msgid[uint32] + body
};
typedef std::shared_ptr<SMsgPackNode> MsgSptr;


/*
*  ��Ϣ����
*/
class CMsgPackQueue
{
public:
    enum ENumDef
    { 
        MAXSMALLMSG_SIZE = 512,             // �ڴ�С��
        MAXBIGMSG_SIZE = 1024*1024          // �ڴ���
    };

    CMsgPackQueue(void);
    ~CMsgPackQueue(void);

public:
    bool PushMsg(unsigned int id, ::google::protobuf::MessageLite* pMsgBody);

    // �����������
    bool PushData(const char* pProtoBuf, unsigned int& nInOutMsgLength, bool& push_new);
    
    // �׽ڵ������
    MsgSptr PopFront();

    // ȡ�ÿɶ����׽ڵ�
    MsgSptr& ReadableFront();

    // �쳣
    void SetError();

    // �Ƿ��쳣
    bool IsError();

public:
    // new buf pool
    const char* AllocateBuf(unsigned int nLength);

    // del buf pool
    bool DelBuf(const char* pBuf, unsigned int nLength);

    // Э����г���
    size_t GetSize();


private:
    std::list<MsgSptr>                      m_lstMsg;           // ��Ϣ�б�
    MsgSptr                                 m_curmsg;           // ��ǰ��Ϣ
    std::mutex                              m_mutex;            // queue lock
    std::atomic_bool                        m_bError;           // �������
    size_t                                  m_nCurSize;         // ��ǰ��С
};

#endif
