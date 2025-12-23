# Web Client - Online Testing System

Một ứng dụng web hiện đại cho hệ thống thi trắc nghiệm trực tuyến.

## Tính Năng

- ✅ Đăng ký / Đăng nhập
- ✅ Luyện tập với câu hỏi tùy chỉnh
- ✅ Tham gia / Tạo phòng thi (TEACHER)
- ✅ Làm bài thi với bộ đếm thời gian
- ✅ Lịch sử và thống kê
- ✅ Real-time updates (push notifications)

## Yêu Cầu

- WebSocket Gateway chạy trên port 8080 (xem [gateway/README.md](../gateway/README.md))
- Backend TCP Server chạy trên port 8888 (xem [server/README.md](../server/README.md))
- Trình duyệt hiện đại (Chrome, Firefox, Safari, Edge)

## Cách Sử Dụng

### 1. Khởi Động Backend Server

```bash
cd server
make run
```

### 2. Khởi Động WebSocket Gateway

```bash
cd gateway
make run
```

### 3. Mở Web Client

**Option A: Serve từ Python**
```bash
cd client
python3 -m http.server 3000
```

Sau đó mở browser: `http://localhost:3000`

**Option B: Serve từ Node.js**
```bash
cd client
npx http-server -p 3000
```

**Option C: Mở file trực tiếp**
```bash
# Đơn giản nhất, mở index.html trực tiếp trong browser
# File -> Open -> client/index.html
```

## Kiến Trúc

```
client/
├── index.html              # Single Page Application
├── css/
│   └── main.css           # Main styles
├── js/
│   ├── protocol.js        # Protocol constants
│   ├── websocket-client.js # WebSocket handler
│   ├── auth.js            # Authentication logic
│   └── main.js            # Main app logic
└── lib/
    └── Chart.js           # For statistics charts
```

## Luồng Giao Tiếp

```
Browser (Web Client)
    ↓ WebSocket Connection ↓
Gateway (port 8080)
    ↓ Convert JSON ↔ [Type][Length][JSON] ↓
Backend Server (port 8888)
    ↓ TCP Socket ↓
Database (SQLite)
```

## Các Trang

### Login & Register
- Đăng nhập: nhập tên tài khoản và mật khẩu
- Đăng ký: tạo tài khoản mới, chọn vai trò (USER/TEACHER)

### Dashboard
- Menu chính với các tùy chọn
- Hiển thị tên người dùng và vai trò

### Practice Mode
- Chọn số câu hỏi, chủ đề, độ khó
- Làm bài với bộ đếm thời gian
- Xem kết quả

### Test Mode
- Xem danh sách phòng thi
- Tham gia hoặc tạo phòng mới (TEACHER)
- Chờ trong phòng chờ
- Làm bài thi
- Xem kết quả và bảng điểm

### History
- Xem danh sách tất cả bài đã làm
- Thông tin: ngày, loại, điểm

### Statistics
- Biểu đồ điểm theo thời gian
- Biểu đồ phân bố chủ đề

## Lưu Ý

- Session token được lưu trong `localStorage`
- Tự động kết nối lại nếu mất mạng (5 lần retry)
- Timeout cho các request: 5-10 giây
- Responsive design, hoạt động tốt trên mobile

## Troubleshooting

### Không kết nối được đến server
- Kiểm tra gateway chạy trên port 8080
- Kiểm tra backend server chạy trên port 8888
- Check browser console (F12) xem error message

### Timeout
- Đảm bảo network latency thấp (< 100ms)
- Kiểm tra firewall không chặn WebSocket

### CORS/Security Issues
- Mở file HTML từ `http://localhost`, không từ `file://`
- Hoặc serve từ Python/Node.js HTTP server

## Development

### Build assets
Chỉ cần edit HTML/CSS/JS files, không cần build process.

### Debug
- Mở Developer Console: F12 hoặc Ctrl+Shift+I
- Xem WebSocket messages trong tab Network
- Check console logs

## Browser Compatibility

- Chrome/Chromium: ✅ Full support
- Firefox: ✅ Full support
- Safari: ✅ Full support
- Edge: ✅ Full support
- IE 11: ❌ Not supported (use modern browser)

## Liên Quan

- [Backend Server](../server/README.md)
- [WebSocket Gateway](../gateway/README.md)
- [Protocol Specification](../docs/application_design.md)
