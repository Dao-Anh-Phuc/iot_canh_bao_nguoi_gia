window.onload = async () => {
  const token = localStorage.getItem('token');
  const loginSection = document.getElementById('login-section');
  const registerSection = document.getElementById('register-section');
  const profileSection = document.getElementById('profile-section');
  const deviceIdSection = document.getElementById('device-id-section');
  const phoneSection = document.getElementById('phone-section');

  // API Base URL - sẽ thay bằng Render URL sau khi deploy
  const API_BASE = window.location.hostname === 'localhost' 
    ? 'http://localhost:3000' 
    : 'https://iot-backend-346j.onrender.com'; // Thay bằng Render URL

  // Chuyển đổi giữa login và register
  document.getElementById('show-register').onclick = () => {
    loginSection.style.display = 'none';
    registerSection.style.display = 'block';
  };
  document.getElementById('show-login').onclick = () => {
    registerSection.style.display = 'none';
    loginSection.style.display = 'block';
  };

  // Xử lý đăng ký
  document.getElementById('register-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const email = document.getElementById('reg-email').value;
    const password = document.getElementById('reg-password').value;
    const messageEl = document.getElementById('register-message');
    
    try {
      const response = await fetch(`${API_BASE}/auth/register`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ email, password })
      });
      const data = await response.json();
      
      if (response.ok && data.id) {
        messageEl.innerText = 'Register successful! Please login.';
        messageEl.className = 'success';
        // Clear form
        document.getElementById('register-form').reset();
        // Auto switch to login after 2 seconds
        setTimeout(() => {
          document.getElementById('show-login').click();
        }, 2000);
      } else {
        messageEl.innerText = data.message || 'Register failed';
        messageEl.className = 'error';
      }
    } catch (error) {
      console.error('Register error:', error);
      messageEl.innerText = 'Network error. Please try again.';
      messageEl.className = 'error';
    }
  });

  // Nếu đã đăng nhập, hiển thị profile và form nhập ID thiết bị
  if (token) {
    loginSection.style.display = 'none';
    registerSection.style.display = 'none';
    profileSection.style.display = 'block';
    deviceIdSection.style.display = 'block';
    phoneSection.style.display = 'none';

    // Lấy thông tin user
  const profileRes = await fetch(`${API_BASE}/auth/profile`, {
      method: 'GET',
      headers: { 'Authorization': `Bearer ${token}` }
    });
    const profileData = await profileRes.json();
    if (profileData.user) {
      document.getElementById('welcome-message').innerText =
        `Welcome, ${profileData.user.email}!`;
    }

    // Xử lý nhập ID thiết bị
    document.getElementById('device-id-form').addEventListener('submit', async (e) => {
      e.preventDefault();
      const deviceId = document.getElementById('input-device-id').value;
  const deviceRes = await fetch(`${API_BASE}/auth/devices/${deviceId}`, {
        method: 'GET',
        headers: { 'Authorization': `Bearer ${token}` }
      });
      const deviceData = await deviceRes.json();
      if (deviceData.device) {
        document.getElementById('device-id-message').innerText =
          `Device found: ${deviceData.device.device_id}`;
        phoneSection.style.display = 'block';
        // Nếu đã có số điện thoại, hiển thị luôn
        if (deviceData.device.phone_number) {
          document.getElementById('input-phone-number').value = deviceData.device.phone_number;
        } else {
          document.getElementById('input-phone-number').value = '';
        }
        window.currentDeviceId = deviceId;
      } else {
        document.getElementById('device-id-message').innerText = 'Device not found!';
        phoneSection.style.display = 'none';
      }
    });

    // Xử lý cập nhật số điện thoại cho thiết bị
    document.getElementById('phone-form').addEventListener('submit', async (e) => {
      e.preventDefault();
      const phone_number = document.getElementById('input-phone-number').value;
      const deviceId = window.currentDeviceId;
      // Cập nhật số điện thoại lên backend
      const res = await fetch(`${API_BASE}/auth/devices/${deviceId}/phone`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${token}`
        },
        body: JSON.stringify({ phone_number })
      });
      const result = await res.json();
      if (result.device) {
        document.getElementById('phone-message').innerText = 'Phone number updated!';
        // Gửi lệnh tới ESP32 để nó nhận số điện thoại mới
        await fetch(`${API_BASE}/auth/devices/${deviceId}/command`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ command: 'call_user' })
        });
      } else {
        document.getElementById('phone-message').innerText = result.message || 'Error updating phone number';
      }
    });

    // Logout
    document.getElementById('logout-button').addEventListener('click', () => {
      localStorage.removeItem('token');
      window.location.reload();
    });

  } else {
    // Nếu chưa đăng nhập, hiển thị form login
    loginSection.style.display = 'block';
    registerSection.style.display = 'none';
    profileSection.style.display = 'none';

    document.getElementById('login-form').addEventListener('submit', async (e) => {
      e.preventDefault();
      const email = document.getElementById('email').value;
      const password = document.getElementById('password').value;
      const response = await fetch(`${API_BASE}/auth/login`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ email, password })
      });
      const data = await response.json();
      if (data.token) {
        localStorage.setItem('token', data.token);
        window.location.reload();
      } else {
        document.getElementById('login-message').innerText =
          data.message || 'Login failed';
      }
    });
  }
};