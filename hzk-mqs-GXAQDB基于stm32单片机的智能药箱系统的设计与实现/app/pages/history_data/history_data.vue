<template>
  <view class="history-page">
    <!-- 粘性布局容器：包含所有需要固定在顶部的元素 -->
    <view class="sticky-container">
      <!-- 字段选择器 - 显示label -->
      <view class="field-selector">
        <picker @change="onFieldChange" :value="selectedFieldIndex" :range="fieldLabels">
          <view class="picker-view">
            选择设备: {{ fieldLabels[selectedFieldIndex] || '请选择' }}
          </view>
        </picker>
      </view>
    </view>
    
    <!-- 数据列表 -->
    <view class="history-list" v-if="historyData.length > 0">
      <view class="list-header">
        <view class="col">时间</view>
        <view class="col">数值</view>
        <view class="col">字段标识（key）</view>
      </view>
      <view class="list-item" v-for="(item, index) in historyData" :key="index">
        <view class="col">{{ item.datetime }}</view>
        <view class="col">{{ item.value }}</view>
        <view class="col">{{ item.key }}</view>
      </view>
    </view>
    
    <!-- 空状态 -->
    <view class="empty" v-if="historyData.length === 0 && hasQueried">
      <template v-if="!selectedField">请先选择查询字段</template>
      <template v-else>没有找到匹配的历史数据</template>
    </view>
  </view>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue';
import config from '../../page_config/config.js';
import historyManager from '../../utils/historyManager.js';
import '../history_data/history_data.scss';

// 设备配置相关
const devices = computed(() => {
  return Array.isArray(config.devices) ? config.devices : [];
});

const fieldLabels = computed(() => {
  return ['请选择设备', ...devices.value.map(device => device.label || '未知字段')];
});

const fieldKeys = computed(() => {
  return [null, ...devices.value.map(device => device.key || '')];
});

// 状态管理
const selectedFieldIndex = ref(0);
const startDate = ref('');
const endDate = ref('');
const historyData = ref([]);
const hasQueried = ref(false);

// 获取当前选中的字段
const selectedField = computed(() => {
  if (selectedFieldIndex.value < 0 || selectedFieldIndex.value >= fieldKeys.value.length) {
    return null;
  }
  return fieldKeys.value[selectedFieldIndex.value];
});

// 组件挂载时初始化
onMounted(() => {
  onFieldChange({ detail: { value: selectedFieldIndex.value } });
  if (selectedField.value) {
    queryHistory();
  }
});

// 字段变化处理
const onFieldChange = (e) => {
  selectedFieldIndex.value = e.detail.value;
  queryHistory();
};

// 查询历史数据
const queryHistory = () => {
  if (devices.value.length === 0) {
    uni.showToast({ title: '未配置设备字段', icon: 'none' });
    return;
  }
  
  if (!selectedField.value) {
    historyData.value = [];
    hasQueried.value = true;
    return;
  }
  
  try {
    const startTime = startDate.value ? new Date(startDate.value).getTime() : null;
    const endTime = endDate.value ? new Date(endDate.value).getTime() : null;
    
    const results = historyManager.queryHistoryData({
      field: selectedField.value,
      startTime,
      endTime
    });
    
    // 处理查询结果并排序
    historyData.value = results
      .map(record => ({
        datetime: record.datetime || new Date(record.timestamp).toLocaleString(),
        value: record[selectedField.value] ?? '未知',
        key: selectedField.value 
      }))
      .sort((a, b) => new Date(b.datetime) - new Date(a.datetime));
    
    hasQueried.value = true;
  } catch (error) {
    uni.showToast({ title: '查询失败，请重试', icon: 'none' });
  }
};
</script>
<!-- </script> -->