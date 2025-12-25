/**
 * WebSocket Client Handler
 * Manages connection to WebSocket gateway
 */

class WebSocketClient {
    constructor(url = 'ws://localhost:8080') {
        this.url = url;
        this.socket = null;
        this.sessionToken = localStorage.getItem('sessionToken') || null;
        this.userId = localStorage.getItem('userId') || null;
        this.username = localStorage.getItem('username') || null;
        this.role = localStorage.getItem('role') || null;

        // Message handlers map: { messageType: [callback1, callback2, ...] }
        this.handlers = new Map();

        // Connection state
        this.isConnected = false;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
        this.reconnectDelay = 1000;
    }

    /**
     * Connect to WebSocket server
     */
    connect() {
        return new Promise((resolve, reject) => {
            try {
                this.socket = new WebSocket(this.url);

                this.socket.onopen = () => {
                    console.log('✓ WebSocket connected');
                    this.isConnected = true;
                    this.reconnectAttempts = 0;
                    resolve();
                };

                this.socket.onmessage = (event) => {
                    this.handleMessage(event.data);
                };

                this.socket.onerror = (error) => {
                    console.error('✗ WebSocket error:', error);
                    reject(error);
                };

                this.socket.onclose = () => {
                    console.log('WebSocket disconnected');
                    this.isConnected = false;
                    this.attemptReconnect();
                };

                // Connection timeout
                setTimeout(() => {
                    if (!this.isConnected) {
                        reject(new Error('Connection timeout'));
                    }
                }, 5000);
            } catch (error) {
                reject(error);
            }
        });
    }

    /**
     * Attempt to reconnect to WebSocket
     */
    attemptReconnect() {
        if (this.reconnectAttempts >= this.maxReconnectAttempts) {
            console.error('Max reconnection attempts reached');
            this.notifyDisconnect();
            return;
        }

        this.reconnectAttempts++;
        const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1);
        console.log(`Reconnecting in ${delay}ms (attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts})`);

        setTimeout(() => {
            this.connect().catch(() => {
                this.attemptReconnect();
            });
        }, delay);
    }

    /**
     * Send message to server
     * @param {number} messageType - Message type code
     * @param {object} payload - Message payload
     */
    send(messageType, payload = {}) {
        if (!this.isConnected) {
            console.error('WebSocket not connected');
            return false;
        }

        try {
            // Add session token to all messages (except register/login)
            if (messageType !== 101 && messageType !== 102) {
                payload.session_token = this.sessionToken;
            }

            // Add type field
            payload.type = messageType;

            const json = JSON.stringify(payload);
            this.socket.send(json);

            console.log(`→ Sent message type ${messageType} (${json.length} bytes)`);
            return true;
        } catch (error) {
            console.error('Failed to send message:', error);
            return false;
        }
    }

    async ensureConnection() {
        if (this.isConnected) return;
        console.log('Restoring connection before action...');
        return this.connect();
    }

    /**
     * Handle incoming message
     * @param {string} data - JSON message data
     */
    handleMessage(data) {
        try {
            const message = JSON.parse(data);
            const messageType = message.type;

            console.log(`← Received message type ${messageType}`);

            // Call registered handlers
            if (this.handlers.has(messageType)) {
                const callbacks = this.handlers.get(messageType);
                for (const callback of callbacks) {
                    try {
                        callback(message);
                    } catch (error) {
                        console.error(`Error in handler for message type ${messageType}:`, error);
                    }
                }
            }
        } catch (error) {
            console.error('Failed to parse message:', error);
        }
    }

    async ensureConnection() {
        if (this.isConnected) return;
        console.log('Restoring connection before action...');
        return this.connect();
    }

    /**
     * Register message handler
     * @param {number} messageType - Message type code
     * @param {function} callback - Handler function
     */
    on(messageType, callback) {
        if (!this.handlers.has(messageType)) {
            this.handlers.set(messageType, []);
        }
        this.handlers.get(messageType).push(callback);
    }

    /**
     * Unregister message handler
     * @param {number} messageType - Message type code
     * @param {function} callback - Handler function
     */
    off(messageType, callback) {
        if (!this.handlers.has(messageType)) return;

        const callbacks = this.handlers.get(messageType);
        const index = callbacks.indexOf(callback);
        if (index > -1) {
            callbacks.splice(index, 1);
        }
    }

    /**
     * Register/Login/Logout
     */
    async register(username, password, role = 'USER') {
        await this.ensureConnection();

        return new Promise((resolve, reject) => {
            const handler = (msg) => {
                this.off(801, handler);
                if (msg.message) {
                    resolve({ success: true, ...msg });
                } else {
                    reject(msg);
                }
            };
            const errorHandler = (msg) => {
                this.off(802, errorHandler);
                reject(msg);
            };

            this.on(801, handler);
            this.on(802, errorHandler);

            this.send(101, { username, password, role });

            // Timeout
            setTimeout(() => {
                this.off(801, handler);
                this.off(802, errorHandler);
                reject(new Error('Register timeout'));
            }, 5000);
        });
    }

    async login(username, password) {
        await this.ensureConnection();

        return new Promise((resolve, reject) => {
            const handler = (msg) => {
                this.off(803, handler);
                // Store session info
                this.sessionToken = msg.session_token;
                this.userId = msg.user_id;
                this.username = msg.username;
                this.role = msg.role;

                localStorage.setItem('sessionToken', this.sessionToken);
                localStorage.setItem('userId', this.userId);
                localStorage.setItem('username', this.username);
                localStorage.setItem('role', this.role);

                resolve({ success: true, ...msg });
            };
            const errorHandler = (msg) => {
                this.off(802, errorHandler);
                reject(msg);
            };

            this.on(803, handler);
            this.on(802, errorHandler);

            this.send(102, { username, password });

            // Timeout
            setTimeout(() => {
                this.off(803, handler);
                this.off(802, errorHandler);
                reject(new Error('Login timeout'));
            }, 5000);
        });
    }

    logout() {
        return new Promise((resolve, reject) => {
            const handler = (msg) => {
                this.off(801, handler);
                // Clear session info
                this.sessionToken = null;
                this.userId = null;
                this.username = null;
                this.role = null;

                localStorage.removeItem('sessionToken');
                localStorage.removeItem('userId');
                localStorage.removeItem('username');
                localStorage.removeItem('role');

                resolve(msg);
            };

            this.on(801, handler);
            this.send(103, {});

            // Timeout
            setTimeout(() => {
                this.off(801, handler);
                reject(new Error('Logout timeout'));
            }, 5000);
        });
    }

    /**
     * Notify UI that connection is lost
     */
    notifyDisconnect() {
        // Dispatch custom event
        window.dispatchEvent(new CustomEvent('ws-disconnected'));
    }

    /**
     * Check if logged in
     */
    isLoggedIn() {
        return this.sessionToken !== null && this.isConnected;
    }

    /**
     * Disconnect
     */
    disconnect() {
        if (this.socket) {
            this.socket.close();
            this.socket = null;
            this.isConnected = false;
        }
    }
}

// Global WebSocket client instance
let ws = null;

/**
 * Initialize WebSocket client
 */
async function initWebSocket(gatewayUrl = 'ws://localhost:8080') {
    if (ws && ws.isConnected) {
        return ws;
    }

    ws = new WebSocketClient(gatewayUrl);

    try {
        await ws.connect();
        console.log('✓ WebSocket client initialized');
        return ws;
    } catch (error) {
        console.error('✗ Failed to initialize WebSocket:', error);
        throw error;
    }
}
