#ifndef CHATGPT_CHATGPT_H
#define CHATGPT_CHATGPT_H

#include <iostream>
#include "json/json.h"

class ChatGPT
{
public:
    ChatGPT(int64_t session_id, int max_session_count);
    ~ChatGPT();

    std::string message(const std::string &msg); // 向 GPT 提问

private:
    std::string post();    // 向 ChatGPT 发起请求
    Json::Value messages;  // 记录会话的容器
    int64_t session_id;    // 会话持有者
    int max_session_count; // 会话最大记录
    void push_back(Json::Value &msg);
};

size_t callback(void *contents, size_t size,
                size_t nmemb, std::string *response); // 处理 POST 请求的回调函数
#endif                                                // CHATGPT_CHATGPT_H
