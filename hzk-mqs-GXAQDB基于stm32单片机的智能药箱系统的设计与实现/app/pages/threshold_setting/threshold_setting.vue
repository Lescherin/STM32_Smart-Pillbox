<template>
  <view class="threshold-page">
    
    <!-- 设备选择器 -->
    <view class="sticky-container">
      <view class="field-selector">
        <picker @change="onDeviceChange" :value="selectedDeviceIndex" :range="deviceLabels" v-if="devices.length > 0">
          <view class="picker-view">
            选择设备: {{ deviceLabels[selectedDeviceIndex] || '请选择' }}
          </view>
        </picker>
      </view>
    </view>
    
    <!-- 当前选中设备的阈值设置 -->
    <view class="threshold-card" v-if="currentDevice">
      <view class="device-info">
        <image class="device-icon" :src="currentDevice.icon" mode="widthFix"></image>
        <view class="device-name">{{ currentDevice.label }}</view>
      </view>
      
      <view class="threshold-controls">
        <view class="threshold-item">
          <text class="label">最小值阈值</text>
          <input 
            type="number" 
            v-model.number="currentThreshold.min"
            class="input"
          />
        </view>
        
        <view class="threshold-item">
          <text class="label">最大值阈值</text>
          <input 
            type="number" 
            v-model.number="currentThreshold.max"
            class="input"
          />
        </view>
      </view>
    </view>
    
    <!-- 空状态提示 -->
    <view class="empty" v-if="!currentDevice && devices.length > 0">
      请先选择设备
    </view>
    <view class="empty" v-if="devices.length === 0">
      未配置任何设备
    </view>
    
    <button class="save-btn" @click="saveThreshold" v-if="currentDevice">
      保存设置
    </button>
  </view>
</template>

<script setup>
import { ref, onMounted, computed, watch } from 'vue';
import config from '../../page_config/config.js';
import thresholdManager from '../../utils/thresholdManager';
import './threshold_setting.scss';

// 设备列表和状态管理
const devices = ref([]);
const deviceLabels = computed(() => devices.value.map(device => device.label || '未知设备'));
const selectedDeviceIndex = ref(-1);
const currentThreshold = ref({ min: 0, max: 0 });
const loading = ref(false);

// 当前选中的设备
const currentDevice = computed(() => {
  if (devices.value.length === 0 || selectedDeviceIndex.value < 0) return null;
  return devices.value[selectedDeviceIndex.value] || null;
});

// 初始化组件
onMounted(() => {
  try {
    // 初始化阈值管理器
    thresholdManager.initThresholds();
    
    // 加载有效的设备列表
    if (Array.isArray(config.devices)) {
      devices.value = config.devices.filter(device => 
        device && typeof device.key === 'string'
      );
    }
    
    // 默认选择第一个设备
    if (devices.value.length > 0) {
      selectedDeviceIndex.value = 0;
    }
  } catch (error) {
    uni.showToast({
      title: '初始化失败',
      icon: 'none'
    });
  }
});

// 监听设备变化，更新阈值显示
watch([selectedDeviceIndex, devices], () => {
  if (currentDevice.value) {
    loadThreshold();
  }
}, { immediate: true });

// 加载设备阈值
function loadThreshold() {
  if (!currentDevice.value) return;
  
  const deviceKey = currentDevice.value.key;
  
  // 确保阈值管理器已初始化
  if (!thresholdManager.cachedThresholds) {
    thresholdManager.initThresholds();
  }
  
  // 获取阈值配置
  const thresholds = thresholdManager.getThresholds();
  const savedThreshold = thresholds[deviceKey];
  
  if (savedThreshold) {
    // 使用保存的阈值
    currentThreshold.value = {
      min: Number(savedThreshold.min),
      max: Number(savedThreshold.max)
    };
  } else {
    // 使用设备默认值或零值
    currentThreshold.value = {
      min: Number(currentDevice.value.thresholdMin || 0),
      max: Number(currentDevice.value.thresholdMax || 0)
    };
  }
}

// 切换设备选择
const onDeviceChange = (e) => {
  selectedDeviceIndex.value = e.detail.value;
};

// 保存阈值设置
const saveThreshold = () => {
  if (!currentDevice.value) {
    uni.showToast({ title: '请选择设备', icon: 'none' });
    return;
  }
  
  // 验证输入数据
  const max = Number(currentThreshold.value.max);
  const min = Number(currentThreshold.value.min);
  
  if (isNaN(max) || isNaN(min)) {
    uni.showToast({ title: '请输入有效的数字', icon: 'none' });
    return;
  }
  
  if (max < min) {
    uni.showToast({ title: '最大值不能小于最小值', icon: 'none' });
    return;
  }
  
  loading.value = true;
  
  try {
    const deviceKey = currentDevice.value.key;
    
    // 保存阈值设置
    const result = thresholdManager.updateThreshold(deviceKey, { min, max });
    
    if (result) {
      // 更新缓存确保立即生效
      if (thresholdManager.getThresholds) {
        thresholdManager.cachedThresholds = thresholdManager.getThresholds();
      }
      
      // 显示成功提示
      uni.showToast({ title: '保存成功', icon: 'success' });
      
      // 显示详细提示信息
      setTimeout(() => {
        uni.showModal({
          title: '提示',
          content: '阈值设置已保存并立即生效。系统将根据新的阈值进行实时监控。',
          showCancel: false,
          confirmText: '确定'
        });
      }, 500);
    } else {
      throw new Error('保存返回失败');
    }
  } catch (error) {
    uni.showToast({ title: '保存失败，请重试', icon: 'none' });
  } finally {
    loading.value = false;
  }
};
</script>
