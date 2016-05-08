#ifndef SIMPLEAUTHENTICATOR_H
#define SIMPLEAUTHENTICATOR_H

#include <json.hpp>
using json = nlohmann::json;

#include "Authenticator.h"

class SimpleAuthenticator : public Authenticator
{
public:
    SimpleAuthenticator() {}

    bool isValid(json auth);
};

#endif // SIMPLEAUTHENTICATOR_H
