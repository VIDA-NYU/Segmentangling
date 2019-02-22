#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <memory>

#ifdef CONTOUR_TREE_USE_SPDLOG
#include <spdlog/spdlog.h>
#endif

namespace contourtree {

class Logger
{
private:
    Logger() {}
    ~Logger() {}

    static Logger* logger;
#ifdef CONTOUR_TREE_USE_SPDLOG
    std::shared_ptr<spdlog::logger> spdlogger;
#endif

public:
#ifdef CONTOUR_TREE_USE_SPDLOG
    static void setLogger(std::shared_ptr<spdlog::logger> spdlogger);
#endif
    static void log(std::string str);
};

}

#endif // LOGGER_HPP
