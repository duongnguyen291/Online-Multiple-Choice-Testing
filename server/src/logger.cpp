#include "../include/logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>

Logger* Logger::instance = nullptr;

Logger::Logger() : min_level(INFO) {
    log_file.open("server.log", std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
    }
}

Logger::~Logger() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

Logger* Logger::get_instance() {
    if (instance == nullptr) {
        instance = new Logger();
    }
    return instance;
}

std::string Logger::get_timestamp() {
    time_t now = time(nullptr);
    char buf[80];
    struct tm* tm_info = localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buf);
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO ";
        case WARN:  return "WARN ";
        case ERROR: return "ERROR";
        default:    return "UNKNW";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < min_level) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(log_mutex);
    
    std::string log_entry = "[" + get_timestamp() + "] [" + 
                           level_to_string(level) + "] " + message;
    
    // Write to file
    if (log_file.is_open()) {
        log_file << log_entry << std::endl;
        log_file.flush();
    }
    
    // Also write to console
    if (level >= WARN) {
        std::cerr << log_entry << std::endl;
    } else {
        std::cout << log_entry << std::endl;
    }
}

