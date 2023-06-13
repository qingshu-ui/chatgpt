#ifndef CHATGPT_AUTHENTICATION_H
#define CHATGPT_AUTHENTICATION_H

#include <iostream>

class Authentication
{
public:
    Authentication();
    Authentication(const std::string &authorization, const std::string &organization);
    ~Authentication();
    std::string authorization;
    std::string organization;

    bool is_complete();
};

#endif // CHATGPT_AUTHENTICATION_H