const express = require('express');
const path = require('path');
const cors = require('cors');
require('dotenv').config();

const authRoutes = require('./routes/authRoutes');
const app = express();
const port = process.env.PORT || 3000;

// Import database connection
require('./db');

// Middleware để parse JSON
app.use(express.json());

// Cấu hình CORS
const corsOptions = {
  origin: process.env.CORS_ORIGIN || '*',
  credentials: true,
  optionsSuccessStatus: 200
};
app.use(cors(corsOptions));

// Health check endpoint cho Render
app.get('/health', (req, res) => {
  res.status(200).json({ 
    status: 'OK', 
    timestamp: new Date().toISOString(),
    uptime: process.uptime()
  });
});

// Serve static files từ frontend
app.use(express.static(path.join(__dirname, '../frontend')));

// API routes
app.use('/auth', authRoutes);

// Route cho root - serve frontend
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, '../frontend/index.html'));
});

// Catch-all handler: serve frontend cho tất cả routes không tìm thấy
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, '../frontend/index.html'));
});

// Error handling middleware
app.use((err, req, res, next) => {
  console.error('Error:', err);
  res.status(500).json({ message: 'Internal server error' });
});

// Khởi động server
app.listen(port, '0.0.0.0', () => {
  console.log(`🚀 Server running on port ${port}`);
  console.log(`📱 Frontend: http://localhost:${port}`);
  console.log(`🔗 API: http://localhost:${port}/auth`);
  console.log(`💚 Health: http://localhost:${port}/health`);
});
