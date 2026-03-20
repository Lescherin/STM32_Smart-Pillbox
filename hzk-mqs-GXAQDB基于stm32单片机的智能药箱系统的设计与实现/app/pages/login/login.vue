<template>
  <view class="login">
    <view class="login-container">
      <!-- 标题切换 -->
      <view class="login_top">
        <text>{{ isLoginMode ? '欢迎回来' : '注册账号' }}</text>
        <text class="subtitle">{{ isLoginMode ? '请登录您的账户' : '请创建您的账户' }}</text>
      </view>
      
      <!-- 输入区域 -->
      <view class="login_body">
        <view class="login_input">
          <!-- 用户名输入框 -->
          <view class="input-group">
            <view class="input-label">用户名</view>
            <input 
              class="uni-input" 
              placeholder="请输入用户名" 
              placeholder-class="placeholder-style"
              v-model="username"
            />
          </view>
          
          <!-- 密码输入框 -->
          <view class="input-group">
            <view class="input-label">密码</view>
            <input 
              class="uni-input" 
              placeholder="请输入密码" 
              placeholder-class="placeholder-style"
              v-model="password"
              type="password"
            />
          </view>
          
          <!-- 确认密码（仅注册显示） -->
          <view class="input-group" v-if="!isLoginMode">
            <view class="input-label">确认密码</view>
            <input 
              class="uni-input" 
              placeholder="请再次输入密码" 
              placeholder-class="placeholder-style"
              v-model="confirmPassword"
              type="password"
            />
          </view>
          
          <!-- 功能按钮区 -->
          <view class="submit-btn">
            <button 
              type="primary" 
              @click="handleSubmit" 
              :disabled="!isFormValid || isLoading"
              :loading="isLoading"
            >
              {{ isLoginMode ? '登 录' : '注 册' }}
            </button>
          </view>
          
          <!-- 模式切换和找回密码 -->
          <view class="login-links">
            <text 
              class="link" 
              @click="toggleMode"
            >
              {{ isLoginMode ? '没有账号？注册' : '已有账号？登录' }}
            </text>
            <text 
              class="link" 
              @click="handleForgotPassword"
              v-if="isLoginMode"
            >
              忘记密码？
            </text>
          </view>
        </view>
      </view>
    </view>
  </view>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue';
import DB from '@/utils/sqlite.js';
import '../login/login.scss'; 

// 状态管理
const isLoginMode = ref(true); // true:登录模式 false:注册模式
const username = ref('');
const password = ref('');
const confirmPassword = ref('');
const isLoading = ref(false);
const resetPasswordVisible = ref(false); // 找回密码弹窗控制

// 表单验证
const isFormValid = computed(() => {
  if (!username.value.trim() || !password.value.trim()) {
    return false;
  }
  // 注册模式需要验证密码一致性
  if (!isLoginMode.value && password.value !== confirmPassword.value) {
    return false;
  }
  return true;
});

// 初始化数据库
onMounted(async () => {
  try {
    if (!DB.isOpen()) {
      await DB.openSqlite();
    }
  } catch (error) {
    uni.showToast({
      title: '数据库连接失败',
      icon: 'none'
    });
  }
});

// 切换登录/注册模式
const toggleMode = () => {
  isLoginMode.value = !isLoginMode.value;
  // 清空表单
  password.value = '';
  confirmPassword.value = '';
};

// 处理登录/注册提交
const handleSubmit = async () => {
  if (isLoading.value) return;
  isLoading.value = true;

  try {
    if (isLoginMode.value) {
      await handleLogin();
    } else {
      await handleRegister();
    }
  } catch (error) {
    uni.showToast({
      title: error.message || '操作失败',
      icon: 'none'
    });
  } finally {
    isLoading.value = false;
  }
};

// 登录处理
const handleLogin = async () => {
  const user = await DB.selectTableData('user', 'username', username.value);
  
  if (user.length === 0) {
    throw new Error('用户名不存在');
  }
  
  if (user[0].password !== password.value) {
    throw new Error('密码错误');
  }
  
  // 登录成功
  uni.showToast({
    title: '登录成功',
    icon: 'none'
  });
  
  // 保存登录状态
  uni.setStorageSync('loginUser', user[0]);
  
  setTimeout(() => {
    uni.switchTab({
      url: '/pages/index/index'
    });
  }, 1000);
};

// 注册处理
const handleRegister = async () => {
  // 检查用户名是否已存在
  const existingUser = await DB.selectTableData('user', 'username', username.value);
  if (existingUser.length > 0) {
    throw new Error('用户名已存在');
  }
  
  // 插入新用户
  const condition = "'username','password'";
  const data = `'${username.value}','${password.value}'`;
  await DB.insertTableData('user', data, condition);
  
  uni.showToast({
    title: '注册成功，请登录',
    icon: 'none'
  });
  
  // 切换到登录模式
  setTimeout(toggleMode, 1000);
};

// 处理忘记密码
const handleForgotPassword = async () => {
  if (!username.value.trim()) {
    return uni.showToast({
      title: '请输入用户名',
      icon: 'none'
    });
  }
  
  try {
    const user = await DB.selectTableData('user', 'username', username.value);
    if (user.length === 0) {
      return uni.showToast({
        title: '用户名不存在',
        icon: 'none'
      });
    }
    
    // 这里简化处理，实际应用中可能需要更复杂的验证
    uni.showModal({
      title: '找回密码',
      content: '请输入新密码',
      editable: true,
      placeholderText: '新密码',
      success: async (res) => {
        if (res.confirm && res.content) {
          await DB.updateTableData(
            'user', 
            `password = '${res.content}'`, 
            'username', 
            username.value
          );
          uni.showToast({
            title: '密码已重置',
            icon: 'none'
          });
        }
      }
    });
  } catch (e) {
    uni.showToast({
      title: '操作失败',
      icon: 'none'
    });
  }
};
</script>


<style>
/* 新增样式 */
.login-links {
  display: flex;
  justify-content: space-between;
  margin-top: 20rpx;
  padding: 0 20rpx;
}

.link {
  color: #007aff;
  font-size: 28rpx;
}
</style>