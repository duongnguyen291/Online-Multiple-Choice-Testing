Database Schema
@startuml
' Database Schema for Online Multiple-Choice Testing Application

entity "Users" as users {
    +user_id: INT
    username: VARCHAR(255)
    hashed_password: VARCHAR(255)
    role: VARCHAR(10)
    created_at: TIMESTAMP
}

entity "Questions" as questions {
    +question_id: INT
    content: TEXT
    options: JSON
    correct_option: VARCHAR(1)
    difficulty: VARCHAR(10)
    topic: VARCHAR(255)
    created_by: INT
    created_at: TIMESTAMP
}

entity "Sessions" as sessions {
    +session_token: VARCHAR(255)
    user_id: INT
    expiry_timestamp: TIMESTAMP
    created_at: TIMESTAMP
}

entity "PracticeHistory" as practice_history {
    +practice_id: INT
    user_id: INT
    correct_count: INT
    total_questions: INT
    filters_used: JSON
    completed_at: TIMESTAMP
    score_percentage: FLOAT
}

entity "TestRooms" as test_rooms {
    +room_id: INT
    name: VARCHAR(255)
    creator_id: INT
    status: VARCHAR(20)
    num_questions: INT
    duration_minutes: INT
    filters_used: JSON
    start_timestamp: TIMESTAMP
    end_timestamp: TIMESTAMP
    created_at: TIMESTAMP
    question_bank_id: INT
}

entity "RoomParticipants" as room_participants {
    +room_id: INT
    +user_id: INT
    status: VARCHAR(20)
    score: INT
    joined_at: TIMESTAMP
    time_spent: INT
}

entity "TestRoomQuestions" as test_room_questions {
    +room_id: INT
    +question_id: INT
    question_order: INT
}

entity "UserTestAnswers" as user_test_answers {
    +user_id: INT
    +room_id: INT
    +question_id: INT
    selected_option: VARCHAR(1)
    last_updated: TIMESTAMP
    is_correct: BOOLEAN
}

' Relationships
users ||--o{ questions : created_by
users ||--o{ sessions : user_id
users ||--o{ practice_history : user_id
users ||--o{ room_participants : user_id
test_rooms ||--o{ room_participants : room_id
test_rooms ||--o{ test_room_questions : room_id
test_rooms ||--o{ user_test_answers : room_id
questions ||--o{ test_room_questions : question_id
questions ||--o{ user_test_answers : question_id
practice_history ||--o{ practice_history : user_id
sessions ||--o{ sessions : user_id
@enduml

  
Luồng Đăng Nhập và Quản Lí Phiên

@startuml
title 1. Luong Dang Nhap & Quan Ly Session

autonumber
skinparam sequenceMessageAlign center

actor Client
participant Server
database Database

activate Client
Client -> Server: C2S_LOGIN (username, password)
activate Server

Server -> Database: SELECT user, hashed_password\nWHERE username = ?
activate Database
Database --> Server: User info (neu ton tai)
deactivate Database

Server -> Server: Kiem tra password (bcrypt/hash)
opt Neu xac thuc thanh cong
    Server -> Server: Tao session_token duy nhat
    Server -> Database: INSERT vao bang Sessions\n(token, user_id, expiry)
    activate Database
    Database --> Server: OK
    deactivate Database
    
    Server --> Client: S2C_LOGIN_OK (session_token, role)
    Client -> Client: Luu lai session_token
else Neu xac thuc that bai
    Server --> Client: S2C_RESPONSE_ERROR (1001, "Sai ten dang nhap hoac mat khau")
end

deactivate Server
deactivate Client

@enduml



Luồng Luyện Tập

@startuml
title 2. Luong Luyen Tap (Practice Mode)

autonumber
skinparam sequenceMessageAlign center

actor Client
participant Server
database Database

activate Client
Client -> Server: C2S_PRACTICE_REQUEST (session_token, filters)
activate Server

Server -> Server: Xac thuc session_token
Server -> Database: SELECT * FROM Questions\n(dua tren filters)
activate Database
Database --> Server: List [Questions]
deactivate Database

Server --> Client: S2C_PRACTICE_QUESTIONS (questions, duration)
deactivate Server

Client -> Client: loop (Nguoi dung lam bai...)

Client -> Server: C2S_PRACTICE_SUBMIT (session_token, answers)
activate Server
Server -> Server: Xac thuc token
Server -> Server: Cham diem (so sanh answers voi DB)
note right of Server: Server co the cache cau tra loi dung\nhoac query lai DB

Server -> Database: INSERT vao PracticeHistory\n(user_id, correct_count, total)
activate Database
Database --> Server: OK
deactivate Database

Server --> Client: S2C_PRACTICE_RESULT (correct_count, total)
deactivate Server
deactivate Client
@enduml



Luồng Phòng thi: Tham gia & Bắt đầu
@startuml
actor "Client A (Owner)" as A
actor "Client B (Participant)" as B
entity "Server" as S
entity "Database" as DB

A -> S : C2S_CREATE_ROOM
S -> DB : Insert new room (TestRooms)
S -> A : S2C_ROOM_CREATED

B -> S : C2S_LIST_ROOMS
S -> DB : Query available rooms (TestRooms)
S -> B : S2C_ROOM_LIST

B -> S : C2S_JOIN_ROOM
S -> DB : Insert participant (RoomParticipants)
S -> B : S2C_JOIN_OK

S -> A : S2C_USER_JOINED_ROOM (Push)

A -> S : C2S_START_TEST
S -> DB : Validate owner, fetch questions (TestRoomQuestions)
S -> S : Start Timer
S -> A : S2C_TEST_STARTED (Push with questions)
S -> B : S2C_TEST_STARTED (Push with questions)
S -> DB : Update room status (TestRooms)

@enduml



Luồng Phòng thi: Làm bài & Hết giờ
Sơ đồ này mô tả những gì xảy ra trong khi thi và luồng sự kiện do server chủ động kích hoạt (hết giờ).

@startuml
actor "Client A (Owner)" as A
actor "Client B (Participant)" as B
entity "Server" as S
entity "Database" as DB

B -> S : C2S_CHANGE_ANSWER (question 1, option A)
S -> DB : Update UserTestAnswers

B -> S : C2S_SUBMIT_TEST
S -> DB : Update RoomParticipants (score = submitted)
S -> B : S2C_RESPONSE_OK

S -> S : Timer Ends
S -> DB : Auto grade (update scores for all participants)
S -> DB : Update room status (TestRooms)
S -> S : S2C_TEST_ENDED (Push)

S -> A : S2C_YOUR_RESULT (Push)
S -> B : S2C_YOUR_RESULT (Push)
@enduml





Luồng Xem Kết quả Phòng thi 

@startuml
actor "Client" as C
entity "Server" as S
entity "Database" as DB

C -> S : C2S_VIEW_ROOM_RESULTS (room_id, token)
S -> DB : Validate token
S -> DB : Query TestRooms (status = 'FINISHED')
S -> DB : Query results (RoomParticipants, Users)
S -> C : S2C_ROOM_RESULTS_DATA (results array)

@enduml




