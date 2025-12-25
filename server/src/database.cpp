#include "../include/database.h"
#include "../include/logger.h"
#include "../include/session.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

Database::Database(const std::string& path) : db(nullptr), db_path(path) {
    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Cannot open database: " + std::string(sqlite3_errmsg(db)));
        db = nullptr;
    } else {
        LOG_INFO("Database opened successfully: " + path);
        // Enable foreign keys
        execute_sql("PRAGMA foreign_keys = ON;");
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
        LOG_INFO("Database closed");
    }
}

bool Database::execute_sql(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("SQL error: " + std::string(err_msg));
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

int64_t Database::get_last_insert_rowid() {
    return sqlite3_last_insert_rowid(db);
}

bool Database::initialize() {
    // Read schema from file (try multiple paths)
    std::ifstream schema_file;
    std::vector<std::string> schema_paths = {
        "database/schema.sql",           // From project root
        "../database/schema.sql",        // From server directory
        "../../database/schema.sql"      // From server/bin directory
    };
    
    bool schema_found = false;
    for (const auto& path : schema_paths) {
        schema_file.open(path);
        if (schema_file.is_open()) {
            schema_found = true;
            LOG_INFO("Found schema file: " + path);
            break;
        }
    }
    
    if (!schema_found) {
        LOG_ERROR("Cannot open schema file. Tried: " + schema_paths[0] + ", " + schema_paths[1] + ", " + schema_paths[2]);
        return false;
    }
    
    std::stringstream buffer;
    buffer << schema_file.rdbuf();
    std::string schema = buffer.str();
    schema_file.close();
    
    // Execute schema
    if (!execute_sql(schema)) {
        return false;
    }
    
    LOG_INFO("Database schema initialized");
    
    // Insert sample data if empty
    sqlite3_stmt* stmt;
    const char* check_sql = "SELECT COUNT(*) FROM Users;";
    if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            if (count == 0) {
                LOG_INFO("Inserting sample data...");
                
                // Try multiple paths for init data
                std::vector<std::string> init_paths = {
                    "database/init_data.sql",
                    "../database/init_data.sql",
                    "../../database/init_data.sql"
                };
                
                bool init_found = false;
                for (const auto& path : init_paths) {
                    std::ifstream init_file(path);
                    if (init_file.is_open()) {
                        init_found = true;
                        LOG_INFO("Found init data file: " + path);
                        std::stringstream init_buffer;
                        init_buffer << init_file.rdbuf();
                        execute_sql(init_buffer.str());
                        init_file.close();
                        break;
                    }
                }
                
                if (!init_found) {
                    LOG_WARN("Sample data file not found, continuing without sample data");
                }
            }
        }
        sqlite3_finalize(stmt);
    }
    
    return true;
}

// User operations
bool Database::create_user(const std::string& username, const std::string& hashed_password, 
                          const std::string& role) {
    const char* sql = "INSERT INTO Users (username, hashed_password, role) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_TRANSIENT);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    if (success) {
        LOG_INFO("User created: " + username);
    } else {
        LOG_ERROR("Failed to create user: " + std::string(sqlite3_errmsg(db)));
    }
    
    return success;
}

bool Database::get_user_by_username(const std::string& username, User& user) {
    const char* sql = "SELECT user_id, username, hashed_password, role, created_at FROM Users WHERE username = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.user_id = sqlite3_column_int(stmt, 0);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.hashed_password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

bool Database::get_user_by_id(int user_id, User& user) {
    const char* sql = "SELECT user_id, username, hashed_password, role, created_at FROM Users WHERE user_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.user_id = sqlite3_column_int(stmt, 0);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.hashed_password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

// Session operations
bool Database::create_session(const std::string& token, int user_id, int expiry_seconds) {
    const char* sql = "INSERT INTO Sessions (session_token, user_id, expiry_timestamp) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    std::string expiry = SessionManager::get_future_timestamp(expiry_seconds);
    
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, user_id);
    sqlite3_bind_text(stmt, 3, expiry.c_str(), -1, SQLITE_TRANSIENT);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool Database::get_session(const std::string& token, Session& session) {
    const char* sql = "SELECT session_token, user_id, expiry_timestamp FROM Sessions WHERE session_token = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        session.session_token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        session.user_id = sqlite3_column_int(stmt, 1);
        session.expiry_timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

bool Database::delete_session(const std::string& token) {
    const char* sql = "DELETE FROM Sessions WHERE session_token = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool Database::is_session_valid(const std::string& token) {
    Session session;
    if (!get_session(token, session)) {
        return false;
    }
    return !SessionManager::is_timestamp_expired(session.expiry_timestamp);
}

int Database::get_user_id_from_session(const std::string& token) {
    Session session;
    if (!get_session(token, session)) {
        return -1;
    }
    if (SessionManager::is_timestamp_expired(session.expiry_timestamp)) {
        return -1;
    }
    return session.user_id;
}

void Database::cleanup_expired_sessions() {
    std::string current_time = SessionManager::get_current_timestamp();
    const char* sql = "DELETE FROM Sessions WHERE expiry_timestamp < ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, current_time.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

// Question operations
std::vector<Question> Database::get_random_questions(int count, const std::string& topic, 
                                                     const std::string& difficulty) {
    std::vector<Question> questions;
    std::stringstream sql;
    sql << "SELECT question_id, content, options, correct_option, difficulty, topic, created_by "
        << "FROM Questions WHERE 1=1";
    
    if (topic != "all" && !topic.empty()) {
        sql << " AND topic = '" << topic << "'";
    }
    if (difficulty != "all" && !difficulty.empty()) {
        sql << " AND difficulty = '" << difficulty << "'";
    }
    sql << " ORDER BY RANDOM() LIMIT " << count << ";";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        return questions;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Question q;
        q.question_id = sqlite3_column_int(stmt, 0);
        q.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string options_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        q.options = json::parse(options_str);
        q.correct_option = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        q.difficulty = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        q.topic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        q.created_by = sqlite3_column_int(stmt, 6);
        questions.push_back(q);
    }
    
    sqlite3_finalize(stmt);
    return questions;
}

bool Database::get_question_by_id(int question_id, Question& question) {
    const char* sql = "SELECT question_id, content, options, correct_option, difficulty, topic, created_by "
                     "FROM Questions WHERE question_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, question_id);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        question.question_id = sqlite3_column_int(stmt, 0);
        question.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string options_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        question.options = json::parse(options_str);
        question.correct_option = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        question.difficulty = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        question.topic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        question.created_by = sqlite3_column_int(stmt, 6);
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

bool Database::create_question(const std::string& content, const json& options, const std::string& correct_option,
                              const std::string& difficulty, const std::string& topic, int created_by, int& question_id) {
    const char* sql = "INSERT INTO Questions (content, options, correct_option, difficulty, topic, created_by) "
                     "VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("Failed to prepare create_question: " + std::string(sqlite3_errmsg(db)));
        return false;
    }
    
    std::string options_str = options.dump();
    
    sqlite3_bind_text(stmt, 1, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, options_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, correct_option.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, difficulty.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, topic.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, created_by);
    
    int result = sqlite3_step(stmt);
    bool success = result == SQLITE_DONE;
    
    if (success) {
        question_id = static_cast<int>(get_last_insert_rowid());
    } else {
        LOG_ERROR("create_question execution failed: " + std::string(sqlite3_errmsg(db)));
    }
    
    sqlite3_finalize(stmt);
    return success;
}

bool Database::update_question(int question_id, const std::string& content, const json& options, 
                              const std::string& correct_option, const std::string& difficulty, const std::string& topic) {
    const char* sql = "UPDATE Questions SET content=?, options=?, correct_option=?, difficulty=?, topic=? "
                     "WHERE question_id=?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    std::string options_str = options.dump();
    
    sqlite3_bind_text(stmt, 1, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, options_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, correct_option.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, difficulty.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, topic.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, question_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::delete_question(int question_id) {
    const char* sql = "DELETE FROM Questions WHERE question_id=?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, question_id);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

std::vector<Question> Database::get_questions_by_creator(int creator_id) {
    std::vector<Question> questions;
    const char* sql = "SELECT question_id, content, options, correct_option, difficulty, topic, created_by "
                     "FROM Questions WHERE created_by = ? ORDER BY question_id DESC;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return questions;
    }
    
    sqlite3_bind_int(stmt, 1, creator_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Question q;
        q.question_id = sqlite3_column_int(stmt, 0);
        q.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        q.options = json::parse(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        q.correct_option = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        q.difficulty = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        q.topic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        q.created_by = sqlite3_column_int(stmt, 6);
        questions.push_back(q);
    }
    
    sqlite3_finalize(stmt);
    return questions;
}

std::vector<Question> Database::get_all_questions() {
    std::vector<Question> questions;
    const char* sql = "SELECT question_id, content, options, correct_option, difficulty, topic, created_by "
                     "FROM Questions ORDER BY question_id DESC;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return questions;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Question q;
        q.question_id = sqlite3_column_int(stmt, 0);
        q.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        q.options = json::parse(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        q.correct_option = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        q.difficulty = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        q.topic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        q.created_by = sqlite3_column_int(stmt, 6);
        questions.push_back(q);
    }
    
    sqlite3_finalize(stmt);
    return questions;
}

// Practice history operations
bool Database::save_practice_result(int user_id, int correct_count, int total_questions, 
                                   const std::string& filters_json, float score_percentage) {
    const char* sql = "INSERT INTO PracticeHistory (user_id, correct_count, total_questions, filters_used, score_percentage) "
                     "VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, correct_count);
    sqlite3_bind_int(stmt, 3, total_questions);
    sqlite3_bind_text(stmt, 4, filters_json.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, score_percentage);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

// Test room operations
bool Database::create_test_room(const std::string& name, int creator_id, int num_questions, 
                               int duration_minutes, const std::string& filters_json, int& room_id) {
    const char* sql = "INSERT INTO TestRooms (name, creator_id, status, num_questions, duration_minutes, filters_used) "
                     "VALUES (?, ?, 'NOT_STARTED', ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, creator_id);
    sqlite3_bind_int(stmt, 3, num_questions);
    sqlite3_bind_int(stmt, 4, duration_minutes);
    sqlite3_bind_text(stmt, 5, filters_json.c_str(), -1, SQLITE_TRANSIENT);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (success) {
        room_id = static_cast<int>(get_last_insert_rowid());
    }
    sqlite3_finalize(stmt);
    
    return success;
}

std::vector<TestRoom> Database::get_all_rooms() {
    std::vector<TestRoom> rooms;
    const char* sql = "SELECT room_id, name, creator_id, status, num_questions, duration_minutes, "
                     "filters_used, start_timestamp, end_timestamp FROM TestRooms ORDER BY created_at DESC;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return rooms;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TestRoom room;
        room.room_id = sqlite3_column_int(stmt, 0);
        room.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        room.creator_id = sqlite3_column_int(stmt, 2);
        room.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        room.num_questions = sqlite3_column_int(stmt, 4);
        room.duration_minutes = sqlite3_column_int(stmt, 5);
        
        const char* filters_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (filters_text) {
            room.filters_used = json::parse(filters_text);
        }
        
        const char* start_ts = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        room.start_timestamp = start_ts ? start_ts : "";
        
        const char* end_ts = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        room.end_timestamp = end_ts ? end_ts : "";
        
        rooms.push_back(room);
    }
    
    sqlite3_finalize(stmt);
    return rooms;
}

bool Database::get_room_by_id(int room_id, TestRoom& room) {
    const char* sql = "SELECT room_id, name, creator_id, status, num_questions, duration_minutes, "
                     "filters_used, start_timestamp, end_timestamp FROM TestRooms WHERE room_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, room_id);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        room.room_id = sqlite3_column_int(stmt, 0);
        room.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        room.creator_id = sqlite3_column_int(stmt, 2);
        room.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        room.num_questions = sqlite3_column_int(stmt, 4);
        room.duration_minutes = sqlite3_column_int(stmt, 5);
        
        const char* filters_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (filters_text) {
            room.filters_used = json::parse(filters_text);
        }
        
        const char* start_ts = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        room.start_timestamp = start_ts ? start_ts : "";
        
        const char* end_ts = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        room.end_timestamp = end_ts ? end_ts : "";
        
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

bool Database::update_room_status(int room_id, const std::string& status) {
    const char* sql = "UPDATE TestRooms SET status = ? WHERE room_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, room_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool Database::update_room_timestamps(int room_id, const std::string& start_time, const std::string& end_time) {
    const char* sql = "UPDATE TestRooms SET start_timestamp = ?, end_timestamp = ? WHERE room_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, start_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, end_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, room_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

// Room participant operations
bool Database::add_participant(int room_id, int user_id) {
    const char* sql = "INSERT INTO RoomParticipants (room_id, user_id, status) VALUES (?, ?, 'JOINED');";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, room_id);
    sqlite3_bind_int(stmt, 2, user_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

std::vector<std::string> Database::get_room_participants(int room_id) {
    std::vector<std::string> participants;
    const char* sql = "SELECT u.username FROM RoomParticipants rp "
                     "JOIN Users u ON rp.user_id = u.user_id "
                     "WHERE rp.room_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return participants;
    }
    
    sqlite3_bind_int(stmt, 1, room_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        participants.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }
    
    sqlite3_finalize(stmt);
    return participants;
}

bool Database::is_user_in_room(int room_id, int user_id) {
    const char* sql = "SELECT COUNT(*) FROM RoomParticipants WHERE room_id = ? AND user_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, room_id);
    sqlite3_bind_int(stmt, 2, user_id);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        found = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

// Room questions operations
bool Database::add_room_questions(int room_id, const std::vector<int>& question_ids) {
    const char* sql = "INSERT INTO TestRoomQuestions (room_id, question_id, question_order) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    for (size_t i = 0; i < question_ids.size(); ++i) {
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, room_id);
        sqlite3_bind_int(stmt, 2, question_ids[i]);
        sqlite3_bind_int(stmt, 3, static_cast<int>(i + 1));
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }
    
    sqlite3_finalize(stmt);
    return true;
}

std::vector<Question> Database::get_room_questions(int room_id) {
    std::vector<Question> questions;
    const char* sql = "SELECT q.question_id, q.content, q.options, q.correct_option, q.difficulty, q.topic, q.created_by "
                     "FROM TestRoomQuestions trq "
                     "JOIN Questions q ON trq.question_id = q.question_id "
                     "WHERE trq.room_id = ? ORDER BY trq.question_order;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return questions;
    }
    
    sqlite3_bind_int(stmt, 1, room_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Question q;
        q.question_id = sqlite3_column_int(stmt, 0);
        q.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string options_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        q.options = json::parse(options_str);
        q.correct_option = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        q.difficulty = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        q.topic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        q.created_by = sqlite3_column_int(stmt, 6);
        questions.push_back(q);
    }
    
    sqlite3_finalize(stmt);
    return questions;
}

// User test answers operations
bool Database::save_user_answer(int user_id, int room_id, int question_id, const std::string& selected_option) {
    const char* sql = "INSERT OR REPLACE INTO UserTestAnswers (user_id, room_id, question_id, selected_option, last_updated) "
                     "VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, room_id);
    sqlite3_bind_int(stmt, 3, question_id);
    sqlite3_bind_text(stmt, 4, selected_option.c_str(), -1, SQLITE_TRANSIENT);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool Database::update_answer_correctness(int user_id, int room_id, int question_id, bool is_correct) {
    const char* sql = "UPDATE UserTestAnswers SET is_correct = ? WHERE user_id = ? AND room_id = ? AND question_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, is_correct ? 1 : 0);
    sqlite3_bind_int(stmt, 2, user_id);
    sqlite3_bind_int(stmt, 3, room_id);
    sqlite3_bind_int(stmt, 4, question_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

int Database::get_user_score(int user_id, int room_id) {
    const char* sql = "SELECT COUNT(*) FROM UserTestAnswers WHERE user_id = ? AND room_id = ? AND is_correct = 1;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, room_id);
    
    int score = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        score = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return score;
}

bool Database::update_participant_score(int room_id, int user_id, int score) {
    const char* sql = "UPDATE RoomParticipants SET score = ? WHERE room_id = ? AND user_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, score);
    sqlite3_bind_int(stmt, 2, room_id);
    sqlite3_bind_int(stmt, 3, user_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool Database::update_participant_status(int room_id, int user_id, const std::string& status) {
    const char* sql = "UPDATE RoomParticipants SET status = ? WHERE room_id = ? AND user_id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, room_id);
    sqlite3_bind_int(stmt, 3, user_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

// Statistics operations (placeholder implementations)
json Database::get_user_practice_history(int user_id) {
    json history = json::array();
    const char* sql = "SELECT practice_id, correct_count, total_questions, score_percentage, completed_at "
                     "FROM PracticeHistory WHERE user_id = ? ORDER BY completed_at DESC LIMIT 20;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return history;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        json item;
        item["mode"] = "PRACTICE";
        item["score"] = std::to_string(sqlite3_column_int(stmt, 1)) + "/" + 
                       std::to_string(sqlite3_column_int(stmt, 2));
        item["date"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        history.push_back(item);
    }
    
    sqlite3_finalize(stmt);
    return history;
}

json Database::get_user_test_history(int user_id) {
    json history = json::array();
    const char* sql = "SELECT tr.name, rp.score, tr.num_questions, rp.joined_at "
                     "FROM RoomParticipants rp "
                     "JOIN TestRooms tr ON rp.room_id = tr.room_id "
                     "WHERE rp.user_id = ? AND tr.status = 'FINISHED' "
                     "ORDER BY rp.joined_at DESC LIMIT 20;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return history;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        json item;
        item["mode"] = "TEST";
        item["room_name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        item["score"] = std::to_string(sqlite3_column_int(stmt, 1)) + "/" + 
                       std::to_string(sqlite3_column_int(stmt, 2));
        item["date"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        history.push_back(item);
    }
    
    sqlite3_finalize(stmt);
    return history;
}

json Database::get_user_statistics(int user_id) {
    (void)user_id; // TODO: Implement statistics aggregation
    json stats;
    stats["score_over_time"] = json::array();
    stats["topic_distribution"] = json::array();
    return stats;
}

json Database::get_room_results(int room_id) {
    json results = json::array();
    const char* sql = "SELECT u.username, rp.score, tr.num_questions "
                     "FROM RoomParticipants rp "
                     "JOIN Users u ON rp.user_id = u.user_id "
                     "JOIN TestRooms tr ON rp.room_id = tr.room_id "
                     "WHERE rp.room_id = ? ORDER BY rp.score DESC;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return results;
    }
    
    sqlite3_bind_int(stmt, 1, room_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        json item;
        item["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        item["score"] = std::to_string(sqlite3_column_int(stmt, 1)) + "/" + 
                       std::to_string(sqlite3_column_int(stmt, 2));
        results.push_back(item);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

std::vector<Question> Database::get_questions_by_filter(const std::string& topic, 
                                                        const std::string& difficulty, int limit) {
    return get_random_questions(limit, topic, difficulty);
}

