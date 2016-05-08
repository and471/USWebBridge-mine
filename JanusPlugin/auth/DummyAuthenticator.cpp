#include "DummyAuthenticator.h"
#include "Authenticator.h"

bool DummyAuthenticator::isValid(json auth) {
    return true;
}

