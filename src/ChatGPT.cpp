#include "ChatGPT.h"
#include "curl/curl.h"
#include "logger.h"

ChatGPT::ChatGPT(int64_t session_id,
                 int max_session_count,
                 const Authentication &authentication) : session_id(session_id),
                                                         max_session_count(max_session_count),
                                                         authentication(authentication)

{
}

ChatGPT::~ChatGPT()
{
}

std::string ChatGPT::message(const std::string &msg)
{
    Json::Value message(Json::ValueType::objectValue);
    message["role"] = "user";
    message["content"] = msg;
    this->push_back(message);
    std::string resp = this->post();
    Json::Value resp_json;
    Json::Reader reader;
    if (reader.parse(resp, resp_json))
    {
        if (resp_json["error"].isNull())
        {
            auto choices = resp_json["choices"];
            Json::FastWriter writer;
            std::string aaa = writer.write(choices);
            Json::Value result_message = choices[0]["message"];

            this->push_back(result_message);
            return result_message["content"].asString();
        }
        std::string error_str = resp_json["error"]["message"].asString();
        return error_str;
    }
    return std::string();
}

size_t callback(void *contents, size_t size, size_t nmemb, std::string *response)
{
    size_t total_size = size * nmemb;
    response->append((char *)contents, total_size);
    return total_size;
}

std::string ChatGPT::post()
{
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    std::string response;

    if (curl)
    {
        // https://api.openai.com/v1/chat/completions
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        Json::FastWriter writer;
        Json::Value params(Json::ValueType::objectValue);
        params["model"] = "gpt-3.5-turbo";
        params["messages"] = this->messages;
        std::string params_str = writer.write(params);
        LOG(INFO) << params_str.c_str();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params_str.c_str());

        std::vector<std::string> headers;
        headers.push_back("Content-Type: application/json");
        headers.push_back("Authorization: Bearer " + this->authentication.authorization);
        headers.push_back("OpenAI-Organization: " + this->authentication.organization);

        struct curl_slist *m_headers = nullptr;
        for (auto it = headers.begin(); it != headers.end(); it++)
        {
            m_headers = curl_slist_append(m_headers, (*it).c_str());
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        curl_easy_setopt(curl, CURLOPT_PROXY, "localhost");
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, 7890L);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cout << "Failed to perform request: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            std::cout << "Response: " << response << std::endl;
        }

        curl_slist_free_all(m_headers);
        curl_easy_cleanup(curl);
    }
    return response;
}

void ChatGPT::push_back(Json::Value &msg)
{
    int size = this->messages.size();
    if (size < this->max_session_count)
    {
        this->messages[size] = msg;
        return;
    }
    for (int i = 0; i < this->max_session_count; i++)
    {
        if (i == this->max_session_count - 1)
        {
            this->messages[i] = msg;
        }
        else
        {
            this->messages[i] = this->messages[i + 1];
        }
    }
}
