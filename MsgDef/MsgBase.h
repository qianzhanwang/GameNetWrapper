#ifndef _H_MSG_BASE_H_
#define _H_MSG_BASE_H_

#pragma warning (disable : 4512)
#pragma warning (disable : 4996)
#include "../MsgDef/MsgDefine.pb.h"
#include <memory>
// #pragma pack(push, 1) 

// 接受结构体
struct MsgPacket
{
    MsgPacket()
    {
        sessionid = 0;
        msgid = 0;
        roleid = 0;
        msg_sptr.reset();
    }

	MsgPacket(MsgPacket&& src)
	{
		sessionid = src.sessionid;
		msgid = src.msgid;
		roleid = src.roleid;
		msg_sptr = std::move(src.msg_sptr);
	}

    unsigned int sessionid;
    unsigned int roleid;
    unsigned int msgid;
    std::shared_ptr<::google::protobuf::MessageLite> msg_sptr;
};


// #pragma pack(pop) 
#endif //_H_MSG_BASE_H_