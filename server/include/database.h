#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Forward declarations
struct User;
struct Question;
struct TestRoom;
struct Session;

// User structure
struct User {
    int user_id;
    std::string username;
    std::string hashed_password;
    std::string role; // "USER" or "TEACHER"
    std::string created_at;
};

// Question structure
struct Question {
    int question_id;
    std::string content;
    json options; // {"a": "...", "b": "...", ...}
    std::string correct_option;
    std::string difficulty;
    std::string topic;
    int created_by;
};

// Test Room structure
struct TestRoom {
    int room_id;
    std::string name;
    int creator_id;
    std::string status; // NOT_STARTED, ONGOING, FINISHED
    int num_questions;
    int duration_minutes;
    json filters_used;
    std::string start_timestamp;
    std::string end_timestamp;
};

// Session structure
struct Session {
    std::string session_token;
    int user_id;
    std::string expiry_timestamp;
};

class Database {
private:
    sqlite3* db;
    std::string db_path;
    
    // Helper: execute SQL with no return
    bool execute_sql(const std::string& sql);
    
    // Helper: get last insert rowid
    int64_t get_last_insert_rowid();

public:
    Database(const std::string& path);
    ~Database();
    
    // Initialize database (create tables)
    bool initialize();
    
    // Check if database is open
    bool is_open() const { return db != nullptr; }
    
    // User operations
    bool create_user(const std::string& username, const std::string& hashed_password, const std::string& role);
    bool get_user_by_username(const std::string& username, User& user);
    bool get_user_by_id(int user_id, User& user);
    
    // Session operations
    bool create_session(const std::string& token, int user_id, int expiry_seconds);
    bool get_session(const std::string& token, Session& session);
    bool delete_session(const std::string& token);
    bool is_session_valid(const std::string& token);
    int get_user_id_from_session(const std::string& token);
    void cleanup_expired_sessions();
    
    // Question operations
    std::vector<Question> get_questions_by_filter(const std::string& topic, const std::string& difficulty, int limit);
    bool get_question_by_id(int question_id, Question& question);
    std::vector<Question> get_random_questions(int count, const std::string& topic, const std::string& difficulty);
    
    // Practice history operations
    bool save_practice_result(int user_id, int correct_count, int total_questions, 
                             const std::string& filters_json, float score_percentage);
    
    // Test room operations
    bool create_test_room(const std::string& name, int creator_id, int num_questions, 
                         int duration_minutes, const std::string& filters_json, int& room_id);
    std::vector<TestRoom> get_all_rooms();
    bool get_room_by_id(int room_id, TestRoom& room);
    bool update_room_status(int room_id, const std::string& status);
    bool update_room_timestamps(int room_id, const std::string& start_time, const std::string& end_time);
    
    // Room participant operations
    bool add_participant(int room_id, int user_id);
    bool update_participant_status(int room_id, int user_id, const std::string& status);
    bool update_participant_score(int room_id, int user_id, int score);
    std::vector<std::string> get_room_participants(int room_id);
    bool is_user_in_room(int room_id, int user_id);
    
    // Room questions operations
    bool add_room_questions(int room_id, const std::vector<int>& question_ids);
    std::vector<Question> get_room_questions(int room_id);
    
    // User test answers operations
    bool save_user_answer(int user_id, int room_id, int question_id, const std::string& selected_option);
    bool update_answer_correctness(int user_id, int room_id, int question_id, bool is_correct);
    int get_user_score(int user_id, int room_id);
    
    // Statistics operations
    json get_user_practice_history(int user_id);
    json get_user_test_history(int user_id);
    json get_user_statistics(int user_id);
    json get_room_results(int room_id);
};

#endif // DATABASE_H

