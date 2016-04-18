#ifndef _H_MSGBUILD_H_
#define _H_MSGBUILD_H_

#include "MsgBase.h"
#include <memory>
#include <map>
#include <string>
#include "MsgIndex.pb.h"

/*
#define BULDMSG_BEGIN \
std::shared_ptr<::google::protobuf::MessageLite> MsgBuildTool::BuildRecvMsgPacket(MsgProto::MsgIndex sMsgType) \
{ \
switch (sMsgType) \
{

#define BUILDMSG_CASE_FUN(msg) \
case MsgProto::MSG_##msg: \
                                { \
    return std::move(std::shared_ptr<::google::protobuf::MessageLite>(new MsgProto::##msg())); \
        }

#define BUILDMSG_END \
default: \
    break; \
} \
    return std::move(std::shared_ptr<::google::protobuf::MessageLite>(nullptr));\
};
*/

#define BULDMSG_BEGIN \
void MsgBuildTool::InitTypeInfo() \
{

#define BUILDMSG_CASE_FUN(msg) \
                { \
        MsgProto::##msg msg; \
        std::string str = msg.GetTypeName(); \
        m_map_str_id[str] = MsgProto::MSG_##msg; \
        m_map_id_msg[MsgProto::MSG_##msg] = str; \
                }

#define BUILDMSG_END \
}

class MsgBuildTool
{
public:
    typedef std::shared_ptr<::google::protobuf::MessageLite> msglite_sptr;
    MsgBuildTool(const MsgBuildTool&) = delete;
    void operator = (const MsgBuildTool&) = delete;

    MsgBuildTool(){};

public:
    void InitTypeInfo();

    msglite_sptr NewMessage(MsgProto::MsgIndex sMsgType)
    {
        msglite_sptr msg;
        std::string& str = m_map_id_msg[sMsgType];
        const google::protobuf::Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(str);
        if (descriptor)
        {
            const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
            if (prototype)
            {
                msg = msglite_sptr(prototype->New());
            }
        }
        return msg;
    }

    MsgProto::MsgIndex GetMsgID(google::protobuf::MessageLite* msg)
    {
        if (msg != nullptr)
        {
            auto iter = m_map_str_id.find(msg->GetTypeName());
            if (iter != m_map_str_id.end())
            {
                return iter->second;
            }
        }
        return MsgProto::MSG_INDEX_INIT;
    }

private:
    std::map<std::string, MsgProto::MsgIndex> m_map_str_id;
    std::map<MsgProto::MsgIndex, std::string> m_map_id_msg;
    // todo : 结合MsgPacket的释放，添加回收机制
};


#endif //_H_MSGBUILD_H_