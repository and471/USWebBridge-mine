#include "Logger.h"
#include <Modules/Sensors/TimeManager.h>

namespace Logger {


void log(std::ostream &ios, const char *s){

    ios << "[LOG] ["<< TimeManager::GetLocalTimestamp() <<"]\t"<< s << std::endl;

}

void log(std::ostream &ios, std::string &s){
    log(ios, s.data());
}

void debug(std::ostream &ios, const char *s){

    ios << "[DEBUG] ["<< TimeManager::GetLocalTimestamp() <<"]\t"<< s << std::endl;

}

void debug(std::ostream &ios, std::string &s){
    debug(ios, s.data());
}

void warn(std::ostream &ios, const char *s){

    ios << "[WARNING] ["<< TimeManager::GetLocalTimestamp() <<"]\t"<< s << std::endl;

}

void warn(std::ostream &ios, std::string &s){
    warn(ios, s.data());
}

void error(std::ostream &ios, const char *s){

    ios << "[ERROR] ["<< TimeManager::GetLocalTimestamp() <<"]\t"<< s << std::endl;

}

void error(std::ostream &ios, std::string &s){
    error(ios, s.data());
}


}
