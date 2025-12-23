# Hướng Dẫn Redesign Giao Diện Web

## Tổng Quan Thay Đổi

Đã redesign giao diện từ Single Page Application đơn giản sang **Dashboard với Sidebar Navigation**.

## Files Đã Thay Đổi

### 1. HTML (index.html)
- ✅ **Đã tạo mới**: Layout với sidebar bên trái, content area bên phải
- ✅ **Auth pages**: Login/Register tách riêng
- ✅ **Sidebar**: Navigation với icons, user info
- ✅ **Main content**: Multiple pages (Statistics, Questions, Practice, Test Rooms, History)
- ✅ **Modals**: Question form, Create room form

### 2. CSS (css/main.css)  
- ✅ **Đã tạo mới**: Modern design với sidebar layout
- ✅ **Color scheme**: Professional color variables
- ✅ **Responsive**: Mobile-friendly (sidebar collapse)
- ✅ **Components**: Cards, badges, modals, tables
- ✅ **Animations**: Smooth transitions

### 3. JavaScript Protocol (js/protocol.js)
- ✅ **Đã update**: Thêm message types cho quản lý câu hỏi
  - `C2S_LIST_QUESTIONS = 601`
  - `C2S_CREATE_QUESTION = 602`
  - `C2S_UPDATE_QUESTION = 603`
  - `C2S_DELETE_QUESTION = 604`
  - `S2C_QUESTIONS_LIST = 1301`
  - `S2C_QUESTION_CREATED = 1302`
  - `S2C_QUESTION_UPDATED = 1303`
  - `S2C_QUESTION_DELETED = 1304`

### 4. Questions Manager (js/questions.js)
- ✅ **Đã tạo mới**: Module quản lý câu hỏi cho giáo viên
- **Chức năng**:
  - Hiển thị danh sách câu hỏi
  - Tìm kiếm và lọc (theo môn, độ khó)
  - Thêm câu hỏi mới
  - Sửa câu hỏi
  - Xóa câu hỏi
  - Form validation

### 5. Main App Logic (js/main.js)
- ⚠️ **CẦN UPDATE**: Cập nhật để hỗ trợ sidebar navigation

## Các Thay Đổi Cần Thực Hiện Trong main.js

### A. Navigation System

```javascript
// Thay thế navigateTo() function
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
    document.getElementById('page-title').textContent = titles[pageName] || pageName;
    
    // Load page data
    switch(pageName) {
        case 'statistics':
            loadStatistics();
            break;
        case 'questions':
            if (questionsManager) {
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
```

### B. Auth Flow Update

```javascript
// Sau khi login thành công
function onLoginSuccess(data) {
    // Hide auth container
    document.getElementById('auth-container').style.display = 'none';
    
    // Show app container
    document.getElementById('app-container').style.display = 'flex';
    
    // Update sidebar user info
    document.getElementById('sidebar-username').textContent = data.username;
    document.getElementById('sidebar-role').textContent = 
        data.role === 'TEACHER' ? 'Giáo viên' : 'Học viên';
    
    // Show appropriate menu
    if (data.role === 'TEACHER') {
        document.getElementById('student-menu').style.display = 'none';
        document.getElementById('teacher-menu').style.display = 'block';
        
        // Initialize questions manager
        questionsManager = new QuestionsManager(wsClient);
    } else {
        document.getElementById('student-menu').style.display = 'block';
        document.getElementById('teacher-menu').style.display = 'none';
    }
    
    // Navigate to statistics (default page)
    navigateTo('statistics');
}
```

### C. Sidebar Click Handlers

```javascript
// Thêm vào DOMContentLoaded
document.addEventListener('DOMContentLoaded', () => {
    // Sidebar navigation
    document.querySelectorAll('.nav-item').forEach(item => {
        item.addEventListener('click', (e) => {
            e.preventDefault();
            const page = item.getAttribute('data-page');
            navigateTo(page);
        });
    });
    
    // Logout button
    document.getElementById('btn-logout')?.addEventListener('click', async () => {
        await AuthModule.logout();
        location.reload();
    });
    
    // Auth page switching
    document.getElementById('show-register')?.addEventListener('click', (e) => {
        e.preventDefault();
        document.getElementById('page-login').classList.remove('active');
        document.getElementById('page-register').classList.add('active');
    });
    
    document.getElementById('show-login')?.addEventListener('click', (e) => {
        e.preventDefault();
        document.getElementById('page-register').classList.remove('active');
        document.getElementById('page-login').classList.add('active');
    });
    
    // Auth forms
    document.getElementById('login-form')?.addEventListener('submit', handleLogin);
    document.getElementById('register-form')?.addEventListener('submit', handleRegister);
});
```

## Backend Changes Needed

### Server Protocol Update (server/include/protocol.h)

```cpp
// Add to protocol.h
#define C2S_LIST_QUESTIONS    601
#define C2S_CREATE_QUESTION   602
#define C2S_UPDATE_QUESTION   603
#define C2S_DELETE_QUESTION   604

#define S2C_QUESTIONS_LIST    1301
#define S2C_QUESTION_CREATED  1302
#define S2C_QUESTION_UPDATED  1303
#define S2C_QUESTION_DELETED  1304
```

### Server Protocol Implementation (server/src/protocol.cpp)

Cần thêm handlers cho:

1. **LIST_QUESTIONS**: Lấy danh sách câu hỏi (có filter)
```cpp
void Protocol::handle_list_questions(int client_socket, const json& payload) {
    // Check if user is TEACHER
    // Query Questions table with filters
    // Return S2C_QUESTIONS_LIST with questions array
}
```

2. **CREATE_QUESTION**: Tạo câu hỏi mới (TEACHER only)
```cpp
void Protocol::handle_create_question(int client_socket, const json& payload) {
    // Check permission (TEACHER only)
    // Validate data
    // INSERT INTO Questions
    // Return S2C_QUESTION_CREATED
}
```

3. **UPDATE_QUESTION**: Sửa câu hỏi (TEACHER only)
```cpp
void Protocol::handle_update_question(int client_socket, const json& payload) {
    // Check permission
    // UPDATE Questions WHERE question_id = ?
    // Return S2C_QUESTION_UPDATED
}
```

4. **DELETE_QUESTION**: Xóa câu hỏi (TEACHER only)
```cpp
void Protocol::handle_delete_question(int client_socket, const json& payload) {
    // Check permission
    // DELETE FROM Questions WHERE question_id = ?
    // Return S2C_QUESTION_DELETED
}
```

## Testing Checklist

- [ ] Login hiển thị giao diện mới đúng
- [ ] Sidebar navigation hoạt động
- [ ] User info hiển thị đúng
- [ ] Teacher menu vs Student menu hiển thị đúng role
- [ ] Trang Statistics load và hiển thị chart
- [ ] Trang Questions (TEACHER):
  - [ ] Hiển thị danh sách câu hỏi
  - [ ] Tìm kiếm hoạt động
  - [ ] Filter theo môn/độ khó
  - [ ] Thêm câu hỏi mới
  - [ ] Sửa câu hỏi
  - [ ] Xóa câu hỏi
- [ ] Trang Practice hoạt động như cũ
- [ ] Trang Test Rooms hoạt động như cũ
- [ ] Trang History hoạt động như cũ
- [ ] Logout hoạt động
- [ ] Responsive trên mobile

## Files Backup

Các file cũ đã được backup:
- `client/index-old.html` - HTML cũ
- `client/index.html.backup` - Backup trước khi redesign
- `client/css/main-old.css` - CSS cũ
- `client/css/main.css.backup` - Backup CSS
- `client/js/main.js.backup` - JavaScript cũ

## Rollback (Nếu cần)

```bash
cd /home/duong/Downloads/github/online-testing-multiplechoice/client
mv index.html index-new.html
mv index-old.html index.html
mv css/main.css css/main-new.css
mv css/main-old.css css/main.css
```

## Next Steps

1. **Update main.js**: Implement navigation và auth flow mới
2. **Implement backend handlers**: Thêm 4 handlers cho quản lý câu hỏi
3. **Test frontend**: Chạy và kiểm tra giao diện
4. **Test integration**: Test toàn bộ flow với backend

## Screenshots (Mô tả)

### Before (Old Design)
- Header top, navigation menu horizontal
- Pages stack vertically
- Simple card-based layout

### After (New Design)  
- **Sidebar bên trái**:
  - Logo + App name
  - User avatar + name + role
  - Navigation menu (vertical)
  - Logout button ở footer
- **Main content bên phải**:
  - Topbar với page title và status
  - Content area với stats cards, tables, forms
  - Modern card design với shadows
- **Auth pages**: Centered with gradient background

## Color Scheme

- Primary: #4F46E5 (Indigo)
- Secondary: #10B981 (Green)
- Danger: #EF4444 (Red)
- Warning: #F59E0B (Amber)
- Sidebar: #1F2937 (Dark gray)
- Background: #F9FAFB (Light gray)

## Icons

Sử dụng emoji icons:
- 📊 Statistics
- 📝 Questions
- ✏️ Practice
- 🏆 Test Rooms
- 📜 History
- 🚪 Logout

## Questions về Thiết Kế

> **Q: Tại sao chọn sidebar navigation?**
> A: Sidebar layout phổ biến cho dashboard apps, dễ navigation, professional hơn.

> **Q: Trang chủ cho học viên là gì?**
> A: Statistics (thống kê) - hiển thị overview về kết quả học tập.

> **Q: Giáo viên có thể làm gì?**
> A: Quản lý câu hỏi (CRUD), tạo phòng thi, xem thống kê chi tiết.

> **Q: Học viên có thể làm gì?**
> A: Luyện tập, tham gia thi, xem lịch sử, xem thống kê cá nhân.
