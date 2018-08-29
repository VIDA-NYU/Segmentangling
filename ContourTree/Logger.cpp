#include "Logger.hpp"

#include <iostream>

namespace contourtree {

Logger* Logger::logger = NULL;

#ifdef USE_SPDLOG
void Logger::setLogger(shared_ptr<spdlog::logger> spdlogger) {
    if(this->logger == NULL) {
        this->logger = new Logger();
    }
    logger->spspdlogger = spdlogger;
}
#endif


void Logger::log(std::string str) {
#ifdef USE_SPDLOG
    logger->spspdlogger->debug(str);
#else
    std::cout << str << std::endl;
#endif
}

}
