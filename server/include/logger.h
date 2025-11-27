#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
private:
    static Logger* instance;
    std::ofstream log_file;
    std::mutex log_mutex;
    LogLevel min_level;
    
    Logger();
    std::string get_timestamp();
    std::string level_to_string(LogLevel level);
    
public:
    static Logger* get_instance();
    
    void set_min_level(LogLevel level) { min_level = level; }
    void log(LogLevel level, const std::string& message);
    
    // Convenience methods
    void debug(const std::string& message) { log(DEBUG, message); }
    void info(const std::string& message) { log(INFO, message); }
    void warn(const std::string& message) { log(WARN, message); }
    void error(const std::string& message) { log(ERROR, message); }
    
    ~Logger();
};

// Global logger macros
#define LOG_DEBUG(msg) Logger::get_instance()->debug(msg)
#define LOG_INFO(msg) Logger::get_instance()->info(msg)
#define LOG_WARN(msg) Logger::get_instance()->warn(msg)
#define LOG_ERROR(msg) Logger::get_instance()->error(msg)

#endif // LOGGER_H

