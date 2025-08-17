const express = require('express');
const { register, login } = require('../controllers/authController');
const authMiddleware = require('../middleware/authMiddleware');
const pool = require('../db');

const router = express.Router();

router.post('/register', register);
router.post('/login', login);

// API lấy thông tin profile người dùng
router.get('/profile', authMiddleware, async (req, res) => {
  try {
    const result = await pool.query('SELECT id, email FROM users WHERE id = $1', [req.userId]);
    if (result.rows.length === 0) return res.status(404).json({ message: 'User not found' });
    res.json({ user: result.rows[0] });
  } catch (err) {
    res.status(500).json({ message: 'Error fetching profile' });
  }
});

// Thêm thiết bị mới
router.post('/devices', authMiddleware, async (req, res) => {
  const { device_id, phone_number } = req.body;
  try {
    // Kiểm tra thiết bị đã tồn tại chưa
    const check = await pool.query('SELECT * FROM devices WHERE device_id = $1', [device_id]);
    if (check.rowCount > 0) return res.status(400).json({ message: 'Device already exists' });

    // Thêm thiết bị mới
    const result = await pool.query(
      'INSERT INTO devices (device_id, user_id, phone_number) VALUES ($1, $2, $3) RETURNING *',
      [device_id, req.userId, phone_number]
    );
    res.status(201).json({ device: result.rows[0] });
  } catch (err) {
    res.status(500).json({ message: 'Error adding device' });
  }
});

// Lấy thông tin thiết bị
router.get('/devices/:deviceId', authMiddleware, async (req, res) => {
  const { deviceId } = req.params;
  try {
    const result = await pool.query(
      'SELECT * FROM devices WHERE device_id = $1',
      [deviceId]
    );
    if (result.rowCount === 0) return res.status(404).json({ message: 'Device not found' });
    res.json({ device: result.rows[0] });
  } catch (err) {
    res.status(500).json({ message: 'Error fetching device' });
  }
});

// Cập nhật số điện thoại cho thiết bị
router.put('/devices/:deviceId/phone', authMiddleware, async (req, res) => {
  const { deviceId } = req.params;
  const { phone_number } = req.body;
  try {
    const result = await pool.query(
      'UPDATE devices SET phone_number = $1, user_id = $2 WHERE device_id = $3 RETURNING *',
      [phone_number, req.userId, deviceId]
    );
    if (result.rowCount === 0) return res.status(404).json({ message: 'Device not found' });
    res.json({ device: result.rows[0] });
  } catch (err) {
    res.status(500).json({ message: 'Error updating phone number' });
  }
});

// Lấy số điện thoại của thiết bị
router.get('/devices/:deviceId/phone', async (req, res) => {
  const { deviceId } = req.params;
  try {
    const result = await pool.query(
      'SELECT phone_number FROM devices WHERE device_id = $1',
      [deviceId]
    );
    if (result.rowCount === 0) return res.status(404).json({ message: 'Device not found' });
    res.json({ phone_number: result.rows[0].phone_number });
  } catch (err) {
    res.status(500).json({ message: 'Error fetching phone number' });
  }
});

// API để log alerts từ ESP32
router.post('/alerts', async (req, res) => {
  const { device_id, alert_type, message, phone_number } = req.body;
  try {
    const result = await pool.query(
      'INSERT INTO alerts (device_id, alert_type, message, phone_number) VALUES ($1, $2, $3, $4) RETURNING *',
      [device_id, alert_type, message, phone_number]
    );
    res.status(201).json({ alert: result.rows[0] });
  } catch (err) {
    console.error('Error logging alert:', err);
    res.status(500).json({ message: 'Error logging alert' });
  }
});

// API để lấy lịch sử alerts
router.get('/alerts/:deviceId', authMiddleware, async (req, res) => {
  const { deviceId } = req.params;
  try {
    const result = await pool.query(
      'SELECT * FROM alerts WHERE device_id = $1 ORDER BY sent_at DESC LIMIT 50',
      [deviceId]
    );
    res.json({ alerts: result.rows });
  } catch (err) {
    res.status(500).json({ message: 'Error fetching alerts' });
  }
});

module.exports = router;