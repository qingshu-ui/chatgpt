#include "Authentication.h"

Authentication::Authentication()
{
}

Authentication::Authentication(const std::string &authorization,
                               const std::string &organization) : authorization(authorization),
                                                                  organization(organization)
{
}

Authentication::~Authentication()
{
}

bool Authentication::is_complete()
{
    if (this->authorization.length() > 0 && this->organization.length() > 0)
    {
        if (!this->authorization._Equal("$OPENAI_API_KEY") && !this->organization._Equal("$ORGANIZATION"))
        {
            return true;
        }
    }
    return false;
}
