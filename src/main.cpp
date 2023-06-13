#include <iostream>
#include "Plugin.h"
#include "ChatGPT.h"
#include "logger.h"
#include "cqhttp_base.pb.h"
#include <vector>
#include "google/protobuf/util/json_util.h"
#include <filesystem>
#include "Authentication.h"

using namespace cqhttp;
using namespace std;
using namespace google::protobuf::util;
namespace fs = std::filesystem;

typedef std::map<int64_t, std::shared_ptr<ChatGPT>> ChatgptMap;
class ChatgptPlugin : public Plugin
{
private:
    string authorization;
    string organization;
    Authentication authentication;

public:
    using Plugin::Plugin;
    ChatgptMap chatgpt_map;
    void onPrivateMessage(std::shared_ptr<cqhttp::PrivateMessageEvent> event, std::shared_ptr<Bot> bot) override;

    void onGroupMessage(std::shared_ptr<cqhttp::GroupMessageEvent> event, std::shared_ptr<Bot> bot) override;

    void onCreated() override; // 插件被加载时调用，在这里初始化
};

void ChatgptPlugin::onPrivateMessage(std::shared_ptr<cqhttp::PrivateMessageEvent> event, std::shared_ptr<Bot> bot)
{
    if (event->user_id() != 1718692748)
    {
        return;
    }
    if (chatgpt_map.count(event->user_id()) < 1)
    {
        std::shared_ptr<ChatGPT> gpt_ptr(new ChatGPT(event->user_id(), 6, this->authentication));
        chatgpt_map.insert({event->user_id(), gpt_ptr});
    }

    // Make sender as forward message.
    vector<ForwardMessage> v_forward_msg;
    ForwardMessage forward_msg;
    forward_msg.set_type("node");
    ForwardMessage_Data *f_date = new ForwardMessage_Data;
    f_date->set_uin(to_string(event->user_id()));
    auto stranger_info = bot->getStrangerInfo(event->user_id());
    if (stranger_info)
        f_date->set_name(stranger_info->nickname());
    f_date->set_content(event->message());
    forward_msg.set_allocated_data(f_date);
    v_forward_msg.push_back(forward_msg);

    // Make chatgpt result as forward message.
    ForwardMessage send_forwward_msg;
    send_forwward_msg.set_type("node");
    ForwardMessage_Data *send_data = new ForwardMessage_Data;
    send_data->set_uin(to_string(bot->self_id));
    auto login_info = bot->getLoginInfo();
    send_data->set_name(login_info->nickname());

    auto &&chat = chatgpt_map.at(event->user_id());    // Get chatgpt instance obj.
    auto result_msg = chat->message(event->message()); // Send a message to chatgpt and wait for a response.
    send_data->set_content(result_msg);
    send_forwward_msg.set_allocated_data(send_data);
    v_forward_msg.push_back(send_forwward_msg);
    bot->sendPrivateForwardMsg(event->user_id(), v_forward_msg);
}

void ChatgptPlugin::onGroupMessage(std::shared_ptr<cqhttp::GroupMessageEvent> event, std::shared_ptr<Bot> bot)
{
    std::string at_obj = "[CQ:at,qq=2639125193]";
    auto message = event->message();
    auto is_at = message.find(at_obj);

    if (is_at != std::string::npos)
    {
        auto result_group_msg = message.replace(is_at, at_obj.length(), "");
        LOG(INFO) << "收到@消息: " << result_group_msg;
        if (chatgpt_map.count(event->user_id()) < 1)
        {
            std::shared_ptr<ChatGPT> gpt_ptr(new ChatGPT(event->user_id(), 6, this->authentication));
            chatgpt_map.insert({event->user_id(), gpt_ptr});
        }
        auto &&chat = chatgpt_map.at(event->user_id());
        auto result_msg = chat->message(result_group_msg);

        // Make received group message as forward message.
        vector<ForwardMessage> v_forward_msg;
        ForwardMessage make_receive_forward;
        make_receive_forward.set_type("node");
        ForwardMessage_Data *receive_data = new ForwardMessage_Data;
        receive_data->set_uin(to_string(event->user_id()));
        auto group_member_info = bot->getGroupMemberInfo(event->group_id(), event->user_id());
        if (group_member_info)
            receive_data->set_name(group_member_info->nickname());
        receive_data->set_content(result_group_msg);
        make_receive_forward.set_allocated_data(receive_data);
        v_forward_msg.push_back(make_receive_forward);

        // Make received chatgpt response as forward message.
        ForwardMessage sender_forward;
        sender_forward.set_type("node");
        ForwardMessage_Data *sender_data = new ForwardMessage_Data;
        sender_data->set_uin(to_string(bot->self_id));
        auto login_info = bot->getLoginInfo();
        if (login_info)
            sender_data->set_name(login_info->nickname());
        sender_data->set_content(result_msg);
        sender_forward.set_allocated_data(sender_data);
        v_forward_msg.push_back(sender_forward);

        bot->sendGroupForwardMsg(event->group_id(), v_forward_msg);
    }

    if (event->message().find("获取群信息") != std::string::npos)
    {
        auto group_info = bot->getGroupInfo(event->group_id());
        if (group_info)
        {
            ostringstream ostr;
            ostr << "group_name: " << group_info->group_name() << endl
                 << "group_id: " << group_info->group_id() << endl
                 << "group_level: " << group_info->group_level() << endl
                 << "group_memo: " << group_info->group_memo() << endl
                 << "group_create_time: " << group_info->group_create_time();

            bot->sendGroupMessage(ostr.str(), event->group_id());
        }
    }
}

void ChatgptPlugin::onCreated()
{
    // 检查 plugins/chatgpt/config.json 目录/文件 是否存在，不存在则创建
    string path_str("plugins/chatgpt/config.json");
    fs::path path(path_str);

    if (!fs::exists(path))
    {
        if (!fs::exists(path.parent_path()))
        {
            LOG(INFO) << "文件夹不存在，正在创建";
            if (fs::create_directories(path.parent_path()))
            {
                LOG(INFO) << "目录创建成功";
            }
            else
            {
                LOG(INFO) << "目录创建失败";
            }
        }

        LOG(INFO) << "配置文件不存在，正在创建配置文件";
        ofstream config_file(path_str);
        if (config_file)
        {
            LOG(INFO) << "配置文件创建成功，正在写入数据";
            Json::Value config_json(Json::ValueType::objectValue);
            config_json["authorization"] = "$OPENAI_API_KEY";
            config_json["organization"] = "$ORGANIZATION";
            Json::FastWriter writer;
            config_file << writer.write(config_json);
            config_file.close();
        }
    }

    // 读取文件中的数据，保存到内存中
    ifstream config_file(path_str, ios::in);
    if (config_file.is_open())
    {
        string line;
        string all_content;
        LOG(INFO) << "正在读取配置文件";
        while (getline(config_file, line))
        {
            all_content += line;
        }
        if (all_content.length() > 0)
        {
            Json::Value config_json(Json::ValueType::nullValue);
            Json::Reader reader;
            LOG(INFO) << "正在解析配置文件";
            if (reader.parse(all_content, config_json))
            {
                LOG(INFO) << "配置文件解析成功";
                this->authorization = config_json["authorization"].asString();
                this->organization = config_json["organization"].asString();
            }
            else
            {
                LOG(INFO) << "配置文件解析失败";
            }
        }
        else
        {
            LOG(ERR) << "错误的配置";
        }
    }

    // 创建 Authentication 对象
    if (this->authorization.length() > 0 && this->organization.length() > 0)
    {
        authentication = Authentication(this->authorization, this->organization);
    }

    LOG(INFO) << "配置文件初始化完成";
    LOG(INFO) << "Authentication: " << this->authentication.authorization;
    LOG(INFO) << "Organization: " << this->authentication.organization;
    LOG(INFO) << "Authentication is complete: " << (this->authentication.is_complete() ? "TRUE" : "FALSE");
}

extern "C" EXPORT_API Plugin *registerPlugin()
{
    return new ChatgptPlugin(
        "chatgpt",
        "qingshu",
        "A chatgpt api demo.",
        "zhenglantao@gmail.com");
}