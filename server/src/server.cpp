#include "../include/server.h"
#include "../include/logger.h"
#include "../include/session.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>

Server::Server(int port, Database* database) : server_fd(-1), epoll_fd(-1), port(port), db(database) {
}

Server::~Server() {
    stop();
}

bool Server::setup_server_socket() {
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        LOG_ERROR("Failed to create socket");
        return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_ERROR("Failed to set SO_REUSEADDR");
        return false;
    }
    
    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        LOG_ERROR("Failed to bind socket to port " + std::to_string(port));
        return false;
    }
    
    // Listen
    if (listen(server_fd, 10) < 0) {
        LOG_ERROR("Failed to listen on socket");
        return false;
    }
    
    // Set non-blocking
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    
    LOG_INFO("Server listening on port " + std::to_string(port));
    return true;
}

bool Server::setup_epoll() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        LOG_ERROR("Failed to create epoll");
        return false;
    }
    
    // Add server socket to epoll
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        LOG_ERROR("Failed to add server socket to epoll");
        return false;
    }
    
    LOG_INFO("Epoll initialized");
    return true;
}

void Server::handle_new_connection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("Failed to accept connection");
        }
        return;
    }
    
    // Set non-blocking
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    
    // Add to epoll
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; // Edge-triggered
    event.data.fd = client_fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
        LOG_ERROR("Failed to add client to epoll");
        close(client_fd);
        return;
    }
    
    // Create client info
    ClientInfo client;
    client.sockfd = client_fd;
    client.user_id = -1;
    clients[client_fd] = client;
    
    LOG_INFO("New connection accepted: fd=" + std::to_string(client_fd));
}

void Server::handle_client_disconnect(int client_fd) {
    LOG_INFO("Client disconnected: fd=" + std::to_string(client_fd));
    
    // Remove from room_clients map
    for (auto& pair : room_clients) {
        pair.second.erase(client_fd);
    }
    
    // Remove from clients map
    clients.erase(client_fd);
    
    // Remove from epoll
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
    
    // Close socket
    close(client_fd);
}

void Server::handle_client_message(int client_fd) {
    // Luồng xử lý khi nhận (Receive Logic) - theo application_design.md:
    // 1. Nhận (recv) ít nhất 6 byte vào bộ đệm (buffer)
    // 2. Đọc 2 byte đầu để lấy msg_type (nhớ dùng ntohs())
    // 3. Đọc 4 byte tiếp theo để lấy payload_length (nhớ dùng ntohl())
    // 4. Nếu payload_length > 0, tiếp tục nhận (recv) cho đến khi đủ payload_length byte
    // 5. Sau khi có đủ payload_length byte, trích xuất chuỗi JSON từ bộ đệm
    // 6. Phân tích (parse) chuỗi JSON và xử lý logic
    // 7. Lặp lại từ bước 1 (vì bộ đệm có thể còn dữ liệu của gói tin tiếp theo)
    
    int messages_processed = 0;
    const int MAX_MESSAGES_PER_LOOP = 1000; // Giới hạn để tránh infinite loop (tăng lên vì có thể có nhiều messages)
    
    // Bước 7: Lặp lại từ bước 1 - xử lý tất cả messages có sẵn trong buffer
    while (messages_processed < MAX_MESSAGES_PER_LOOP) {
        Message msg;
        RecvResult result = Protocol::recv_message(client_fd, msg);
        
        if (result == RECV_NO_DATA) {
            // Không còn data trong buffer (EAGAIN) - đây là trường hợp bình thường
            // Dừng loop và chờ event tiếp theo từ epoll
            break;
        } else if (result == RECV_ERROR) {
            // Lỗi hoặc connection closed - disconnect client
            handle_client_disconnect(client_fd);
            return;
        } else if (result == RECV_INCOMPLETE) {
            // Partial message - không nên xảy ra với TCP, nhưng log và disconnect để an toàn
            LOG_ERROR("Incomplete message received, disconnecting client");
            handle_client_disconnect(client_fd);
            return;
        } else if (result == RECV_SUCCESS) {
            // Bước 6: Phân tích (parse) chuỗi JSON và xử lý logic
            messages_processed++;
            LOG_INFO("Received message type " + std::to_string(msg.type) + " from fd=" + std::to_string(client_fd));
            
            // Route message based on type
            switch (msg.type) {
                case C2S_REGISTER:
                    handle_register(client_fd, msg.payload);
                    break;
                case C2S_LOGIN:
                    handle_login(client_fd, msg.payload);
                    break;
                case C2S_LOGOUT:
                    handle_logout(client_fd, msg.payload);
                    break;
                case C2S_PRACTICE_REQUEST:
                    handle_practice_request(client_fd, msg.payload);
                    break;
                case C2S_PRACTICE_SUBMIT:
                    handle_practice_submit(client_fd, msg.payload);
                    break;
                case C2S_LIST_ROOMS:
                    handle_list_rooms(client_fd, msg.payload);
                    break;
                case C2S_CREATE_ROOM:
                    handle_create_room(client_fd, msg.payload);
                    break;
                case C2S_JOIN_ROOM:
                    handle_join_room(client_fd, msg.payload);
                    break;
                case C2S_START_TEST:
                    handle_start_test(client_fd, msg.payload);
                    break;
                case C2S_CHANGE_ANSWER:
                    handle_change_answer(client_fd, msg.payload);
                    break;
                case C2S_SUBMIT_TEST:
                    handle_submit_test(client_fd, msg.payload);
                    break;
                case C2S_GET_HISTORY:
                    handle_get_history(client_fd, msg.payload);
                    break;
                case C2S_GET_STATS:
                    handle_get_stats(client_fd, msg.payload);
                    break;
                case C2S_VIEW_ROOM_RESULTS:
                    handle_view_room_results(client_fd, msg.payload);
                    break;
                default:
                    LOG_WARN("Unknown message type: " + std::to_string(msg.type));
                    json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "Unknown message type");
                    Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
                    break;
            }
            
            // Sau khi xử lý xong, tiếp tục loop để xử lý message tiếp theo (nếu có)
            // Đây chính là "Lặp lại từ bước 1" trong quy định
        }
    }
    
    if (messages_processed > 1) {
        LOG_DEBUG("Processed " + std::to_string(messages_processed) + " messages in one loop (following 'Lặp lại từ bước 1' rule)");
    }
    
    if (messages_processed >= MAX_MESSAGES_PER_LOOP) {
        LOG_WARN("Reached MAX_MESSAGES_PER_LOOP limit, stopping to prevent infinite loop");
    }
}

bool Server::validate_session(int client_fd, const std::string& session_token, int& user_id, std::string& role) {
    if (!db->is_session_valid(session_token)) {
        json error = Protocol::create_error_response(ERR_INVALID_SESSION, "Invalid or expired session");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
        return false;
    }
    
    user_id = db->get_user_id_from_session(session_token);
    if (user_id < 0) {
        json error = Protocol::create_error_response(ERR_INVALID_SESSION, "Invalid session");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
        return false;
    }
    
    User user;
    if (!db->get_user_by_id(user_id, user)) {
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "User not found");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
        return false;
    }
    
    role = user.role;
    return true;
}

void Server::broadcast_to_room(int room_id, uint16_t msg_type, const json& payload) {
    if (room_clients.find(room_id) == room_clients.end()) {
        return;
    }
    
    for (int client_fd : room_clients[room_id]) {
        Protocol::send_message(client_fd, msg_type, payload);
    }
}

void Server::broadcast_to_all(uint16_t msg_type, const json& payload) {
    for (const auto& pair : clients) {
        Protocol::send_message(pair.first, msg_type, payload);
    }
}

bool Server::start() {
    if (!setup_server_socket()) {
        return false;
    }
    
    if (!setup_epoll()) {
        return false;
    }
    
    LOG_INFO("Server started successfully");
    
    struct epoll_event events[MAX_EVENTS];
    
    // Main event loop
    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR) {
                continue;
            }
            LOG_ERROR("epoll_wait failed");
            break;
        }
        
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == server_fd) {
                // New connection
                handle_new_connection();
            } else {
                // Client message
                handle_client_message(events[i].data.fd);
            }
        }
        
        // Cleanup expired sessions periodically
        static int cleanup_counter = 0;
        if (++cleanup_counter >= 1000) {
            db->cleanup_expired_sessions();
            cleanup_counter = 0;
        }
    }
    
    return true;
}

void Server::stop() {
    // Close all client connections
    for (const auto& pair : clients) {
        close(pair.first);
    }
    clients.clear();
    
    // Close server socket
    if (server_fd >= 0) {
        close(server_fd);
        server_fd = -1;
    }
    
    // Close epoll
    if (epoll_fd >= 0) {
        close(epoll_fd);
        epoll_fd = -1;
    }
    
    LOG_INFO("Server stopped");
}

// Authentication handlers
void Server::handle_register(int client_fd, const json& payload) {
    try {
        std::string username = payload["username"];
        std::string password = payload["password"];
        std::string role = payload["role"];
        
        // Validate role
        if (role != "USER" && role != "TEACHER") {
            role = "USER";
        }
        
        // Hash password
        std::string hashed_password = SessionManager::hash_password(password);
        
        // Create user
        if (db->create_user(username, hashed_password, role)) {
            json response = Protocol::create_success_response("Registration successful");
            Protocol::send_message(client_fd, S2C_RESPONSE_OK, response);
            LOG_INFO("User registered: " + username);
        } else {
            json error = Protocol::create_error_response(ERR_USERNAME_EXISTS, "Username already exists");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("handle_register error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

void Server::handle_login(int client_fd, const json& payload) {
    try {
        std::string username = payload["username"];
        std::string password = payload["password"];
        
        User user;
        if (!db->get_user_by_username(username, user)) {
            LOG_WARN("Login failed: User not found - " + username);
            json error = Protocol::create_error_response(ERR_LOGIN_FAILED, "Invalid username or password");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
            return;
        }
        
        // Verify password
        if (!SessionManager::verify_password(password, user.hashed_password)) {
            LOG_WARN("Login failed: Password mismatch for user - " + username);
            json error = Protocol::create_error_response(ERR_LOGIN_FAILED, "Invalid username or password");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
            return;
        }
        
        // Generate session token
        std::string token = SessionManager::generate_token(32);
        if (!db->create_session(token, user.user_id, 86400)) { // 24 hours
            json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "Failed to create session");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
            return;
        }
        
        // Update client info
        clients[client_fd].session_token = token;
        clients[client_fd].user_id = user.user_id;
        clients[client_fd].username = user.username;
        clients[client_fd].role = user.role;
        
        // Send response
        json response;
        response["session_token"] = token;
        response["username"] = user.username;
        response["role"] = user.role;
        Protocol::send_message(client_fd, S2C_LOGIN_OK, response);
        
        LOG_INFO("User logged in: " + username);
    } catch (const std::exception& e) {
        LOG_ERROR("handle_login error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

void Server::handle_logout(int client_fd, const json& payload) {
    try {
        std::string session_token = payload["session_token"];
        
        db->delete_session(session_token);
        
        // Clear client info
        if (clients.find(client_fd) != clients.end()) {
            clients[client_fd].session_token = "";
            clients[client_fd].user_id = -1;
        }
        
        json response = Protocol::create_success_response("Logged out successfully");
        Protocol::send_message(client_fd, S2C_RESPONSE_OK, response);
        
        LOG_INFO("User logged out: fd=" + std::to_string(client_fd));
    } catch (const std::exception& e) {
        LOG_ERROR("handle_logout error: " + std::string(e.what()));
    }
}

// Practice mode handlers
void Server::handle_practice_request(int client_fd, const json& payload) {
    try {
        std::string session_token = payload["session_token"];
        int user_id;
        std::string role;
        
        if (!validate_session(client_fd, session_token, user_id, role)) {
            return;
        }
        
        int num_questions = payload["num_questions"];
        std::string topic = payload.value("topic", "all");
        std::string difficulty = payload.value("difficulty", "all");
        
        // Get random questions
        std::vector<Question> questions = db->get_random_questions(num_questions, topic, difficulty);
        
        if (questions.empty()) {
            json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "No questions available");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
            return;
        }
        
        // Prepare response
        json response;
        response["duration_seconds"] = 1800; // 30 minutes
        response["questions"] = json::array();
        
        for (const auto& q : questions) {
            json question_json;
            question_json["q_id"] = q.question_id;
            question_json["content"] = q.content;
            // Add options (without correct answer)
            for (auto it = q.options.begin(); it != q.options.end(); ++it) {
                question_json["option_" + it.key()] = it.value();
            }
            response["questions"].push_back(question_json);
        }
        
        Protocol::send_message(client_fd, S2C_PRACTICE_QUESTIONS, response);
        LOG_INFO("Practice questions sent to user " + std::to_string(user_id));
    } catch (const std::exception& e) {
        LOG_ERROR("handle_practice_request error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

void Server::handle_practice_submit(int client_fd, const json& payload) {
    try {
        std::string session_token = payload["session_token"];
        int user_id;
        std::string role;
        
        if (!validate_session(client_fd, session_token, user_id, role)) {
            return;
        }
        
        json answers = payload["answers"];
        int correct_count = 0;
        int total_questions = answers.size();
        
        // Check each answer
        for (const auto& answer : answers) {
            int q_id = answer["q_id"];
            std::string selected = answer["selected_option"];
            
            Question question;
            if (db->get_question_by_id(q_id, question)) {
                if (selected == "option_" + question.correct_option) {
                    correct_count++;
                }
            }
        }
        
        // Save to history
        float score_percentage = (float)correct_count / total_questions * 100.0f;
        json filters;
        filters["topic"] = payload.value("topic", "all");
        filters["difficulty"] = payload.value("difficulty", "all");
        db->save_practice_result(user_id, correct_count, total_questions, filters.dump(), score_percentage);
        
        // Send result
        json response;
        response["correct_count"] = correct_count;
        response["total_questions"] = total_questions;
        Protocol::send_message(client_fd, S2C_PRACTICE_RESULT, response);
        
        LOG_INFO("Practice submitted: user=" + std::to_string(user_id) + 
                ", score=" + std::to_string(correct_count) + "/" + std::to_string(total_questions));
    } catch (const std::exception& e) {
        LOG_ERROR("handle_practice_submit error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

// Test room handlers
void Server::handle_list_rooms(int client_fd, const json& payload) {
    try {
        std::string session_token = payload["session_token"];
        int user_id;
        std::string role;
        
        if (!validate_session(client_fd, session_token, user_id, role)) {
            return;
        }
        
        std::vector<TestRoom> rooms = db->get_all_rooms();
        
        json response;
        response["rooms"] = json::array();
        
        for (const auto& room : rooms) {
            json room_json;
            room_json["room_id"] = room.room_id;
            room_json["name"] = room.name;
            room_json["status"] = room.status;
            room_json["num_questions"] = room.num_questions;
            room_json["duration_minutes"] = room.duration_minutes;
            response["rooms"].push_back(room_json);
        }
        
        Protocol::send_message(client_fd, S2C_ROOM_LIST, response);
        LOG_INFO("Room list sent to user " + std::to_string(user_id));
    } catch (const std::exception& e) {
        LOG_ERROR("handle_list_rooms error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

void Server::handle_create_room(int client_fd, const json& payload) {
    try {
        std::string session_token = payload["session_token"];
        int user_id;
        std::string role;
        
        if (!validate_session(client_fd, session_token, user_id, role)) {
            return;
        }
        
        // Check if user is TEACHER
        if (role != "TEACHER") {
            json error = Protocol::create_error_response(ERR_NOT_ROOM_OWNER, "Only teachers can create rooms");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
            return;
        }
        
        std::string name = payload["name"];
        int num_questions = payload["num_questions"];
        int duration_minutes = payload["duration_minutes"];
        std::string topic = payload.value("topic", "all");
        std::string difficulty = payload.value("difficulty", "all");
        
        json filters;
        filters["topic"] = topic;
        filters["difficulty"] = difficulty;
        
        int room_id;
        if (db->create_test_room(name, user_id, num_questions, duration_minutes, filters.dump(), room_id)) {
            json response;
            response["room_id"] = room_id;
            response["message"] = "Room created successfully";
            Protocol::send_message(client_fd, S2C_ROOM_CREATED, response);
            
            LOG_INFO("Room created: id=" + std::to_string(room_id) + ", name=" + name);
        } else {
            json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "Failed to create room");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("handle_create_room error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

void Server::handle_join_room(int client_fd, const json& payload) {
    try {
        std::string session_token = payload["session_token"];
        int user_id;
        std::string role;
        
        if (!validate_session(client_fd, session_token, user_id, role)) {
            return;
        }
        
        int room_id = payload["room_id"];
        
        // Get room info
        TestRoom room;
        if (!db->get_room_by_id(room_id, room)) {
            json error = Protocol::create_error_response(ERR_ROOM_NOT_FOUND, "Room not found");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
            return;
        }
        
        // Check if room not started
        if (room.status != "NOT_STARTED") {
            json error = Protocol::create_error_response(ERR_ROOM_STARTED, "Room already started or finished");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
            return;
        }
        
        // Add participant
        if (!db->add_participant(room_id, user_id)) {
            json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "Failed to join room");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
            return;
        }
        
        // Add to room_clients
        room_clients[room_id].insert(client_fd);
        
        // Get all participants
        std::vector<std::string> participants = db->get_room_participants(room_id);
        
        // Send response to joining client
        json response;
        response["room_id"] = room_id;
        response["room_name"] = room.name;
        response["participants"] = participants;
        Protocol::send_message(client_fd, S2C_JOIN_OK, response);
        
        // Broadcast to other participants
        User user;
        if (db->get_user_by_id(user_id, user)) {
            json broadcast;
            broadcast["username"] = user.username;
            broadcast_to_room(room_id, S2C_USER_JOINED_ROOM, broadcast);
        }
        
        LOG_INFO("User " + std::to_string(user_id) + " joined room " + std::to_string(room_id));
    } catch (const std::exception& e) {
        LOG_ERROR("handle_join_room error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

// Placeholder handlers (to be implemented in Phase 5)
void Server::handle_start_test(int client_fd, const json& payload) {
    (void)payload; // Will be used in Phase 5
    LOG_INFO("handle_start_test called (not implemented yet)");
    json response = Protocol::create_success_response("Start test - not implemented");
    Protocol::send_message(client_fd, S2C_RESPONSE_OK, response);
}

void Server::handle_change_answer(int client_fd, const json& payload) {
    (void)client_fd; // Will be used in Phase 5
    (void)payload;   // Will be used in Phase 5
    LOG_INFO("handle_change_answer called (not implemented yet)");
}

void Server::handle_submit_test(int client_fd, const json& payload) {
    (void)payload; // Will be used in Phase 5
    LOG_INFO("handle_submit_test called (not implemented yet)");
    json response = Protocol::create_success_response("Submit test - not implemented");
    Protocol::send_message(client_fd, S2C_RESPONSE_OK, response);
}

void Server::handle_get_history(int client_fd, const json& payload) {
    try {
        std::string session_token = payload["session_token"];
        int user_id;
        std::string role;
        
        if (!validate_session(client_fd, session_token, user_id, role)) {
            return;
        }
        
        json practice_history = db->get_user_practice_history(user_id);
        json test_history = db->get_user_test_history(user_id);
        
        json history = json::array();
        history.insert(history.end(), practice_history.begin(), practice_history.end());
        history.insert(history.end(), test_history.begin(), test_history.end());
        
        json response;
        response["history"] = history;
        Protocol::send_message(client_fd, S2C_HISTORY_DATA, response);
    } catch (const std::exception& e) {
        LOG_ERROR("handle_get_history error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

void Server::handle_get_stats(int client_fd, const json& payload) {
    try {
        std::string session_token = payload["session_token"];
        int user_id;
        std::string role;
        
        if (!validate_session(client_fd, session_token, user_id, role)) {
            return;
        }
        
        json stats = db->get_user_statistics(user_id);
        Protocol::send_message(client_fd, S2C_STATS_DATA, stats);
    } catch (const std::exception& e) {
        LOG_ERROR("handle_get_stats error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

void Server::handle_view_room_results(int client_fd, const json& payload) {
    try {
        std::string session_token = payload["session_token"];
        int user_id;
        std::string role;
        
        if (!validate_session(client_fd, session_token, user_id, role)) {
            return;
        }
        
        int room_id = payload["room_id"];
        
        TestRoom room;
        if (!db->get_room_by_id(room_id, room)) {
            json error = Protocol::create_error_response(ERR_ROOM_NOT_FOUND, "Room not found");
            Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
            return;
        }
        
        json results = db->get_room_results(room_id);
        json response;
        response["room_id"] = room_id;
        response["room_name"] = room.name;
        response["results"] = results;
        Protocol::send_message(client_fd, S2C_ROOM_RESULTS_DATA, response);
    } catch (const std::exception& e) {
        LOG_ERROR("handle_view_room_results error: " + std::string(e.what()));
        json error = Protocol::create_error_response(ERR_SYSTEM_ERROR, "System error");
        Protocol::send_message(client_fd, S2C_RESPONSE_ERROR, error);
    }
}

