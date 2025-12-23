/**
 * Authentication Module
 */

class AuthModule {
    static async login(username, password) {
        try {
            const response = await ws.login(username, password);
            console.log('✓ Login successful');
            showNotification('Đăng nhập thành công!', 'success');
            return response;
        } catch (error) {
            console.error('✗ Login failed:', error);
            if (error.code === ERR_LOGIN_FAILED) {
                showNotification('Tên đăng nhập hoặc mật khẩu không đúng', 'error');
            } else {
                showNotification('Lỗi đăng nhập: ' + (error.message || error.code), 'error');
            }
            throw error;
        }
    }
    
    static async register(username, password, confirmPassword, role = ROLE_USER) {
        // Validate
        if (password !== confirmPassword) {
            showNotification('Mật khẩu không khớp', 'error');
            throw new Error('Passwords do not match');
        }
        
        if (password.length < 6) {
            showNotification('Mật khẩu phải có ít nhất 6 ký tự', 'error');
            throw new Error('Password too short');
        }
        
        try {
            const response = await ws.register(username, password, role);
            console.log('✓ Registration successful');
            showNotification('Đăng ký thành công! Hãy đăng nhập.', 'success');
            return response;
        } catch (error) {
            console.error('✗ Registration failed:', error);
            if (error.code === ERR_USERNAME_EXISTS) {
                showNotification('Tên tài khoản đã tồn tại', 'error');
            } else {
                showNotification('Lỗi đăng ký: ' + (error.message || error.code), 'error');
            }
            throw error;
        }
    }
    
    static async logout() {
        try {
            await ws.logout();
            console.log('✓ Logout successful');
            showNotification('Đã đăng xuất', 'success');
            navigateTo('login');
        } catch (error) {
            console.error('✗ Logout failed:', error);
            showNotification('Lỗi đăng xuất', 'error');
        }
    }
    
    static isLoggedIn() {
        return ws && ws.isLoggedIn();
    }
    
    static getSessionToken() {
        return ws ? ws.sessionToken : null;
    }
    
    static getUsername() {
        return ws ? ws.username : null;
    }
    
    static getRole() {
        return ws ? ws.role : null;
    }
    
    static getUserId() {
        return ws ? ws.userId : null;
    }
    
    static isTeacher() {
        return this.getRole() === ROLE_TEACHER;
    }
}
