// Utility functions for better UX
const showLoading = (buttonEl, text = 'Loading...') => {
  buttonEl.disabled = true;
  buttonEl.dataset.originalText = buttonEl.textContent;
  buttonEl.textContent = text;
  buttonEl.classList.add('loading');
};

const hideLoading = (buttonEl) => {
  buttonEl.disabled = false;
  buttonEl.textContent = buttonEl.dataset.originalText || buttonEl.textContent;
  buttonEl.classList.remove('loading');
};

const showMessage = (elementId, message, type = 'info') => {
  const el = document.getElementById(elementId);
  el.textContent = message;
  el.className = type;
  
  // Auto clear message after 5 seconds
  setTimeout(() => {
    el.textContent = '';
    el.className = '';
  }, 5000);
};

const validateEmail = (email) => {
  const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
  return emailRegex.test(email);
};

const validatePassword = (password) => {
  return password.length >= 6;
};

const validateDeviceId = (deviceId) => {
  const deviceRegex = /^[a-zA-Z0-9]{6,20}$/;
  return deviceRegex.test(deviceId);
};

const validatePhone = (phone) => {
  const phoneRegex = /^(\+84|84|0)(3|5|7|8|9)([0-9]{8})$/;
  return phoneRegex.test(phone);
};

// Enhanced API call with better error handling
const apiCall = async (url, options = {}) => {
  try {
    const response = await fetch(url, {
      headers: {
        'Content-Type': 'application/json',
        ...options.headers
      },
      ...options
    });
    
    const data = await response.json();
    
    if (!response.ok) {
      throw new Error(data.message || `HTTP error! status: ${response.status}`);
    }
    
    return data;
  } catch (error) {
    console.error('API Error:', error);
    throw error;
  }
};

window.utils = {
  showLoading,
  hideLoading,
  showMessage,
  validateEmail,
  validatePassword,
  validateDeviceId,
  validatePhone,
  apiCall
};
