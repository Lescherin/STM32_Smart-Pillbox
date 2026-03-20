<template>
  <view class="container">
    <!-- 顶部标题栏 -->
    <view class="header">
      <view class="title-section">
        <text class="title">摄像头查看</text>
        <text class="subtitle">实时视频流监控</text>
      </view>
      <view class="status-indicator" :class="streamStatus ? 'active' : 'inactive'">
        <text class="status-text">{{ streamStatus ? '网络已连接' : '网络未连接' }}</text>
        <view class="status-dot"></view>
      </view>
    </view>

    <!-- 控制面板 -->
    <view class="control-panel">
      <view class="input-section">
        <view class="input-group">
          <text class="label">IP地址</text>
          <input 
            v-model="ipAddress" 
            class="ip-input" 
            placeholder="例如: 192.168.0.127" 
            placeholder-class="placeholder"
          />
        </view>
        
        <view class="button-group">
          <button 
            class="btn btn-primary" 
            :disabled="!isValidIP || isStreaming" 
            @tap="startStream"
          >
            {{ isStreaming ? '连接中...' : '开始连接' }}
          </button>
		  
          <button 
            class="btn btn-save" 
            :disabled="!isValidIP" 
            @tap="saveIPAddress"
          >
            保存IP
          </button>
        </view>
      </view>
    </view>

    <!-- 视频流显示区域 -->
    <view class="stream-section">
      <view class="stream-container" v-if="isStreaming">
        <view class="stream-header">
          <text class="stream-title">实时视频流</text>
          <text class="resolution-info">{{ getResolutionText() }}</text>
        </view>
        
        <!-- 视频控制选项 -->
        <view class="video-controls">
          <button 
            class="control-btn" 
            @tap="rotateVideo"
            :class="{ active: rotation !== 0 }"
          >
            <text class="icon">🔄</text>
            <text class="control-text">旋转</text>
          </button>
          <button 
            class="control-btn" 
            @tap="flipVideo"
            :class="{ active: flipped }"
          >
            <text class="icon">↔</text>
            <text class="control-text">翻转</text>
          </button>
        </view>
        
        <!-- 视频显示区域（带变换效果） -->
        <view class="video-wrapper" :style="videoTransform">
          <image 
            :src="streamUrl" 
            class="stream-image" 
            mode="aspectFit"
            @error="onImageError"
            @load="onImageLoad"
            ref="streamImageRef"
          />
        </view>
        
        <view class="stream-status" :class="streamStatusClass">
          <text class="status-icon">{{ streamStatusIcon }}</text>
          <text>{{ streamStatusText }}</text>
        </view>
      </view>
      <view class="placeholder-section" v-else>
        <view class="placeholder-icon">
          <text class="icon">📷</text>
        </view>
        <text class="placeholder-text">请输入IP地址并开始连接</text>
        <text class="placeholder-hint">支持OV2640摄像头JPEG流</text>
      </view>
    </view>

    <!-- 已保存的IP地址 -->
    <view class="saved-ips-section" v-if="savedIPs.length > 0">
      <view class="section-title">
        <text>已保存的IP地址</text>
        <text class="section-subtitle">点击快速选择</text>
      </view>
      <scroll-view class="ip-list" scroll-y>
        <view 
          v-for="(ip, index) in savedIPs" 
          :key="index" 
          class="ip-item"
          @tap="selectSavedIP(ip)"
        >
          <text class="ip-text">{{ ip }}</text>
          <text class="delete-btn" @tap.stop="deleteSavedIP(index)">×</text>
        </view>
      </scroll-view>
    </view>
  </view>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'

// 响应式数据
const ipAddress = ref('192.168.0.127')
const isStreaming = ref(false)
const streamUrl = ref('') // 用于绑定到image元素
const savedIPs = ref([])
const streamStatus = ref('idle') // idle, loading, error, success
const frameSize = ref(12) // 默认SXGA分辨率 (1280x1024)
const streamImageRef = ref(null)

// 视频变换相关状态
const rotation = ref(0) // 旋转角度：0, 90, 180, 270
const flipped = ref(false) // 是否水平翻转

// 内部变量，用于构建稳定的流地址
let baseStreamUrl = ref('')

// 计算属性
const isValidIP = computed(() => {
  const ipRegex = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
  return ipRegex.test(ipAddress.value)
})

const streamStatusClass = computed(() => {
  return `status-${streamStatus.value}`
})

const streamStatusText = computed(() => {
  const statusMap = {
    idle: '等待连接',
    loading: '正在连接...',
    error: '连接失败，请检查设备是否与摄像头在同一网段。',
    success: '连接成功'
  }
  return statusMap[streamStatus.value]
})

const streamStatusIcon = computed(() => {
  const iconMap = {
    idle: '●',
    loading: '⟳',
    error: '⚠',
    success: '✓'
  }
  return iconMap[streamStatus.value]
})

// 计算视频变换样式
const videoTransform = computed(() => {
  let transform = `rotate(${rotation.value}deg)`
  if (flipped.value) {
    transform += ' scaleX(-1)'
  }
  return { 
    transform,
    transition: 'transform 0.3s ease' // 平滑过渡效果
  }
})

// 方法
const startStream = () => {
  if (!isValidIP.value || isStreaming.value) return

  isStreaming.value = true
  streamStatus.value = 'loading'

  // 构建基础流URL，确保始终使用同一个流
  baseStreamUrl.value = `http://${ipAddress.value}:81/stream`
  
  // 使用带时间戳的URL来触发加载，避免缓存问题
  streamUrl.value = `${baseStreamUrl.value}?t=${Date.now()}`
}


// 旋转视频（每次90度）
const rotateVideo = () => {
  rotation.value = (rotation.value + 90) % 360;
}

// 水平翻转视频
const flipVideo = () => {
  flipped.value = !flipped.value;
}

const saveIPAddress = () => {
  if (!isValidIP.value) return
  
  if (!savedIPs.value.includes(ipAddress.value)) {
    // 限制保存的IP数量，防止过多
    if (savedIPs.value.length >= 10) {
      savedIPs.value.shift();
    }
    savedIPs.value.push(ipAddress.value)
    uni.setStorageSync('savedIPs', JSON.stringify(savedIPs.value))
    uni.showToast({ title: 'IP地址已保存', icon: 'success', duration: 2000 })
  } else {
    uni.showToast({ title: 'IP地址已存在', icon: 'none', duration: 2000 })
  }
}

const selectSavedIP = (ip) => {
  ipAddress.value = ip
}

const deleteSavedIP = (index) => {
  savedIPs.value.splice(index, 1)
  uni.setStorageSync('savedIPs', JSON.stringify(savedIPs.value))
  uni.showToast({ title: '已删除', icon: 'success', duration: 2000 })
}

// 当图片加载失败时触发
const onImageError = () => {
  if (isStreaming.value) {
    streamStatus.value = 'error'
    
    // 如果仍处于 streaming 状态，则尝试重新连接
    setTimeout(() => {
      if (isStreaming.value) {
        streamUrl.value = `${baseStreamUrl.value}?t=${Date.now()}`
      }
    }, 3000)
  }
}

// 当图片成功加载时触发
const onImageLoad = () => {
  if (isStreaming.value) {
    streamStatus.value = 'success'
    
    // 继续请求下一张图片，实现连续播放
    // 这是MJPEG流客户端侧最常见的实现方式
    if (isStreaming.value) {
      streamUrl.value = `${baseStreamUrl.value}?t=${Date.now()}`
    }
  }
}

const handleConfigChange = async (key, value) => {
  if (!isStreaming.value) {
    uni.showToast({ title: '请先连接摄像头', icon: 'none', duration: 2000 });
    return;
  }
  
  try {
    await fetchUrl(`http://${ipAddress.value}/control?var=${key}&val=${value}`)
    if (key === 'framesize') {
      frameSize.value = value
      uni.showToast({ title: '分辨率已更新', icon: 'success', duration: 1500 })
    }
  } catch (err) {
    uni.showToast({ title: `设置失败: ${err.message}`, icon: 'none', duration: 3000 })
  }
}

const getResolutionText = () => {
  const resolutionMap = {
    '13': '1600x1200', '12': '1280x1024', '11': '1280x720', '10': '1024x768',
    '9': '800x600',  '8': '640x480',   '7': '480x320',  '6': '400x296',
    '5': '320x240',  '4': '240x240',   '3': '240x176',  '2': '176x144',
    '1': '160x120',  '0': '96x96'
  }
  return resolutionMap[frameSize.value] || '未知分辨率'
}

const fetchUrl = (url) => {
  return new Promise((resolve, reject) => {
    uni.request({
      url,
      method: 'GET',
      success: (res) => {
        if (res.statusCode === 200) {
          resolve(res.data)
        } else {
          reject(new Error(`HTTP error! status: ${res.statusCode}`))
        }
      },
      fail: (err) => {
        reject(err)
      }
    })
  })
}

onMounted(() => {
  try {
    const saved = uni.getStorageSync('savedIPs')
    if (saved) {
      savedIPs.value = JSON.parse(saved)
    }
  } catch (e) {
    console.error('加载保存的IP地址失败:', e)
  }
})
</script>

<style>
.container {
  padding: 30rpx;
  background: linear-gradient(135deg, #1a2a6c, #b21f1f, #fdbb2d);
  min-height: 100vh;
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
}

/* 头部样式 */
.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 40rpx;
  padding-bottom: 20rpx;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

.title-section {
  display: flex;
  flex-direction: column;
}

.title {
  font-size: 48rpx;
  font-weight: bold;
  color: white;
  margin-bottom: 8rpx;
  text-shadow: 0 2rpx 4rpx rgba(0, 0, 0, 0.2);
}

.subtitle {
  font-size: 26rpx;
  color: rgba(255, 255, 255, 0.8);
}

.status-indicator {
  display: flex;
  align-items: center;
  background: rgba(255, 255, 255, 0.2);
  padding: 12rpx 20rpx;
  border-radius: 50rpx;
  backdrop-filter: blur(5rpx);
  box-shadow: 0 2rpx 8rpx rgba(0, 0, 0, 0.1);
}

.status-text {
  font-size: 24rpx;
  color: white;
  margin-right: 12rpx;
}

.status-dot {
  width: 16rpx;
  height: 16rpx;
  border-radius: 50%;
  transition: all 0.3s ease;
}

.status-indicator.active .status-dot {
  background: #4CD964;
  box-shadow: 0 0 10rpx #4CD964;
  animation: pulse 2s infinite;
}

.status-indicator.inactive .status-dot {
  background: #FF3B30;
}

/* 控制面板样式 */
.control-panel {
  background: rgba(255, 255, 255, 0.95);
  border-radius: 20rpx;
  padding: 30rpx;
  margin-bottom: 30rpx;
  box-shadow: 0 10rpx 30rpx rgba(0, 0, 0, 0.15);
  transition: transform 0.2s ease;
}

.control-panel:active {
  transform: translateY(2rpx);
}

.input-group {
  margin-bottom: 30rpx;
}

.label {
  display: block;
  font-size: 30rpx;
  margin-bottom: 15rpx;
  color: #333;
  font-weight: 500;
}

.ip-input {
  width: 93%;
  height: 80rpx;
  border: 2rpx solid #ddd;
  border-radius: 12rpx;
  padding: 0 20rpx;
  font-size: 28rpx;
  background: white;
  transition: border-color 0.3s ease;
}

.ip-input:focus {
  border-color: #007AFF;
  outline: none;
  box-shadow: 0 0 0 4rpx rgba(0, 122, 255, 0.1);
}

.placeholder {
  color: #999;
}

.button-group {
  display: flex;
  justify-content: space-between;
  gap: 20rpx;
}

.btn {
  flex: 1;
  height: 80rpx;
  border-radius: 12rpx;
  font-size: 28rpx;
  border: none;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: 500;
  transition: all 0.3s;
  cursor: pointer;
}

.btn:active {
  transform: scale(0.98);
}

.btn-primary {
  background: #007AFF;
  color: white;
}

.btn-primary:disabled {
  background: #ccc;
  color: #666;
}

.btn-secondary {
  background: #FF3B30;
  color: white;
}

.btn-secondary:disabled {
  background: #ccc;
  color: #666;
}

.btn-save {
  background: #4CD964;
  color: white;
}

.btn-save:disabled {
  background: #ccc;
  color: #666;
}

/* 配置区域样式 */
.config-section {
  margin-top: 40rpx;
  padding-top: 30rpx;
  border-top: 1rpx solid #eee;
}

.section-title {
  font-size: 32rpx;
  font-weight: bold;
  color: #333;
  margin-bottom: 20rpx;
}

.config-label {
  display: block;
  font-size: 28rpx;
  margin-bottom: 15rpx;
  color: #555;
}

.config-select {
  width: 100%;
  height: 80rpx;
  border: 2rpx solid #ddd;
  border-radius: 12rpx;
  padding: 0 20rpx;
  font-size: 28rpx;
  background: white;
}

/* 视频流区域样式 */
.stream-section {
  background: rgba(255, 255, 255, 0.95);
  border-radius: 20rpx;
  padding: 30rpx;
  margin-bottom: 30rpx;
  box-shadow: 0 10rpx 30rpx rgba(0, 0, 0, 0.15);
}

.stream-container {
  position: relative;
}

.stream-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20rpx;
}

.stream-title {
  font-size: 32rpx;
  font-weight: bold;
  color: #333;
}

.resolution-info {
  font-size: 24rpx;
  color: #666;
  background: #f0f0f0;
  padding: 8rpx 16rpx;
  border-radius: 20rpx;
}

/* 视频控制按钮 */
.video-controls {
  display: flex;
  gap: 15rpx;
  margin-bottom: 20rpx;
  position: relative;
  z-index: 10; /* 确保按钮在视频上方 */
}

.control-btn {
  flex: 1;
  height: 70rpx;
  background: #f5f5f5;
  border: none;
  border-radius: 10rpx;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10rpx;
  font-size: 24rpx;
  color: #333;
  transition: all 0.3s ease;
}

.control-btn .icon {
  font-size: 28rpx;
}

.control-btn.active {
  background: #007AFF;
  color: white;
}

.control-btn:active {
  transform: scale(0.96);
}

/* 视频容器 - 关键修改 */
.video-wrapper {
  width: 100%;
  height: 400rpx;
  display: flex;
  align-items: center;
  justify-content: center;
  overflow: hidden; /* 关键：隐藏超出容器的内容 */
  background: #f5f5f5;
  border-radius: 12rpx;
  border: 1rpx solid #eee;
  position: relative;
  transition: height 0.3s ease; /* 平滑过渡高度变化 */
}

.stream-image {
  max-width: 100%;
  max-height: 100%;
  transition: all 0.3s ease;
  transform-origin: center; /* 确保围绕中心旋转 */
}

/* 当视频旋转90或270度时调整尺寸 */
.video-wrapper.rotated-90,
.video-wrapper.rotated-270 {
  height: 600rpx; /* 增加垂直空间 */
}

.video-wrapper.rotated-90 .stream-image,
.video-wrapper.rotated-270 .stream-image {
  max-width: 100%;
  max-height: 100%;
}

.stream-status {
  position: absolute;
  bottom: 20rpx;
  left: 20rpx;
  display: flex;
  align-items: center;
  padding: 10rpx 20rpx;
  border-radius: 20rpx;
  font-size: 24rpx;
  color: white;
  background: rgba(0, 0, 0, 0.6);
  backdrop-filter: blur(5rpx);
  box-shadow: 0 2rpx 8rpx rgba(0, 0, 0, 0.2);
  z-index: 5; /* 确保状态显示在视频上方 */
}

.status-icon {
  margin-right: 10rpx;
  font-weight: bold;
}

.status-idle {
  background: rgba(153, 153, 153, 0.8);
}

.status-loading {
  background: rgba(255, 149, 0, 0.8);
}

.status-error {
  background: rgba(255, 59, 48, 0.8);
}

.status-success {
  background: rgba(76, 217, 100, 0.8);
}

.placeholder-section {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 400rpx;
  background: #f5f5f5;
  border-radius: 12rpx;
  border: 1rpx dashed #ddd;
}

.placeholder-icon {
  width: 120rpx;
  height: 120rpx;
  border-radius: 50%;
  background: #e9e9e9;
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 30rpx;
  box-shadow: 0 4rpx 12rpx rgba(0, 0, 0, 0.1);
}

.placeholder-icon .icon {
  font-size: 60rpx;
  color: #999;
}

.placeholder-text {
  font-size: 32rpx;
  color: #666;
  margin-bottom: 10rpx;
}

.placeholder-hint {
  font-size: 24rpx;
  color: #999;
}

/* 已保存IP地址样式 */
.saved-ips-section {
  background: rgba(255, 255, 255, 0.95);
  border-radius: 20rpx;
  padding: 30rpx;
  box-shadow: 0 10rpx 30rpx rgba(0, 0, 0, 0.15);
}

.section-subtitle {
  font-size: 24rpx;
  color: #666;
  margin-left: 15rpx;
}

.ip-list {
  max-height: 300rpx;
}

.ip-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20rpx 0;
  border-bottom: 1rpx solid #eee;
  transition: background-color 0.2s ease;
}

.ip-item:last-child {
  border-bottom: none;
}

.ip-item:active {
  background-color: #f5f5f5;
}

.ip-text {
  font-size: 28rpx;
  color: #333;
}

.delete-btn {
  font-size: 36rpx;
  color: #FF3B30;
  width: 50rpx;
  height: 50rpx;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(255, 59, 48, 0.1);
  transition: all 0.2s ease;
}

.delete-btn:active {
  background: rgba(255, 59, 48, 0.2);
  transform: scale(0.9);
}

/* 动画效果 */
@keyframes pulse {
  0% {
    box-shadow: 0 0 0 0 rgba(76, 217, 100, 0.4);
  }
  70% {
    box-shadow: 0 0 0 10rpx rgba(76, 217, 100, 0);
  }
  100% {
    box-shadow: 0 0 0 0 rgba(76, 217, 100, 0);
  }
}

</style>
    