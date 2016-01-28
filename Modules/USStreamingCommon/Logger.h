#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <iomanip>
#include <string>

namespace Logger {


void log(std::ostream &ios, const char * str);
void log(std::ostream &ios, std::string &);
void debug(std::ostream &ios, const char * str);
void debug(std::ostream &ios, std::string &);
void warn(std::ostream &ios, const char * str);
void warn(std::ostream &ios, std::string &);
void error(std::ostream &ios, const char * str);
void error(std::ostream &ios, std::string &);

}

#endif // LOGGER_H_
