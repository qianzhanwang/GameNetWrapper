#ifndef __PROTOMSG_H__
#define __PROTOMSG_H__
#pragma warning (disable : 4512)
#pragma warning (disable : 4996)
#include "../MsgDef/MsgDefine.pb.h"

// protostring和msg转换工具
//
class CProtoMsgTool
{
private:
    CProtoMsgTool(void){};
    ~CProtoMsgTool(void){};

public:
	// msgbody to proto with string
	static bool TranslateMsgBodyToProto(::google::protobuf::MessageLite* pMsgHead, ::google::protobuf::MessageLite* pMsgBody, std::string& strBuf);

    // size of char[]
    static int GetByteSize(::google::protobuf::MessageLite* pMsgHead, ::google::protobuf::MessageLite* pMsgBody);

    // msgbody to proto whith char[], when you call API, call GetByteSize() first
    static bool TranslateMsgBodyToProto(::google::protobuf::MessageLite* pMsgHead, ::google::protobuf::MessageLite* pMsgBody, char* pBuf, unsigned int& nInOutBufLength);

	// protostring to headmsg
	static bool TanslateProtoToMsgHead(const char* pBeginBuf, unsigned int uLength, ::google::protobuf::MessageLite* pMsgHead, unsigned int& uMsgBodyOffset);

	// protostring to msgbody
	static bool TanslateProtoToMsgBody(const char* pBodyBuf, unsigned int uLength, ::google::protobuf::MessageLite* pMsgBody);
};

#endif