#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <random>

class SessionManager {
public:
    // Generate random session token
    static std::string generate_token(size_t length = 32);
    
    // Hash password (simple SHA256 for demo, should use bcrypt in production)
    static std::string hash_password(const std::string& password);
    
    // Verify password against hash
    static bool verify_password(const std::string& password, const std::string& hash);
    
    // Get current timestamp in ISO format
    static std::string get_current_timestamp();
    
    // Get timestamp N seconds in the future
    static std::string get_future_timestamp(int seconds);
    
    // Check if timestamp is expired
    static bool is_timestamp_expired(const std::string& timestamp);
};

#endif // SESSION_H

