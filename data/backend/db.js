
const { Pool } = require('pg');
require('dotenv').config();  // Đọc các biến môi trường từ .env

// Tạo pool kết nối với PostgreSQL
const pool = new Pool({
  connectionString: process.env.DATABASE_URL,  // Đọc từ biến môi trường
});

// Kiểm tra kết nối tới cơ sở dữ liệu
pool.connect()
  .then(() => {
    console.log('Kết nối tới cơ sở dữ liệu thành công!');
  })
  .catch(err => {
    console.error('Lỗi khi kết nối tới cơ sở dữ liệu:', err.message);
  });

module.exports = pool;
