#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#endif

namespace contourtree {

class Logger
{
private:
    Logger() {}
    ~Logger() {}

    static Logger* logger;
#ifdef USE_SPDLOG
    shared_ptr<spdlog::logger> spdlogger;
#endif

public:
#ifdef USE_SPDLOG
    static void setLogger(shared_ptr<spdlog::logger> spdlogger);
#endif
    static void log(std::string str);
};

}

#endif // LOGGER_HPP
