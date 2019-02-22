#include "Logger.hpp"

#include <iostream>

namespace contourtree {

Logger* Logger::logger = NULL;

#ifdef CONTOUR_TREE_USE_SPDLOG
void Logger::setLogger(std::shared_ptr<spdlog::logger> spdlogger) {
    if(Logger::logger == NULL) {
        Logger::logger = new Logger();
    }
    Logger::logger->spdlogger = spdlogger;
}
#endif


void Logger::log(std::string str) {
#ifdef CONTOUR_TREE_USE_SPDLOG
    logger->spdlogger->debug(str);
#else
    std::cout << str << std::endl;
#endif
}

}
