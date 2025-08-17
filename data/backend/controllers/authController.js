const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const pool = require('../db');

// Đăng ký người dùng mới
const register = async (req, res) => {
  try {
    const { email, password } = req.body;

    // Validate input
    if (!email || !password) {
      return res.status(400).json({ message: 'Email and password are required' });
    }

    if (password.length < 6) {
      return res.status(400).json({ message: 'Password must be at least 6 characters' });
    }

    // Kiểm tra nếu email đã tồn tại
    const checkEmail = await pool.query('SELECT * FROM users WHERE email = $1', [email]);

    if (checkEmail.rowCount > 0) {
      return res.status(400).json({ message: 'Email already exists' });
    }

    // Mã hóa mật khẩu
    const hashedPassword = await bcrypt.hash(password, 10);

    const result = await pool.query(
      'INSERT INTO users (email, password_hash) VALUES ($1, $2) RETURNING id, email',
      [email, hashedPassword]
    );
    res.status(201).json(result.rows[0]);  // Trả về thông tin người dùng đã đăng ký
  } catch (err) {
    console.error('Register error:', err);
    res.status(500).json({ message: 'Error creating user' });
  }
};

// Đăng nhập người dùng và trả về JWT token
const login = async (req, res) => {
  const { email, password } = req.body;

  try {
    // Kiểm tra email trong cơ sở dữ liệu
    const result = await pool.query('SELECT * FROM users WHERE email = $1', [email]);
    const user = result.rows[0];

    if (!user || !(await bcrypt.compare(password, user.password_hash))) {
      return res.status(400).json({ message: 'Invalid credentials' });
    }

    // Tạo token JWT
    const token = jwt.sign({ userId: user.id }, process.env.JWT_SECRET, { expiresIn: '7d' });

    // Trả về token
    res.json({ token });
  } catch (err) {
    console.error(err);
    res.status(500).json({ message: 'Error logging in user' });
  }
};

module.exports = {
  register,
  login,
};
