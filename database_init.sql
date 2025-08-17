-- Schema cho Render PostgreSQL
-- Chạy script này trong Render Database Console

-- Tạo bảng users
CREATE TABLE IF NOT EXISTS users (
  id SERIAL PRIMARY KEY,
  email VARCHAR(255) UNIQUE NOT NULL,
  password_hash VARCHAR(255) NOT NULL,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Tạo bảng devices  
CREATE TABLE IF NOT EXISTS devices (
  id SERIAL PRIMARY KEY,
  device_id VARCHAR(20) UNIQUE NOT NULL,
  user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
  phone_number VARCHAR(15),
  status VARCHAR(20) DEFAULT 'active',
  last_seen TIMESTAMP,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Tạo bảng alerts
CREATE TABLE IF NOT EXISTS alerts (
  id SERIAL PRIMARY KEY,
  device_id VARCHAR(20) REFERENCES devices(device_id),
  alert_type VARCHAR(50),
  message TEXT,
  phone_number VARCHAR(15),
  sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  status VARCHAR(20) DEFAULT 'sent'
);

-- Tạo indexes để tăng performance
CREATE INDEX IF NOT EXISTS idx_devices_device_id ON devices(device_id);
CREATE INDEX IF NOT EXISTS idx_alerts_device_id ON alerts(device_id);
CREATE INDEX IF NOT EXISTS idx_alerts_sent_at ON alerts(sent_at);

-- Insert sample device for testing
INSERT INTO devices (device_id, phone_number, status) 
VALUES ('ESP32_001', NULL, 'active')
ON CONFLICT (device_id) DO NOTHING;
