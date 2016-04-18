#include "ProtoMsg.h"
#include <iostream>

// msgbody to protostring
bool CProtoMsgTool::TranslateMsgBodyToProto(::google::protobuf::MessageLite* pMsgHead, ::google::protobuf::MessageLite* pMsgBody, std::string& strBuf)
{
    unsigned int nLength = GetByteSize(pMsgHead, pMsgBody);
    if (nLength==0)
    {
        return false;
    }

    if (strBuf.size() < nLength)
    {
        strBuf.resize(nLength);
    }
    return TranslateMsgBodyToProto(pMsgHead, pMsgBody, (char*)strBuf.c_str(), nLength);
}


// size of char[]
int CProtoMsgTool::GetByteSize(::google::protobuf::MessageLite* pMsgHead, ::google::protobuf::MessageLite* pMsgBody)
{
    if (pMsgBody == NULL || pMsgHead==NULL || pMsgHead->GetTypeName().compare("MsgProto.MsgHead") != 0)
    {
        return 0;
    }

    // MsgProto::MsgHead* pHead = (MsgProto::MsgHead*)pMsgHead;
    return pMsgHead->ByteSize() + pMsgBody->ByteSize() + 2 * sizeof(unsigned short);
}

// msgbody to proto whith char[]
bool CProtoMsgTool::TranslateMsgBodyToProto(::google::protobuf::MessageLite* pMsgHead, ::google::protobuf::MessageLite* pMsgBody, char* pBuf, unsigned int& nInOutBufLength)
{
    if (pMsgBody == NULL || pMsgHead==NULL || pBuf == NULL|| nInOutBufLength==0 
        || pMsgHead->GetTypeName().compare("MsgProto.MsgHead") != 0)
    {
        return 0;
    }
    
    // build string
    unsigned short nHead = (unsigned short)pMsgHead->ByteSize();
    unsigned short nBody = (unsigned short)pMsgBody->ByteSize();
    unsigned int nSize = nHead + nBody +  2 * sizeof(unsigned short)/*prehead*/;
    if (nInOutBufLength < nSize)
    {
        return false;
    }
    nInOutBufLength = nSize;

    unsigned short* pSumL = (unsigned short*)pBuf;
    unsigned short* pHeadL = (unsigned short*)(pBuf + 2);
    char* pHead = pBuf + 4;
    char* pBody = pBuf + 4 + nHead;

    // size
    *pSumL = nHead + nBody;
    *pHeadL = nHead;

    // std::cout << "Head:" << *pHeadL << "Body:" << *pSumL - *pHeadL << std::endl;

    bool bSucc = pMsgHead->SerializePartialToArray(pHead, nHead);
    if (bSucc == false)
    {
        return false;
    }

    // body
    bool bBodySucc = pMsgBody->SerializePartialToArray(pBody, nBody);
    if (bBodySucc == false)
    {
        return false;
    }
    return true;
}

// protostring to headmsg
bool CProtoMsgTool::TanslateProtoToMsgHead(const char* pBuf, unsigned int uLength, ::google::protobuf::MessageLite* pMsgHead, unsigned int& uMsgBodyOffset)
{
    uMsgBodyOffset = 0;
    if (pBuf==NULL || uLength < 4 + 1 || pMsgHead==NULL)
    {
        return false;
    }

    // check length
    unsigned short* pSumLength = (unsigned short*)pBuf;
    unsigned short* pHeadLength = (unsigned short*)(pBuf + sizeof(unsigned short));
    
    unsigned short sSum = *pSumLength;
    unsigned short sHead = *pHeadLength;
    unsigned int uBufLength = sSum + 4;
    uMsgBodyOffset = sHead + 4;
    if (uBufLength > uLength)
    {
        return false;
    }

    // build msg head
    const char* pHead = pBuf + 2 * sizeof(unsigned short);    
    return pMsgHead->ParseFromArray(pHead, sHead);
}

// headmsg to msgbody
bool CProtoMsgTool::TanslateProtoToMsgBody(const char* pBodyBuf, unsigned int uLength, ::google::protobuf::MessageLite* pMsgBody)
{
    if (pBodyBuf==NULL || uLength== 0 || pMsgBody==NULL)
    {
        return false;
    }
    return pMsgBody->ParseFromArray(pBodyBuf, uLength);
}