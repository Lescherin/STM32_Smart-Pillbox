<template>
  <view class="time-setting-page">
    <!-- 粘性布局容器 -->
    <view class="sticky-container">
      <!-- 设备选择器 -->
      <view class="field-selector">
        <picker @change="onDeviceChange" :value="selectedDeviceIndex" :range="deviceLabels" v-if="devices.length > 0">
          <view class="picker-view">
            选择设备: {{ deviceLabels[selectedDeviceIndex] || '请选择' }}
          </view>
        </picker>
      </view>
      
      <!-- 时间选择 -->
      <view class="time-selector" v-if="devices.length > 0">
        <view class="time-input">
          <picker mode="date" :value="selectedDate" start="2023-01-01" end="2030-12-31" @change="onDateChange" class="date-picker">
            {{ selectedDate || '选择日期' }}
          </picker>
          <picker mode="time" :value="selectedTime" start="00:00" end="23:59" @change="onTimeChange" class="time-picker">
            {{ selectedTime || '选择时间' }}
          </picker>
        </view>
        
        <!-- 状态选择 -->
        <view class="status-selector">
          <text class="status-label">触发状态:</text>
          <switch 
            :checked="triggerStatus" 
            @change="onStatusChange" 
            color="#007aff"
          />
          <text>{{ triggerStatus ? '开启' : '关闭' }}</text>
        </view>
      </view>
      
      <!-- 添加定时任务按钮 -->
      <button class="add-btn" @click="addTimeTask" v-if="devices.length > 0 && selectedDeviceIndex !== -1">
        添加定时任务
      </button>
    </view>
    
    <!-- 定时任务列表 -->
    <view class="task-list" v-if="timeTasks.length > 0">
      <view class="task-item" v-for="(task, index) in timeTasks" :key="index">
        <view class="task-info">
          <view class="task-device">
            {{ getDeviceLabel(task.deviceKey) }}
            <span class="task-status" :class="task.status ? 'status-on' : 'status-off'">
              {{ task.status ? '开启' : '关闭' }}
            </span>
          </view>
          <view class="task-time">{{ task.date }} {{ task.time }}</view>
        </view>
        <view class="delete-btn" @click="deleteTask(index)">删除</view>
      </view>
    </view>
    
    <!-- 空状态 -->
    <view class="empty" v-if="devices.length === 0">
      未配置任何控制设备
    </view>
    <view class="empty" v-if="devices.length > 0 && timeTasks.length === 0">
      暂无定时任务，请添加
    </view>
  </view>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue';
import config from '../../page_config/config.js';
import { mqtt_publish } from '../../utils/mqtt_util.js';
import './set_time.scss';

// 常量定义
const STORAGE_KEY = 'timeTasks';

// 状态管理
const devices = ref([]);
const deviceLabels = computed(() => devices.value.map(device => device.label || '未知设备'));
const selectedDeviceIndex = ref(-1);
const selectedDate = ref('');
const selectedTime = ref('');
const triggerStatus = ref(true);
const timeTasks = ref([]);

// 辅助函数：数字补零
function padZero(num) {
  return num < 10 ? `0${num}` : num;
}

// 加载定时任务
function loadTimeTasks() {
  try {
    const saved = uni.getStorageSync(STORAGE_KEY);
    if (Array.isArray(saved)) {
      timeTasks.value = saved;
    }
  } catch (error) {
    // 静默失败，保持空列表
  }
}

// 保存定时任务到本地存储
function saveTimeTasks() {
  try {
    uni.setStorageSync(STORAGE_KEY, timeTasks.value);
    return true;
  } catch (error) {
    return false;
  }
}

// 执行定时任务
const executeTimeTask = (task) => {
  const device = devices.value.find(d => d.key === task.deviceKey);
  if (!device) return;
  
  // 构建MQTT消息
  const rawMessage = task.status ? device.mqttConfig?.onMessage : device.mqttConfig?.offMessage;
  const message = rawMessage ? JSON.stringify({ message: rawMessage }) : null;
  
  if (message) {
    try {
      mqtt_publish(message);
      
      // 显示操作结果通知
      uni.showToast({
        title: `${device.label}已${task.status ? '开启' : '关闭'}`,
        icon: 'none'
      });
    } catch (error) {
      // 静默处理错误
    }
  }
};

// 检查定时任务是否到达执行时间
const checkTimeTasks = () => {
  const now = new Date();
  const currentTimeStr = `${now.getFullYear()}-${padZero(now.getMonth() + 1)}-${padZero(now.getDate())} ${padZero(now.getHours())}:${padZero(now.getMinutes())}`;
  
  // 查找需要执行的任务（精确到分钟）
  const tasksToExecute = timeTasks.value.filter(task => 
    `${task.date} ${task.time}` === currentTimeStr
  );
  
  // 带随机延迟执行任务并移除已执行任务
  tasksToExecute.forEach(task => {
    // 添加随机延迟避免同时执行
    const delay = Math.floor(Math.random() * 1000);
    
    setTimeout(() => {
      executeTimeTask(task);
      
      // 移除已执行的任务
      const taskIndex = timeTasks.value.findIndex(t => 
        t.deviceKey === task.deviceKey && 
        t.date === task.date && 
        t.time === task.time &&
        t.createdAt === task.createdAt
      );
      
      if (taskIndex !== -1) {
        timeTasks.value.splice(taskIndex, 1);
        saveTimeTasks();
      }
    }, delay);
  });
};

// 事件处理函数
const onDeviceChange = (e) => {
  selectedDeviceIndex.value = e.detail.value;
};

const onDateChange = (e) => {
  selectedDate.value = e.detail.value;
};

const onTimeChange = (e) => {
  selectedTime.value = e.detail.value;
};

const onStatusChange = (e) => {
  triggerStatus.value = e.detail.value;
};

// 添加定时任务
const addTimeTask = () => {
  if (!selectedDate.value || !selectedTime.value) {
    uni.showToast({ title: '请选择日期和时间', icon: 'none' });
    return;
  }
  
  const selectedDevice = devices.value[selectedDeviceIndex.value];
  if (!selectedDevice) return;
  
  // 创建并添加新任务
  timeTasks.value.push({
    deviceKey: selectedDevice.key,
    date: selectedDate.value,
    time: selectedTime.value,
    status: triggerStatus.value,
    createdAt: new Date().getTime()
  });
  
  // 保存并提示
  if (saveTimeTasks()) {
    uni.showToast({ title: '定时任务添加成功', icon: 'success' });
  } else {
    uni.showToast({ title: '保存失败', icon: 'none' });
  }
};

// 删除定时任务
const deleteTask = (index) => {
  uni.showModal({
    title: '确认删除',
    content: '确定要删除这个定时任务吗？',
    success: (res) => {
      if (res.confirm) {
        timeTasks.value.splice(index, 1);
        saveTimeTasks();
        uni.showToast({ title: '已删除', icon: 'none' });
      }
    }
  });
};

// 根据设备key获取设备名称
const getDeviceLabel = (deviceKey) => {
  const device = devices.value.find(d => d.key === deviceKey);
  return device ? device.label : '未知设备';
};

// 初始化组件
onMounted(() => {
  // 加载控制设备
  if (Array.isArray(config.devices_ctrl)) {
    devices.value = config.devices_ctrl.filter(device => 
      device && typeof device.key === 'string'
    );
  }
  
  // 加载已保存的定时任务
  loadTimeTasks();
  
  // 设置默认值
  if (devices.value.length > 0) {
    selectedDeviceIndex.value = 0;
  }
  
  // 设置默认日期为今天
  const today = new Date();
  selectedDate.value = `${today.getFullYear()}-${padZero(today.getMonth() + 1)}-${padZero(today.getDate())}`;
  
  // 立即检查一次任务
  checkTimeTasks();
  
  // 设置定时检查
  setInterval(checkTimeTasks, 60000);
});
</script>

