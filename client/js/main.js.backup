/**
 * Main Application Module
 */

// Current page
let currentPage = 'login';

/**
 * Show notification message
 */
function showNotification(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.textContent = message;
    
    document.body.appendChild(notification);
    
    // Show notification
    setTimeout(() => {
        notification.classList.add('show');
    }, 10);
    
    // Remove after 3 seconds
    setTimeout(() => {
        notification.classList.remove('show');
        setTimeout(() => {
            notification.remove();
        }, 300);
    }, 3000);
}

/**
 * Navigate to page
 */
function navigateTo(page) {
    // Hide all pages
    document.querySelectorAll('.page').forEach(el => {
        el.classList.remove('active');
    });
    
    // Show target page
    const targetPage = document.getElementById(`page-${page}`);
    if (targetPage) {
        targetPage.classList.add('active');
        currentPage = page;
        
        // Call page load handler
        if (window[`on${page.charAt(0).toUpperCase() + page.slice(1)}Load`]) {
            window[`on${page.charAt(0).toUpperCase() + page.slice(1)}Load`]();
        }
    }
}

/**
 * Initialize application
 */
async function initializeApp() {
    try {
        // Connect to WebSocket gateway
        await initWebSocket();
        
        // Check if already logged in
        if (AuthModule.isLoggedIn()) {
            navigateTo('dashboard');
        } else {
            navigateTo('login');
        }
        
        // Setup disconnect handler
        window.addEventListener('ws-disconnected', () => {
            showNotification('Mất kết nối đến server. Vui lòng tải lại trang.', 'error');
            navigateTo('login');
        });
    } catch (error) {
        console.error('Failed to initialize app:', error);
        showNotification('Không thể kết nối đến server', 'error');
        navigateTo('login');
    }
}

// Initialize app when DOM is ready
document.addEventListener('DOMContentLoaded', initializeApp);

/**
 * Page load handlers
 */

function onLoginLoad() {
    const form = document.querySelector('#page-login form');
    if (form) {
        form.onsubmit = async (e) => {
            e.preventDefault();
            const username = document.getElementById('login-username').value;
            const password = document.getElementById('login-password').value;
            
            try {
                await AuthModule.login(username, password);
                navigateTo('dashboard');
            } catch (error) {
                // Error already shown by AuthModule
            }
        };
    }
    
    // Show register link
    const registerLink = document.querySelector('#page-login .register-link');
    if (registerLink) {
        registerLink.onclick = () => navigateTo('register');
    }
}

function onRegisterLoad() {
    const form = document.querySelector('#page-register form');
    if (form) {
        form.onsubmit = async (e) => {
            e.preventDefault();
            const username = document.getElementById('register-username').value;
            const password = document.getElementById('register-password').value;
            const confirmPassword = document.getElementById('register-confirm-password').value;
            const role = document.getElementById('register-role').value;
            
            try {
                await AuthModule.register(username, password, confirmPassword, role);
                navigateTo('login');
            } catch (error) {
                // Error already shown by AuthModule
            }
        };
    }
    
    // Show login link
    const loginLink = document.querySelector('#page-register .login-link');
    if (loginLink) {
        loginLink.onclick = () => navigateTo('login');
    }
}

function onDashboardLoad() {
    // Update user info
    const userInfo = document.querySelector('#page-dashboard .user-info');
    if (userInfo) {
        userInfo.innerHTML = `
            <span>Xin chào, <strong>${AuthModule.getUsername()}</strong> (${AuthModule.getRole()})</span>
        `;
    }
    
    // Update menu based on role
    const menuTeacher = document.querySelectorAll('.menu-teacher');
    const menuUser = document.querySelectorAll('.menu-user');
    
    if (AuthModule.isTeacher()) {
        menuTeacher.forEach(el => el.style.display = 'block');
        menuUser.forEach(el => el.style.display = 'none');
    } else {
        menuTeacher.forEach(el => el.style.display = 'none');
        menuUser.forEach(el => el.style.display = 'block');
    }
    
    // Logout button
    const logoutBtn = document.querySelector('#btn-logout');
    if (logoutBtn) {
        logoutBtn.onclick = () => AuthModule.logout();
    }
    
    // Navigation buttons
    setupNavigationButtons();
}

function setupNavigationButtons() {
    // Practice
    document.getElementById('btn-practice')?.addEventListener('click', () => {
        navigateTo('practice');
    });
    
    // Test rooms
    document.getElementById('btn-test-rooms')?.addEventListener('click', () => {
        navigateTo('test-rooms');
    });
    
    // History
    document.getElementById('btn-history')?.addEventListener('click', () => {
        navigateTo('history');
    });
    
    // Statistics
    document.getElementById('btn-statistics')?.addEventListener('click', () => {
        navigateTo('statistics');
    });
    
    // Create room (Teacher only)
    document.getElementById('btn-create-room')?.addEventListener('click', () => {
        navigateTo('create-room');
    });
    
    // Back to dashboard
    document.querySelectorAll('.btn-back-dashboard')?.forEach(btn => {
        btn.addEventListener('click', () => navigateTo('dashboard'));
    });
}

function onPracticeLoad() {
    // Load practice setup page
    const startBtn = document.getElementById('btn-practice-start');
    if (startBtn) {
        startBtn.onclick = async () => {
            const numQuestions = parseInt(document.getElementById('practice-num-questions').value) || 5;
            const topic = document.getElementById('practice-topic').value || 'all';
            const difficulty = document.getElementById('practice-difficulty').value || 'all';
            
            try {
                // Show loading
                document.getElementById('page-practice').innerHTML = '<p class="loading">Đang tải câu hỏi...</p>';
                
                // Send practice request
                ws.send(C2S_PRACTICE_REQUEST, {
                    num_questions: numQuestions,
                    topic: topic,
                    difficulty: difficulty
                });
                
                // Wait for questions response
                const handler = (msg) => {
                    ws.off(S2C_PRACTICE_QUESTIONS, handler);
                    showPracticeExam(msg);
                };
                ws.on(S2C_PRACTICE_QUESTIONS, handler);
                
                // Timeout
                setTimeout(() => {
                    ws.off(S2C_PRACTICE_QUESTIONS, handler);
                    showNotification('Timeout khi tải câu hỏi', 'error');
                }, 10000);
            } catch (error) {
                showNotification('Lỗi: ' + error.message, 'error');
            }
        };
    }
}

function showPracticeExam(questionsData) {
    const questions = questionsData.questions || [];
    const durationSeconds = questionsData.duration_seconds || 1800;
    
    let html = `
        <div class="exam-header">
            <h2>Bài Luyện Tập</h2>
            <div class="exam-timer">
                Thời gian còn lại: <span id="timer">${Math.floor(durationSeconds / 60)}:${(durationSeconds % 60).toString().padStart(2, '0')}</span>
            </div>
        </div>
        <div class="exam-questions" id="exam-questions">
    `;
    
    questions.forEach((q, index) => {
        html += `
            <div class="question-block">
                <h3>Câu ${index + 1}: ${q.content}</h3>
                <div class="options">
        `;
        
        // Parse options
        const optionsObj = typeof q.options === 'string' ? JSON.parse(q.options) : q.options;
        for (const [key, value] of Object.entries(optionsObj)) {
            html += `
                <label class="option">
                    <input type="radio" name="q${q.q_id}" value="${key}">
                    ${value}
                </label>
            `;
        }
        
        html += '</div></div>';
    });
    
    html += `
        </div>
        <div class="exam-actions">
            <button id="btn-practice-submit" class="btn btn-primary">Nộp Bài</button>
        </div>
    `;
    
    document.getElementById('page-practice').innerHTML = html;
    
    // Start timer
    let timeLeft = durationSeconds;
    const timerInterval = setInterval(() => {
        timeLeft--;
        const minutes = Math.floor(timeLeft / 60);
        const seconds = timeLeft % 60;
        document.getElementById('timer').textContent = `${minutes}:${seconds.toString().padStart(2, '0')}`;
        
        if (timeLeft <= 0) {
            clearInterval(timerInterval);
            submitPracticeExam();
        }
    }, 1000);
    
    // Submit button
    document.getElementById('btn-practice-submit').onclick = () => {
        clearInterval(timerInterval);
        submitPracticeExam();
    };
}

function submitPracticeExam() {
    // Collect answers
    const answers = [];
    const questions = document.querySelectorAll('.question-block');
    let questionIndex = 0;
    
    questions.forEach(q => {
        const checked = q.querySelector('input[type=radio]:checked');
        if (checked) {
            // Extract question ID from radio name (q{id})
            const qId = parseInt(checked.name.substring(1));
            answers.push({
                q_id: qId,
                selected_option: checked.value
            });
        }
        questionIndex++;
    });
    
    // Send submit request
    ws.send(C2S_PRACTICE_SUBMIT, { answers: answers });
    
    // Wait for result
    const handler = (msg) => {
        ws.off(S2C_PRACTICE_RESULT, handler);
        showPracticeResult(msg);
    };
    ws.on(S2C_PRACTICE_RESULT, handler);
}

function showPracticeResult(result) {
    const html = `
        <div class="result-card">
            <h2>Kết Quả Luyện Tập</h2>
            <div class="result-score">
                <span class="score">${result.correct_count}/${result.total_questions}</span>
                <span class="percentage">${Math.round(result.correct_count / result.total_questions * 100)}%</span>
            </div>
            <button class="btn btn-primary" onclick="navigateTo('dashboard')">Quay Lại</button>
        </div>
    `;
    
    document.getElementById('page-practice').innerHTML = html;
}

function onTestRoomsLoad() {
    loadRoomList();
}

function loadRoomList() {
    document.getElementById('page-test-rooms').innerHTML = '<p class="loading">Đang tải danh sách phòng...</p>';
    
    ws.send(C2S_LIST_ROOMS, {});
    
    const handler = (msg) => {
        ws.off(S2C_ROOM_LIST, handler);
        showRoomList(msg.rooms || []);
    };
    ws.on(S2C_ROOM_LIST, handler);
    
    setTimeout(() => {
        ws.off(S2C_ROOM_LIST, handler);
    }, 10000);
}

function showRoomList(rooms) {
    let html = `
        <div class="room-list-header">
            <h2>Danh Sách Phòng Thi</h2>
            ${AuthModule.isTeacher() ? `<button id="btn-create-room-inline" class="btn btn-primary">Tạo Phòng</button>` : ''}
        </div>
        <table class="room-table">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Tên Phòng</th>
                    <th>Trạng Thái</th>
                    <th>Câu Hỏi</th>
                    <th>Thời Gian</th>
                    <th>Hành Động</th>
                </tr>
            </thead>
            <tbody>
    `;
    
    if (rooms.length === 0) {
        html += '<tr><td colspan="6" class="empty">Không có phòng thi nào</td></tr>';
    } else {
        rooms.forEach(room => {
            const statusColor = room.status === ROOM_STATUS_ONGOING ? 'orange' :
                               room.status === ROOM_STATUS_FINISHED ? 'gray' : 'green';
            
            html += `
                <tr>
                    <td>${room.room_id}</td>
                    <td>${room.name}</td>
                    <td><span class="status status-${room.status.toLowerCase()}">${room.status}</span></td>
                    <td>${room.num_questions} câu</td>
                    <td>${room.duration_minutes} phút</td>
                    <td>
                        ${room.status === ROOM_STATUS_NOT_STARTED ? 
                            `<button class="btn btn-small" onclick="joinRoom(${room.room_id})">Vào</button>` :
                            `<button class="btn btn-small" onclick="viewRoomDetails(${room.room_id})">Xem</button>`
                        }
                    </td>
                </tr>
            `;
        });
    }
    
    html += `
            </tbody>
        </table>
    `;
    
    document.getElementById('page-test-rooms').innerHTML = html;
    
    // Create room button
    document.getElementById('btn-create-room-inline')?.addEventListener('click', () => {
        navigateTo('create-room');
    });
}

function joinRoom(roomId) {
    ws.send(C2S_JOIN_ROOM, { room_id: roomId });
    
    const handler = (msg) => {
        ws.off(S2C_JOIN_OK, handler);
        showWaitingRoom(msg);
    };
    const errorHandler = (msg) => {
        ws.off(S2C_RESPONSE_ERROR, errorHandler);
        showNotification('Không thể vào phòng: ' + msg.message, 'error');
    };
    
    ws.on(S2C_JOIN_OK, handler);
    ws.on(S2C_RESPONSE_ERROR, errorHandler);
}

function showWaitingRoom(roomData) {
    const isOwner = roomData.room_id && roomData.creator_id === AuthModule.getUserId();
    
    let html = `
        <div class="waiting-room">
            <h2>${roomData.room_name}</h2>
            <div class="room-info">
                <p>Số câu hỏi: ${roomData.num_questions}</p>
                <p>Thời gian: ${roomData.duration_minutes} phút</p>
            </div>
            <div class="participants">
                <h3>Những người tham gia (${roomData.participants?.length || 0}):</h3>
                <ul>
                    ${(roomData.participants || []).map(p => `<li>${p}</li>`).join('')}
                </ul>
            </div>
            ${isOwner ? `<button id="btn-start-test" class="btn btn-primary">Bắt Đầu Thi</button>` : ''}
            <button class="btn" onclick="navigateTo('test-rooms')">Quay Lại</button>
        </div>
    `;
    
    document.getElementById('page-test-rooms').innerHTML = html;
    
    if (isOwner) {
        document.getElementById('btn-start-test')?.addEventListener('click', () => {
            ws.send(C2S_START_TEST, { room_id: roomData.room_id });
        });
    }
    
    // Listen for user joined notifications
    const joinHandler = (msg) => {
        showNotification(`${msg.username} đã vào phòng`, 'info');
        loadRoomList(); // Refresh
    };
    ws.on(S2C_USER_JOINED_ROOM, joinHandler);
    
    // Listen for test start
    const testHandler = (msg) => {
        ws.off(S2C_TEST_STARTED, testHandler);
        showTestExam(msg);
    };
    ws.on(S2C_TEST_STARTED, testHandler);
}

function showTestExam(testData) {
    // Similar to practice exam
    // ... implementation
}

function onHistoryLoad() {
    loadHistory();
}

function loadHistory() {
    document.getElementById('page-history').innerHTML = '<p class="loading">Đang tải lịch sử...</p>';
    
    ws.send(C2S_GET_HISTORY, {});
    
    const handler = (msg) => {
        ws.off(S2C_HISTORY_DATA, handler);
        showHistory(msg.history || []);
    };
    ws.on(S2C_HISTORY_DATA, handler);
}

function showHistory(history) {
    let html = `
        <div class="history-container">
            <h2>Lịch Sử Làm Bài</h2>
            <table class="history-table">
                <thead>
                    <tr>
                        <th>Ngày</th>
                        <th>Loại</th>
                        <th>Phòng/Bài</th>
                        <th>Điểm</th>
                    </tr>
                </thead>
                <tbody>
    `;
    
    if (history.length === 0) {
        html += '<tr><td colspan="4" class="empty">Chưa có lịch sử</td></tr>';
    } else {
        history.forEach(item => {
            html += `
                <tr>
                    <td>${item.completed_at || item.date}</td>
                    <td>${item.mode || item.type}</td>
                    <td>${item.room_name || 'Luyện Tập'}</td>
                    <td>${item.score}</td>
                </tr>
            `;
        });
    }
    
    html += `
                </tbody>
            </table>
        </div>
    `;
    
    document.getElementById('page-history').innerHTML = html;
}

function onStatisticsLoad() {
    loadStatistics();
}

function loadStatistics() {
    document.getElementById('page-statistics').innerHTML = '<p class="loading">Đang tải thống kê...</p>';
    
    ws.send(C2S_GET_STATS, {});
    
    const handler = (msg) => {
        ws.off(S2C_STATS_DATA, handler);
        showStatistics(msg);
    };
    ws.on(S2C_STATS_DATA, handler);
}

function showStatistics(statsData) {
    let html = `
        <div class="statistics-container">
            <h2>Thống Kê</h2>
            <div class="chart-container">
                <canvas id="score-chart"></canvas>
            </div>
            <div class="chart-container">
                <canvas id="topic-chart"></canvas>
            </div>
        </div>
    `;
    
    document.getElementById('page-statistics').innerHTML = html;
    
    // Draw charts if Chart.js is loaded
    if (typeof Chart !== 'undefined') {
        // Score over time chart
        if (statsData.score_over_time) {
            new Chart(document.getElementById('score-chart'), {
                type: 'line',
                data: {
                    labels: statsData.score_over_time.map(d => d.date),
                    datasets: [{
                        label: 'Điểm (%)',
                        data: statsData.score_over_time.map(d => d.score_percent),
                        borderColor: 'rgb(75, 192, 192)',
                        tension: 0.1
                    }]
                }
            });
        }
        
        // Topic distribution chart
        if (statsData.topic_distribution) {
            new Chart(document.getElementById('topic-chart'), {
                type: 'bar',
                data: {
                    labels: statsData.topic_distribution.map(d => d.topic),
                    datasets: [{
                        label: 'Phần Trăm Đúng (%)',
                        data: statsData.topic_distribution.map(d => d.correct_percent),
                        backgroundColor: 'rgba(75, 192, 192, 0.6)'
                    }]
                }
            });
        }
    }
}

function onCreateRoomLoad() {
    const form = document.querySelector('#page-create-room form');
    if (form) {
        form.onsubmit = async (e) => {
            e.preventDefault();
            const name = document.getElementById('room-name').value;
            const numQuestions = parseInt(document.getElementById('room-num-questions').value);
            const durationMinutes = parseInt(document.getElementById('room-duration-minutes').value);
            const topic = document.getElementById('room-topic').value;
            const difficulty = document.getElementById('room-difficulty').value;
            
            ws.send(C2S_CREATE_ROOM, {
                name, num_questions: numQuestions, duration_minutes: durationMinutes, topic, difficulty
            });
            
            const handler = (msg) => {
                ws.off(S2C_ROOM_CREATED, handler);
                showNotification('Tạo phòng thành công!', 'success');
                navigateTo('test-rooms');
            };
            ws.on(S2C_ROOM_CREATED, handler);
        };
    }
}
