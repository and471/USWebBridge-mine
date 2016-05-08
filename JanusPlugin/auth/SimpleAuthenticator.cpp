#include "SimpleAuthenticator.h"
#include "Authenticator.h"

bool SimpleAuthenticator::isValid(json auth) {
    std::string secret = "password";

    try {
        return secret == auth["secret"];
    } catch (std::domain_error e) {}
    return false;
}
