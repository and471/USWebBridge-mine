#ifndef AUTHENTICATOR
#define AUTHENTICATOR

#include <json.hpp>
using json = nlohmann::json;

class Authenticator {

public:
    Authenticator() {};
    virtual bool isValid(json auth) =0;


};

#endif // AUTHENTICATOR

