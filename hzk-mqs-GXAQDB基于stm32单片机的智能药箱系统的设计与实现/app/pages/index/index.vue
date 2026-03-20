<template>
	<view class="wrap">
		<!-- OLED显示屏 -->
		<view class="oled-container">
			<view class="oled-title">
				<text class="title-icon">🖥</text>
				<text>OLED显示屏</text>
			</view>
			<view class="oled-screen">
				<!-- 数据显示区域 -->
				<view class="oled-data-section">
					<view class="oled-line" v-for="(item, index) in dataItems" :key="'oled-'+index">
						<view class="oled-data-row">
							<text class="oled-text">{{ item.label }}: {{ item.value }}</text>
							<!-- 为有阈值配置的数据项添加进度条，显示在右侧 -->
							<view v-if="hasThresholdConfig(item)" class="oled-progress-container">
								<view class="oled-progress-bar">
									<view class="oled-progress-fill" 
									      :style="{ width: getDataProgress(item) + '%' }"
									      :class="getProgressStatusClass(item)">
									</view>
								</view>
							</view>
						</view>
					</view>
					<view class="oled-line">
						<text class="oled-text">单片机状态: {{ Stm32Status ? '在线' : '离线' }}</text>
					</view>
				</view>

				<!-- 分割线 -->
				<view class="oled-divider"></view>

				<!-- 控制区域 -->
				<view class="oled-control-section">
					<view class="oled-control-row" v-for="(item, index) in ctrlItems" :key="'oled-ctrl-'+index">
						<view class="oled-control-info">
							<text class="oled-text">{{ item.label }}</text>
							<view class="oled-status-dot" :class="{ 'active': getCtrlItemStatus(item) }"></view>
						</view>
						<switch 
							:checked="item.currentValue"  
							@change="(e) => handleSwitchChange(e, item)"  
							class="oled-switch" 
							color="#00ff00"
						/>
					</view>
				</view>
			</view>
		</view>

		<view class="tabbar-placeholder"></view>
	</view>
</template>

<script setup>
import {
  ref,
  onMounted,
  onUnmounted
} from 'vue';
import {
  mqttClient,
  connectStatus,
  mqtt_connect,
  mqtt_disconnect,
  mqtt_publish,
  Stm32Status
} from '../../utils/mqtt_util.js';
import '../index/index.scss';
import config from '../../page_config/config.js';
import historyManager from '../../utils/historyManager.js';
import alarmManager from '../../utils/alarmManager.js';
import ZuiKnobBasic from '../../uni_modules/zui-knob-basic/components/zui-knob-basic/zui-knob-basic.vue';

// 设备数据
const dataItems = ref(config.devices);    // 数值监控设备
const ctrlItems = ref(config.devices_ctrl); // 控制设备
const knobItems = ref(config.knobItems);  // 旋转按钮设备
const latestMqttData = ref({});           // 最新MQTT接收数据
const isProcessing = ref(false);          // 状态锁
const tempKnobValues = ref({});           // 临时旋钮值
const lastKnobValues = ref({});           // 上次旋钮值

// 页面挂载时连接MQTT
onMounted(() => {
  mqtt_connect().then(() => {
    mqttClient.value.on('message', (topic, message) => {
      handleMqttMessage(message.toString());
    });
  }).catch(err => {
    console.error('MQTT连接失败:', err);
  });
});

/**
 * 处理MQTT接收的消息
 * @param {string} message - MQTT消息字符串
 */
function handleMqttMessage(message) {
  try {
    const data = JSON.parse(message);
    latestMqttData.value = data;

    // 更新数值监控设备
    dataItems.value.forEach(item => {
      if (data.hasOwnProperty(item.key)) {
        item.value = data[item.key] + (item.unit || '');
      }
    });

    // 更新控制设备状态
    ctrlItems.value.forEach(item => {
      if (data.hasOwnProperty(item.key)) {
        item.value = Boolean(data[item.key]);
      }
    });

    // 保存历史数据和告警检查
    historyManager.saveHistoryData(data);
    alarmManager.checkAndAlarm(data);
  } catch (e) {
    console.error('解析MQTT消息失败:', e);
  }
}

/**
 * 获取控制设备的真实状态
 * @param {object} item - 控制设备项
 * @returns {boolean} - 设备状态
 */
function getCtrlItemStatus(item) {
  // 单片机未连接 → 返回false
  if (!Stm32Status.value) {
    return false;
  }
  // 有item.key对应字段 → 返回字段布尔值
  if (latestMqttData.value.hasOwnProperty(item.key)) {
    return Boolean(latestMqttData.value[item.key]);
  }
  // 无字段/字段值无效 → 返回false
  return false;
}

/**
 * 切换开关状态并发送MQTT消息
 * @param {object} item - 控制设备项
 */
function toggleSwitch(item) {
  if (!connectStatus.value) {
    uni.showToast({
      title: 'MQTT未连接',
      icon: 'none'
    });
    return;
  }

  // 发送MQTT控制消息
  if (item.mqttConfig) {
    const sendMessage = item.currentValue ? item.mqttConfig.onMessage : item.mqttConfig.offMessage;
    mqtt_publish({
      message: sendMessage
    });
  }
}

/**
 * 处理开关变化事件
 * @param {Event} e - 事件对象
 * @param {object} item - 控制设备项
 */
function handleSwitchChange(e, item) {
  item.currentValue = e.detail.value;
  toggleSwitch(item);
}

/**
 * 处理旋钮值更新（实时）
 * @param {object} item - 旋钮设备项
 * @param {number} val - 更新的值
 */
function handleKnobUpdate(item, val) {
  item.value = val;
  tempKnobValues.value[item.label] = val;
}

/**
 * 处理旋钮值改变（发送MQTT消息）
 * @param {object} item - 旋钮设备项
 */
function handleKnobChange(item) {
  if (!connectStatus.value) {
    uni.showToast({
      title: 'MQTT未连接',
      icon: 'none'
    });
    return;
  }

  const currentValue = item.value;
  // 仅当值变化时发送消息
  if (lastKnobValues.value[item.label] !== currentValue) {
    lastKnobValues.value[item.label] = currentValue;

    // 发送MQTT消息
    if (item.mqttConfig?.Message) {
      const sendMessage = `${item.mqttConfig.Message}:${currentValue}`;
      mqtt_publish({
        message: sendMessage
      });
    }
  }
}

/**
 * 检查数据项是否有阈值配置
 * @param {object} item - 数据项
 * @returns {boolean} - 是否有阈值配置
 */
function hasThresholdConfig(item) {
  return item.thresholdMin !== undefined && item.thresholdMax !== undefined;
}

/**
 * 计算数据在阈值范围内的进度百分比
 * @param {object} item - 数据项
 * @returns {number} - 进度百分比
 */
function getDataProgress(item) {
  // 从值字符串中提取数值
  const currentValue = parseFloat(item.value.toString().replace(/[^\d.]/g, '')) || 0;
  const minThreshold = item.thresholdMin || 0;
  const maxThreshold = item.thresholdMax || 100;
  
  // 计算在阈值范围内的百分比
  if (maxThreshold <= minThreshold) return 0;
  
  const progress = ((currentValue - minThreshold) / (maxThreshold - minThreshold)) * 100;
  
  // 限制在 0-100% 范围内
  return Math.max(0, Math.min(100, progress));
}

/**
 * 获取进度条状态样式类
 * @param {object} item - 数据项
 * @returns {string} - CSS类名
 */
function getProgressStatusClass(item) {
  const currentValue = parseFloat(item.value.toString().replace(/[^\d.]/g, '')) || 0;
  const minThreshold = item.thresholdMin || 0;
  const maxThreshold = item.thresholdMax || 100;

  if (currentValue < minThreshold) {
    return 'oled-progress-low';
  } else if (currentValue > maxThreshold) {
    return 'oled-progress-high';
  } else {
    return 'oled-progress-normal';
  }
}

// 页面卸载时断开MQTT连接
onUnmounted(() => {
  mqtt_disconnect();
});
</script>