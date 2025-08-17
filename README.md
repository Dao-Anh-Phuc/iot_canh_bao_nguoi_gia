# IoT Emergency Alert System

## 🚀 Deploy lên Render (FREE)

### Bước 1: Chuẩn bị code
```bash
git init
git add .
git commit -m "Initial commit"
git branch -M main
```

### Bước 2: Tạo GitHub Repository
1. Tạo repo mới trên GitHub
2. Push code lên:
```bash
git remote add origin https://github.com/yourusername/iot-emergency-alert.git
git push -u origin main
```

### Bước 3: Deploy Database trên Render
1. Đăng ký tại [render.com](https://render.com)
2. Tạo **PostgreSQL Database**:
   - Name: `iot-emergency-db`
   - Database: `iot_emergency`
   - User: `iot_user`
   - Region: `Oregon (US West)`
3. Copy **External Database URL**
4. Chạy script `database_init.sql` trong **psql Console**

### Bước 4: Deploy Web Service
1. Tạo **Web Service** mới
2. Connect GitHub repository
3. Settings:
   - **Name**: `iot-emergency-api`
   - **Root Directory**: `/` (root)
   - **Environment**: `Node`
   - **Build Command**: `npm install`
   - **Start Command**: `npm start`
4. Environment Variables:
   ```
   DATABASE_URL=<paste_database_url_here>
   JWT_SECRET=your_super_secure_secret_key
   NODE_ENV=production
   PORT=10000
   ```

### Bước 5: Update ESP32 Code
Sau khi deploy, update API URL trong ESP32:
```cpp
const char* API_BASE_URL = "https://your-service-name.onrender.com";
```

## 📱 Testing
- Frontend: `https://your-service-name.onrender.com`
- API Health: `https://your-service-name.onrender.com/health`
- Device API: `https://your-service-name.onrender.com/auth/devices/ESP32_001/phone`

## 🔧 Local Development
```bash
npm install
cd data/backend
npm run dev
```

## 📋 API Endpoints
- `POST /auth/register` - Đăng ký user
- `POST /auth/login` - Đăng nhập  
- `GET /auth/devices/:deviceId/phone` - Lấy số phone (cho ESP32)
- `PUT /auth/devices/:deviceId/phone` - Cập nhật số phone
- `POST /auth/alerts` - Log alert từ ESP32
