# WebSocket Gateway

Một WebSocket server để kết nối Web client với backend TCP server.

## Mô tả

Gateway này:
- Nghe WebSocket connections từ web browser trên port 8080
- Chuyển đổi giữa WebSocket JSON ↔ TCP [Type][Length][JSON] protocol
- Bridge mỗi WebSocket client tới TCP backend server (port 8888)

## Build

```bash
make clean && make
```

## Run

**Chạy với default settings (port 8080, backend 127.0.0.1:8888):**
```bash
make run
```

**Hoặc chạy với custom settings:**
```bash
./bin/gateway --port 8080 --backend-host 127.0.0.1 --backend-port 8888
```

## Protocol Conversion

### WebSocket → TCP
- Nhận JSON từ WebSocket client:
  ```json
  {
    "type": 102,  // C2S_LOGIN
    "username": "user_a",
    "password": "pass123"
  }
  ```
- Convert sang TCP format: `[Type=102][Length=...][JSON]`
- Gửi qua TCP tới backend

### TCP → WebSocket  
- Nhận từ backend: `[Type=803][Length=...][JSON]`
- Convert sang JSON và thêm type field:
  ```json
  {
    "type": 803,  // S2C_LOGIN_OK
    "session_token": "...",
    "username": "user_a",
    "role": "USER"
  }
  ```
- Gửi qua WebSocket frame tới client

## Dependencies

- OpenSSL (for SHA1): `apt-get install libssl-dev`
- nlohmann/json: Already included in server/include/
- Standard C++17

## Usage

1. **Start backend server** (if not running):
   ```bash
   cd ../server
   make run
   ```

2. **Start WebSocket gateway** (in another terminal):
   ```bash
   cd ../gateway
   make run
   ```

3. **Web client connects** to `ws://localhost:8080`

## Troubleshooting

- **"Failed to connect to backend"**: Make sure TCP server is running on 127.0.0.1:8888
- **"Invalid WebSocket upgrade request"**: Browser not sending proper WebSocket upgrade headers
- **Build errors**: Make sure libssl-dev is installed: `apt-get install libssl-dev`
