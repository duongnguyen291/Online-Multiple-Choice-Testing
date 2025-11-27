-- Database Schema for Online Multiple-Choice Testing Application
-- SQLite3 Database

-- Enable foreign keys
PRAGMA foreign_keys = ON;

-- Bảng Users (Người dùng)
CREATE TABLE IF NOT EXISTS Users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    hashed_password TEXT NOT NULL,
    role TEXT CHECK(role IN ('USER', 'TEACHER')) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Bảng Questions (Câu hỏi)
CREATE TABLE IF NOT EXISTS Questions (
    question_id INTEGER PRIMARY KEY AUTOINCREMENT,
    content TEXT NOT NULL,
    options TEXT NOT NULL, -- JSON: {"a": "...", "b": "...", "c": "...", "d": "..."}
    correct_option TEXT NOT NULL, -- 'a', 'b', 'c', or 'd'
    difficulty TEXT CHECK(difficulty IN ('easy', 'medium', 'hard')) NOT NULL,
    topic TEXT NOT NULL,
    created_by INTEGER,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (created_by) REFERENCES Users(user_id)
);

-- Bảng Sessions (Phiên đăng nhập)
CREATE TABLE IF NOT EXISTS Sessions (
    session_token TEXT PRIMARY KEY,
    user_id INTEGER NOT NULL,
    expiry_timestamp TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE
);

-- Bảng PracticeHistory (Lịch sử Luyện tập)
CREATE TABLE IF NOT EXISTS PracticeHistory (
    practice_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    correct_count INTEGER NOT NULL,
    total_questions INTEGER NOT NULL,
    filters_used TEXT, -- JSON: {"topic": "...", "difficulty": "..."}
    score_percentage REAL,
    completed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE
);

-- Bảng TestRooms (Phòng thi)
CREATE TABLE IF NOT EXISTS TestRooms (
    room_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    creator_id INTEGER NOT NULL,
    status TEXT CHECK(status IN ('NOT_STARTED', 'ONGOING', 'FINISHED')) NOT NULL DEFAULT 'NOT_STARTED',
    num_questions INTEGER NOT NULL,
    duration_minutes INTEGER NOT NULL,
    filters_used TEXT, -- JSON: {"topic": "...", "difficulty": "..."}
    start_timestamp TIMESTAMP,
    end_timestamp TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    question_bank_id INTEGER,
    FOREIGN KEY (creator_id) REFERENCES Users(user_id) ON DELETE CASCADE
);

-- Bảng RoomParticipants (Người tham gia phòng thi)
CREATE TABLE IF NOT EXISTS RoomParticipants (
    room_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    status TEXT CHECK(status IN ('JOINED', 'SUBMITTED')) NOT NULL DEFAULT 'JOINED',
    score INTEGER,
    time_spent INTEGER, -- Thời gian tham gia (seconds)
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (room_id, user_id),
    FOREIGN KEY (room_id) REFERENCES TestRooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE
);

-- Bảng TestRoomQuestions (Đề thi của phòng)
CREATE TABLE IF NOT EXISTS TestRoomQuestions (
    room_id INTEGER NOT NULL,
    question_id INTEGER NOT NULL,
    question_order INTEGER,
    PRIMARY KEY (room_id, question_id),
    FOREIGN KEY (room_id) REFERENCES TestRooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY (question_id) REFERENCES Questions(question_id) ON DELETE CASCADE
);

-- Bảng UserTestAnswers (Bài làm của Người dùng)
CREATE TABLE IF NOT EXISTS UserTestAnswers (
    user_id INTEGER NOT NULL,
    room_id INTEGER NOT NULL,
    question_id INTEGER NOT NULL,
    selected_option TEXT, -- 'a', 'b', 'c', 'd'
    is_correct INTEGER, -- 0 or 1 (boolean)
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, room_id, question_id),
    FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (room_id) REFERENCES TestRooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY (question_id) REFERENCES Questions(question_id) ON DELETE CASCADE
);

-- Indexes for better performance
CREATE INDEX IF NOT EXISTS idx_sessions_user_id ON Sessions(user_id);
CREATE INDEX IF NOT EXISTS idx_sessions_expiry ON Sessions(expiry_timestamp);
CREATE INDEX IF NOT EXISTS idx_questions_topic ON Questions(topic);
CREATE INDEX IF NOT EXISTS idx_questions_difficulty ON Questions(difficulty);
CREATE INDEX IF NOT EXISTS idx_testrooms_status ON TestRooms(status);
CREATE INDEX IF NOT EXISTS idx_testrooms_creator ON TestRooms(creator_id);
CREATE INDEX IF NOT EXISTS idx_practice_user ON PracticeHistory(user_id);

