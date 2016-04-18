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
* 队列节点
*/
struct SMsgPackNode /*: public TObject_pool<SMsgPackNode, 500>, 使用tcmalloc/jemalloc替代*/
{
    SMsgPackNode()
    {
        m_msgid     = 0;
        m_sumlen    = 0;
        m_curlen    = 0;
        m_buf.clear();
    }

    unsigned int        m_msgid;            // 协议ID
    unsigned int        m_sumlen;           // 总长度
    unsigned int        m_curlen;           // 当前长度
    VectorBuffer        m_buf;              // 数据 sumlen[uint32] + msgid[uint32] + body
};
typedef std::shared_ptr<SMsgPackNode> MsgSptr;


/*
*  消息队列
*/
class CMsgPackQueue
{
public:
    enum ENumDef
    { 
        MAXSMALLMSG_SIZE = 512,             // 内存小块
        MAXBIGMSG_SIZE = 1024*1024          // 内存大块
    };

    CMsgPackQueue(void);
    ~CMsgPackQueue(void);

public:
    bool PushMsg(unsigned int id, ::google::protobuf::MessageLite* pMsgBody);

    // 二进制入队列
    bool PushData(const char* pProtoBuf, unsigned int& nInOutMsgLength, bool& push_new);
    
    // 首节点出队列
    MsgSptr PopFront();

    // 取得可读的首节点
    MsgSptr& ReadableFront();

    // 异常
    void SetError();

    // 是否异常
    bool IsError();

public:
    // new buf pool
    const char* AllocateBuf(unsigned int nLength);

    // del buf pool
    bool DelBuf(const char* pBuf, unsigned int nLength);

    // 协议队列长度
    size_t GetSize();


private:
    std::list<MsgSptr>                      m_lstMsg;           // 消息列表
    MsgSptr                                 m_curmsg;           // 当前消息
    std::mutex                              m_mutex;            // queue lock
    std::atomic_bool                        m_bError;           // 解码出错
    size_t                                  m_nCurSize;         // 当前大小
};

#endif
