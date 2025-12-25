/**
 * Main Application Module - Redesigned for Sidebar Layout
 */

// Global variables
let wsClient = null;
let currentUser = null;
let currentPage = 'statistics';
let practiceTimer = null;
let testTimer = null;

/**
 * Show notification message
 */
function showNotification(message, type = 'info') {
    const notification = document.getElementById('notification');
    if (!notification) return;

    notification.textContent = message;
    notification.className = `notification show ${type}`;

    setTimeout(() => {
        notification.classList.remove('show');
    }, 3000);
}

/**
 * Navigate to page (for sidebar navigation)
 */
function navigateTo(pageName) {
    // Hide all content pages
    document.querySelectorAll('.content-page').forEach(page => {
        page.classList.remove('active');
    });

    // Remove active from all nav items
    document.querySelectorAll('.nav-item').forEach(item => {
        item.classList.remove('active');
    });

    // Show selected page
    const page = document.getElementById(`content-${pageName}`);
    if (page) {
        page.classList.add('active');
        currentPage = pageName;
    }

    // Mark nav item as active
    const navItem = document.querySelector(`.nav-item[data-page="${pageName}"]`);
    if (navItem) {
        navItem.classList.add('active');
    }

    // Update page title
    const titles = {
        'statistics': 'Thống Kê',
        'questions': 'Quản Lý Câu Hỏi',
        'practice': 'Luyện Tập',
        'test-rooms': 'Phòng Thi',
        'history': 'Lịch Sử'
    };
    const titleEl = document.getElementById('page-title');
    if (titleEl) {
        titleEl.textContent = titles[pageName] || pageName;
    }

    // Load page data
    onPageLoad(pageName);
}

/**
 * Page load handler
 */
function onPageLoad(pageName) {
    switch (pageName) {
        case 'statistics':
            loadStatistics();
            break;
        case 'questions':
            if (window.questionsManager) {
                questionsManager.loadQuestions();
            }
            break;
        case 'practice':
            resetPracticeSection();
            break;
        case 'test-rooms':
            loadRoomList();
            break;
        case 'history':
            loadHistory();
            break;
    }
}

/**
 * Initialize Application
 */
async function initializeApp() {
    console.log('Initializing app...');

    // Check if already logged in
    const sessionToken = localStorage.getItem('sessionToken');
    const username = localStorage.getItem('username');
    const role = localStorage.getItem('role');

    if (sessionToken && username && role) {
        // Already logged in, show dashboard
        currentUser = { username, role, sessionToken };
        showDashboard();
        await connectWebSocket();
    } else {
        // Show login page
        showAuthPages();
    }

    // Setup auth page event listeners
    setupAuthPages();
}

/**
 * Show auth pages (login/register)
 */
function showAuthPages() {
    document.getElementById('auth-container').style.display = 'flex';
    document.getElementById('app-container').style.display = 'none';

    // Show login by default
    document.getElementById('page-login').classList.add('active');
    document.getElementById('page-register').classList.remove('active');
}

/**
 * Show dashboard after login
 */
function showDashboard() {
    document.getElementById('auth-container').style.display = 'none';
    document.getElementById('app-container').style.display = 'flex';

    // Update sidebar user info
    document.getElementById('sidebar-username').textContent = currentUser.username;
    const roleText = currentUser.role === 'TEACHER' ? 'Giáo viên' : 'Học viên';
    document.getElementById('sidebar-role').textContent = roleText;

    // Show appropriate menu
    if (currentUser.role === 'TEACHER') {
        document.getElementById('student-menu').style.display = 'none';
        document.getElementById('teacher-menu').style.display = 'block';

        // Initialize questions manager for teachers
        if (wsClient && typeof QuestionsManager !== 'undefined') {
            window.questionsManager = new QuestionsManager(wsClient);
        }
    } else {
        document.getElementById('student-menu').style.display = 'block';
        document.getElementById('teacher-menu').style.display = 'none';
    }

    // Setup sidebar navigation
    setupSidebarNavigation();

    // Navigate to default page (statistics)
    navigateTo('statistics');
}

/**
 * Setup auth pages (login/register)
 */
function setupAuthPages() {
    // Login form
    const loginForm = document.getElementById('login-form');
    if (loginForm) {
        loginForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            await handleLogin();
        });
    }

    // Register form
    const registerForm = document.getElementById('register-form');
    if (registerForm) {
        registerForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            await handleRegister();
        });
    }

    // Switch to register
    const showRegisterBtn = document.getElementById('show-register');
    if (showRegisterBtn) {
        showRegisterBtn.addEventListener('click', (e) => {
            e.preventDefault();
            document.getElementById('page-login').classList.remove('active');
            document.getElementById('page-register').classList.add('active');
        });
    }

    // Switch to login
    const showLoginBtn = document.getElementById('show-login');
    if (showLoginBtn) {
        showLoginBtn.addEventListener('click', (e) => {
            e.preventDefault();
            document.getElementById('page-register').classList.remove('active');
            document.getElementById('page-login').classList.add('active');
        });
    }
}

/**
 * Setup sidebar navigation
 */
function setupSidebarNavigation() {
    // Sidebar nav items
    document.querySelectorAll('.nav-item').forEach(item => {
        item.addEventListener('click', (e) => {
            e.preventDefault();
            const page = item.getAttribute('data-page');
            if (page) {
                navigateTo(page);
            }
        });
    });

    // Logout button
    const logoutBtn = document.getElementById('btn-logout');
    if (logoutBtn) {
        logoutBtn.addEventListener('click', async () => {
            await handleLogout();
        });
    }
}

/**
 * Connect to WebSocket server
 */
async function connectWebSocket() {
    try {
        wsClient = new WebSocketClient('ws://localhost:8080');

        // Register message handlers
        wsClient.on(S2C_LOGIN_OK, handleLoginSuccess);
        wsClient.on(S2C_RESPONSE_ERROR, handleError);
        wsClient.on(S2C_PRACTICE_QUESTIONS, handlePracticeQuestions);
        wsClient.on(S2C_PRACTICE_RESULT, handlePracticeResult);
        wsClient.on(S2C_ROOM_LIST, handleRoomList);
        wsClient.on(S2C_ROOM_CREATED, handleRoomCreated);
        wsClient.on(S2C_JOIN_OK, handleJoinOk);
        wsClient.on(S2C_USER_JOINED_ROOM, handleUserJoinedRoom);
        wsClient.on(S2C_TEST_STARTED, handleTestStarted);
        wsClient.on(S2C_HISTORY_DATA, handleHistoryData);
        wsClient.on(S2C_STATS_DATA, handleStatsData);

        await wsClient.connect();

        // Update connection status
        const statusEl = document.getElementById('connection-status');
        if (statusEl) {
            statusEl.textContent = '🟢 Đã kết nối';
            statusEl.className = 'status-badge';
        }

        console.log('WebSocket connected');
    } catch (error) {
        console.error('WebSocket connection failed:', error);
        showNotification('Không thể kết nối tới server', 'error');

        const statusEl = document.getElementById('connection-status');
        if (statusEl) {
            statusEl.textContent = '🔴 Mất kết nối';
            statusEl.className = 'status-badge error';
        }
    }
}

/**
 * Handle login
 */
async function handleLogin() {
    const username = document.getElementById('login-username').value.trim();
    const password = document.getElementById('login-password').value;

    if (!username || !password) {
        showNotification('Vui lòng nhập đầy đủ thông tin', 'error');
        return;
    }

    try {
        // Connect WebSocket first
        if (!wsClient) {
            wsClient = new WebSocketClient('ws://localhost:8080');
            await wsClient.connect();
        }

        // Send login request
        const result = await wsClient.login(username, password);

        if (result.success) {
            // Save to localStorage
            localStorage.setItem('sessionToken', result.sessionToken);
            localStorage.setItem('username', result.username);
            localStorage.setItem('role', result.role);

            currentUser = {
                username: result.username,
                role: result.role,
                sessionToken: result.sessionToken
            };

            showNotification('Đăng nhập thành công!', 'success');
            showDashboard();
        } else {
            showNotification(result.message || 'Đăng nhập thất bại', 'error');
        }
    } catch (error) {
        console.error('Login error:', error);
        const msg = error.message || error.code || 'Lỗi kết nối server';
        showNotification('Lỗi đăng nhập: ' + msg, 'error');
    }
}

/**
 * Handle register
 */
async function handleRegister() {
    const username = document.getElementById('register-username').value.trim();
    const password = document.getElementById('register-password').value;
    const confirmPassword = document.getElementById('register-confirm').value;
    const role = document.getElementById('register-role').value;

    if (!username || !password || !confirmPassword) {
        showNotification('Vui lòng nhập đầy đủ thông tin', 'error');
        return;
    }

    if (password !== confirmPassword) {
        showNotification('Mật khẩu xác nhận không khớp', 'error');
        return;
    }

    if (password.length < 6) {
        showNotification('Mật khẩu phải có ít nhất 6 ký tự', 'error');
        return;
    }

    try {
        // Connect WebSocket first
        if (!wsClient) {
            wsClient = new WebSocketClient('ws://localhost:8080');
            await wsClient.connect();
        }

        // Send register request
        const result = await wsClient.register(username, password, role);

        if (result.success) {
            showNotification('Đăng ký thành công! Vui lòng đăng nhập.', 'success');

            // Switch to login page
            document.getElementById('page-register').classList.remove('active');
            document.getElementById('page-login').classList.add('active');

            // Pre-fill username
            document.getElementById('login-username').value = username;
        } else {
            showNotification(result.message || 'Đăng ký thất bại', 'error');
        }
    } catch (error) {
        console.error('Register error:', error);
        // Show specific error if available
        const msg = error.message || error.code || 'Lỗi kết nối server';
        showNotification('Lỗi đăng ký: ' + msg, 'error');
    }
}

/**
 * Handle logout
 */
async function handleLogout() {
    try {
        if (wsClient) {
            await wsClient.logout();
        }
    } catch (error) {
        console.error('Logout error:', error);
    }

    // Clear localStorage
    localStorage.removeItem('sessionToken');
    localStorage.removeItem('username');
    localStorage.removeItem('role');

    currentUser = null;

    // Reload page to show login
    location.reload();
}

/**
 * Handle login success
 */
function handleLoginSuccess(data) {
    console.log('Login success:', data);
}

/**
 * Handle error
 */
function handleError(data) {
    showNotification(data.message || 'Có lỗi xảy ra', 'error');
}

// ==================== STATISTICS PAGE ====================

function loadStatistics() {
    if (!wsClient) return;

    wsClient.send(C2S_GET_STATS, {});
}

function handleStatsData(data) {
    // Update stats cards
    document.getElementById('total-tests').textContent = data.total_tests || 0;

    const avgScore = data.average_score || 0;
    document.getElementById('avg-score').textContent = avgScore.toFixed(1) + '%';

    const bestScore = data.best_score || 0;
    document.getElementById('best-score').textContent = bestScore.toFixed(1) + '%';

    const totalTime = data.total_time || 0;
    document.getElementById('total-time').textContent = Math.round(totalTime / 60);

    // Render chart
    renderStatsChart(data);
}

function renderStatsChart(data) {
    const ctx = document.getElementById('stats-chart');
    if (!ctx) return;

    const historyData = data.history || [];

    // Destroy existing chart
    if (window.statsChart) {
        window.statsChart.destroy();
    }

    // Create new chart
    window.statsChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: historyData.map((_, i) => `Lần ${i + 1}`),
            datasets: [{
                label: 'Điểm số (%)',
                data: historyData.map(h => h.score),
                borderColor: '#4F46E5',
                backgroundColor: 'rgba(79, 70, 229, 0.1)',
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            plugins: {
                legend: {
                    display: true
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    max: 100
                }
            }
        }
    });
}

// ==================== PRACTICE PAGE ====================

function resetPracticeSection() {
    document.getElementById('practice-setup').style.display = 'block';
    document.getElementById('practice-exam').style.display = 'none';
    document.getElementById('practice-result').style.display = 'none';
}

// Setup practice form
document.addEventListener('DOMContentLoaded', () => {
    const practiceForm = document.getElementById('practice-setup-form');
    if (practiceForm) {
        practiceForm.addEventListener('submit', (e) => {
            e.preventDefault();
            startPractice();
        });
    }

    const submitPracticeBtn = document.getElementById('btn-submit-practice');
    if (submitPracticeBtn) {
        submitPracticeBtn.addEventListener('click', submitPractice);
    }

    const practiceAgainBtn = document.getElementById('btn-practice-again');
    if (practiceAgainBtn) {
        practiceAgainBtn.addEventListener('click', resetPracticeSection);
    }
});

function startPractice() {
    const subject = document.getElementById('practice-subject').value;
    const difficulty = document.getElementById('practice-difficulty').value;
    const count = parseInt(document.getElementById('practice-count').value);

    if (!wsClient) {
        showNotification('Chưa kết nối tới server', 'error');
        return;
    }

    wsClient.send(C2S_PRACTICE_REQUEST, { subject, difficulty, question_count: count });
}

function handlePracticeQuestions(data) {
    const questions = data.questions || [];

    if (questions.length === 0) {
        showNotification('Không tìm thấy câu hỏi phù hợp', 'error');
        return;
    }

    // Hide setup, show exam
    document.getElementById('practice-setup').style.display = 'none';
    document.getElementById('practice-exam').style.display = 'block';

    // Render questions
    const container = document.getElementById('practice-questions-container');
    container.innerHTML = questions.map((q, index) => `
        <div class="exam-question">
            <div class="exam-question-text">
                <strong>Câu ${index + 1}:</strong> ${q.question_text}
            </div>
            <div class="exam-options">
                <label class="exam-option">
                    <input type="radio" name="question-${q.question_id}" value="A">
                    A. ${q.option_a}
                </label>
                <label class="exam-option">
                    <input type="radio" name="question-${q.question_id}" value="B">
                    B. ${q.option_b}
                </label>
                <label class="exam-option">
                    <input type="radio" name="question-${q.question_id}" value="C">
                    C. ${q.option_c}
                </label>
                <label class="exam-option">
                    <input type="radio" name="question-${q.question_id}" value="D">
                    D. ${q.option_d}
                </label>
            </div>
        </div>
    `).join('');

    // Start timer
    let seconds = 0;
    practiceTimer = setInterval(() => {
        seconds++;
        const mins = Math.floor(seconds / 60);
        const secs = seconds % 60;
        document.getElementById('practice-timer').textContent =
            `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    }, 1000);
}

function submitPractice() {
    if (practiceTimer) {
        clearInterval(practiceTimer);
    }

    // Collect answers
    const answers = {};
    document.querySelectorAll('#practice-questions-container .exam-question').forEach(questionEl => {
        const radio = questionEl.querySelector('input[type="radio"]:checked');
        if (radio) {
            const questionId = radio.name.replace('question-', '');
            answers[questionId] = radio.value;
        }
    });

    if (Object.keys(answers).length === 0) {
        showNotification('Bạn chưa trả lời câu nào', 'error');
        return;
    }

    wsClient.send(C2S_PRACTICE_SUBMIT, { answers });
}

function handlePracticeResult(data) {
    document.getElementById('practice-exam').style.display = 'none';
    document.getElementById('practice-result').style.display = 'block';

    const score = data.score || 0;
    const correct = data.correct_count || 0;
    const total = data.total_questions || 0;
    const time = document.getElementById('practice-timer').textContent;

    document.getElementById('practice-score').textContent = score.toFixed(1) + '%';
    document.getElementById('practice-correct').textContent = `${correct}/${total}`;
    document.getElementById('practice-time').textContent = time;

    showNotification('Đã hoàn thành bài luyện tập!', 'success');
}

// ==================== TEST ROOMS PAGE ====================

function loadRoomList() {
    if (!wsClient) return;
    wsClient.send(C2S_LIST_ROOMS, {});
}

function handleRoomList(data) {
    const rooms = data.rooms || [];
    const container = document.getElementById('rooms-list');

    if (rooms.length === 0) {
        container.innerHTML = '<p style="text-align: center; padding: 40px;">Chưa có phòng thi nào</p>';
        return;
    }

    container.innerHTML = rooms.map(room => `
        <div class="room-card" onclick="joinRoom(${room.room_id})">
            <div class="room-name">${room.room_name}</div>
            <div class="room-info">
                <div>📚 Môn: ${room.subject}</div>
                <div>📝 Số câu: ${room.question_count}</div>
                <div>⏱️ Thời gian: ${room.duration} phút</div>
                <div>👥 Người tham gia: ${room.participant_count || 0}</div>
            </div>
            <span class="room-status ${room.status.toLowerCase()}">${getStatusText(room.status)}</span>
        </div>
    `).join('');
}

function getStatusText(status) {
    const map = {
        'NOT_STARTED': 'Đang chờ',
        'ONGOING': 'Đang thi',
        'FINISHED': 'Đã kết thúc'
    };
    return map[status] || status;
}

function joinRoom(roomId) {
    if (!wsClient) return;
    wsClient.send(C2S_JOIN_ROOM, { room_id: roomId });
}

function handleRoomCreated(data) {
    showNotification('Đã tạo phòng thi thành công!', 'success');
    loadRoomList();
}

function handleJoinOk(data) {
    showNotification('Đã tham gia phòng thi', 'success');
}

function handleUserJoinedRoom(data) {
    console.log('User joined room:', data);
}

function handleTestStarted(data) {
    showNotification('Bài thi đã bắt đầu!', 'info');
}

// ==================== HISTORY PAGE ====================

function loadHistory() {
    if (!wsClient) return;
    wsClient.send(C2S_GET_HISTORY, {});
}

function handleHistoryData(data) {
    const history = data.history || [];
    const tbody = document.getElementById('history-tbody');

    if (history.length === 0) {
        tbody.innerHTML = '<tr><td colspan="7" style="text-align: center;">Chưa có lịch sử</td></tr>';
        return;
    }

    tbody.innerHTML = history.map(h => `
        <tr>
            <td>${new Date(h.created_at).toLocaleString('vi-VN')}</td>
            <td>${h.subject}</td>
            <td>${h.type === 'PRACTICE' ? 'Luyện tập' : 'Thi'}</td>
            <td>${h.total_questions}</td>
            <td>${h.correct_answers}</td>
            <td><strong>${h.score.toFixed(1)}%</strong></td>
            <td>${Math.round(h.time_spent / 60)} phút</td>
        </tr>
    `).join('');
}

// Initialize app when DOM is ready
document.addEventListener('DOMContentLoaded', initializeApp);
