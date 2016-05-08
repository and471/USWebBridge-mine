#ifndef DUMMYAUTHENTICATOR_H
#define DUMMYAUTHENTICATOR_H

#include <json.hpp>
using json = nlohmann::json;

#include "Authenticator.h"

class DummyAuthenticator : public Authenticator
{
public:
    DummyAuthenticator() {}

    bool isValid(json auth);
};

#endif // DUMMYAUTHENTICATOR_H
