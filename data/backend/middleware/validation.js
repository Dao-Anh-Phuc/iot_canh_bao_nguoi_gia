const validateEmail = (email) => {
  const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
  return emailRegex.test(email);
};

const validateDeviceId = (deviceId) => {
  // Device ID should be alphanumeric, 6-20 characters
  const deviceRegex = /^[a-zA-Z0-9]{6,20}$/;
  return deviceRegex.test(deviceId);
};

const validatePhoneNumber = (phone) => {
  // Vietnamese phone number format
  const phoneRegex = /^(\+84|84|0)(3|5|7|8|9)([0-9]{8})$/;
  return phoneRegex.test(phone);
};

const validateRegister = (req, res, next) => {
  const { email, password } = req.body;
  
  if (!email || !password) {
    return res.status(400).json({ message: 'Email and password are required' });
  }
  
  if (!validateEmail(email)) {
    return res.status(400).json({ message: 'Invalid email format' });
  }
  
  if (password.length < 6) {
    return res.status(400).json({ message: 'Password must be at least 6 characters' });
  }
  
  next();
};

const validateLogin = (req, res, next) => {
  const { email, password } = req.body;
  
  if (!email || !password) {
    return res.status(400).json({ message: 'Email and password are required' });
  }
  
  if (!validateEmail(email)) {
    return res.status(400).json({ message: 'Invalid email format' });
  }
  
  next();
};

const validateDevice = (req, res, next) => {
  const { device_id } = req.body;
  
  if (!device_id) {
    return res.status(400).json({ message: 'Device ID is required' });
  }
  
  if (!validateDeviceId(device_id)) {
    return res.status(400).json({ message: 'Device ID must be 6-20 alphanumeric characters' });
  }
  
  next();
};

const validatePhone = (req, res, next) => {
  const { phone_number } = req.body;
  
  if (!phone_number) {
    return res.status(400).json({ message: 'Phone number is required' });
  }
  
  if (!validatePhoneNumber(phone_number)) {
    return res.status(400).json({ message: 'Invalid Vietnamese phone number format' });
  }
  
  next();
};

module.exports = {
  validateRegister,
  validateLogin,
  validateDevice,
  validatePhone
};
