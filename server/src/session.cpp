#include "../include/session.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <openssl/sha.h>

std::string SessionManager::generate_token(size_t length) {
    static const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);
    
    std::string token;
    token.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        token += chars[dis(gen)];
    }
    return token;
}

std::string SessionManager::hash_password(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
           password.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool SessionManager::verify_password(const std::string& password, const std::string& hash) {
    return hash_password(password) == hash;
}

std::string SessionManager::get_current_timestamp() {
    time_t now = time(nullptr);
    char buf[80];
    struct tm* tm_info = gmtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buf);
}

std::string SessionManager::get_future_timestamp(int seconds) {
    time_t future = time(nullptr) + seconds;
    char buf[80];
    struct tm* tm_info = gmtime(&future);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buf);
}

bool SessionManager::is_timestamp_expired(const std::string& timestamp) {
    // Parse timestamp string
    struct tm tm_time = {};
    strptime(timestamp.c_str(), "%Y-%m-%d %H:%M:%S", &tm_time);
    time_t expiry_time = timegm(&tm_time);
    
    // Compare with current time
    time_t now = time(nullptr);
    return now >= expiry_time;
}

