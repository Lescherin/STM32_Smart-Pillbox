<template>
	<view class="map-container">
		<!-- 加载状态 -->
		<view v-if="loading" class="loading-overlay">
			<view class="loading-spinner"></view>
			<text>加载地图中...</text>
		</view>

		<!-- 地图组件 -->
		<map 
			class="map" 
			:latitude="currentLatitude" 
			:longitude="currentLongitude" 
			:markers="markers"
			:polyline="polyline" 
			:scale="16" 
			show-location 
			@loaded="onMapLoaded"
			@error="handleMapError"
		></map>

		<!-- 控制按钮区域 -->
		<view class="control-panel">
			<button 
				class="history-btn" 
				@click="toggleHistory" 
				:disabled="historyPoints.length < 2"
			>
				{{ showHistory ? '隐藏轨迹' : '显示轨迹' }}
			</button>
			<button 
				class="reset-btn" 
				@click="moveToOrigin" 
				:disabled="!hasLocation"
			>
				回到起点
			</button>
			<button 
				class="clear-btn" 
				@click="clearHistory" 
				:disabled="historyPoints.length === 0"
			>
				清除轨迹
			</button>
		</view>

		<!-- 错误提示 -->
		<view v-if="mapError" class="error-toast">
			{{ mapError }}
		</view>
	</view>
</template>

<script setup>
import { ref, onMounted, onUnmounted, watch } from 'vue';
import { mqttClient, connectStatus, mqtt_connect } from '../../utils/mqtt_util.js';
import historyManager from '../../utils/historyManager.js';
import '../map/map.scss';

// 地图核心数据
const currentLatitude = ref(39.90923);
const currentLongitude = ref(116.397428);
const hasLocation = ref(false);
const originLatitude = ref(0);
const originLongitude = ref(0);
const loading = ref(true);
const mapError = ref('');

// 标记点和轨迹线
const markers = ref([]);
const polyline = ref([]);

// 历史数据相关
const historyPoints = ref([]);
const showHistory = ref(false);
const MAX_HISTORY_POINTS = 1000; // 最大轨迹点数量

// 高德地图坐标转换工具（WGS84 -> GCJ-02）
function wgs84ToGcj02(lng, lat) {
	const pi = 3.1415926535897932384626;
	const a = 6378245.0;
	const ee = 0.00669342162296594323;

	if (outOfChina(lng, lat)) {
		return { lng, lat };
	}

	let dLat = transformLat(lng - 105.0, lat - 35.0);
	let dLng = transformLng(lng - 105.0, lat - 35.0);
	const radLat = lat / 180.0 * pi;
	let magic = Math.sin(radLat);
	magic = 1 - ee * magic * magic;
	const sqrtMagic = Math.sqrt(magic);
	dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
	dLng = (dLng * 180.0) / (a / sqrtMagic * Math.cos(radLat) * pi);
	return {
		lng: lng + dLng,
		lat: lat + dLat
	};
}

function outOfChina(lng, lat) {
	return (lng < 73.66 || lng > 135.05 || lat < 18.16 || lat > 53.55);
}

function transformLat(x, y) {
	let ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * Math.sqrt(Math.abs(x));
	ret += (20.0 * Math.sin(6.0 * x * Math.PI) + 20.0 * Math.sin(2.0 * x * Math.PI)) * 2.0 / 3.0;
	ret += (20.0 * Math.sin(y * Math.PI) + 40.0 * Math.sin(y / 3.0 * Math.PI)) * 2.0 / 3.0;
	ret += (160.0 * Math.sin(y / 12.0 * Math.PI) + 320 * Math.sin(y * Math.PI / 30.0)) * 2.0 / 3.0;
	return ret;
}

function transformLng(x, y) {
	let ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * Math.sqrt(Math.abs(x));
	ret += (20.0 * Math.sin(6.0 * x * Math.PI) + 20.0 * Math.sin(2.0 * x * Math.PI)) * 2.0 / 3.0;
	ret += (20.0 * Math.sin(x * Math.PI) + 40.0 * Math.sin(x / 3.0 * Math.PI)) * 2.0 / 3.0;
	ret += (150.0 * Math.sin(x / 12.0 * Math.PI) + 300.0 * Math.sin(x / 30.0 * Math.PI)) * 2.0 / 3.0;
	return ret;
}

// 经纬度精度处理
function formatCoord(value) {
	return Number(value.toFixed(6));
}

// 地图加载完成
function onMapLoaded() {
	loading.value = false;
	console.log('地图加载完成');
}

// 地图错误处理
function handleMapError(err) {
	mapError.value = `地图加载失败: ${err.detail.message}`;
	console.error('地图错误:', err);
	setTimeout(() => mapError.value = '', 5000);
}

// MQTT消息处理
function handleMqttMessage(topic, message) {
	try {
		const data = JSON.parse(message.toString());
		if (typeof data.latitude === 'number' && typeof data.longitude === 'number') {
			// 坐标合法性校验
			if (data.longitude < -180 || data.longitude > 180 || data.latitude < -90 || data.latitude > 90) {
				console.warn('经纬度超出合法范围:', data);
				return;
			}

			// 转换为高德坐标系并格式化
			const gcjCoord = wgs84ToGcj02(data.longitude, data.latitude);
			const amapLat = formatCoord(gcjCoord.lat);
			const amapLng = formatCoord(gcjCoord.lng);

			// 更新当前位置
			currentLatitude.value = amapLat;
			currentLongitude.value = amapLng;

			// 设置原点（首次定位）
			if (!hasLocation.value) {
				originLatitude.value = amapLat;
				originLongitude.value = amapLng;
				hasLocation.value = true;
			}

			// 更新标记点
			markers.value = [{
				id: 1,
				latitude: amapLat,
				longitude: amapLng,
				iconPath: '/static/marker.png',
				width: 40,
				height: 40,
				callout: {
					content: `当前位置: ${amapLat.toFixed(6)},${amapLng.toFixed(6)}`,
					color: '#ffffff',
					bgColor: '#007aff',
					padding: 8,
					borderRadius: 4,
					display: 'ALWAYS'
				},
				anchor: { x: 0.5, y: 0.5 }
			}];

			// 保存到历史记录
			historyManager.saveHistoryData({
				LOCATION: `${amapLat},${amapLng}`,
				timestamp: Date.now(),
				coordType: 'GCJ-02'
			});

			// 添加到历史轨迹点（控制最大数量）
			historyPoints.value.push({
				latitude: amapLat,
				longitude: amapLng,
				timestamp: Date.now()
			});
			if (historyPoints.value.length > MAX_HISTORY_POINTS) {
				historyPoints.value.shift(); // 移除最早的点
			}

			// 更新轨迹线
			if (showHistory.value) {
				updatePolyline();
			}
		} else {
			console.warn('MQTT消息经纬度格式不正确:', data);
		}
	} catch (e) {
		console.error('解析MQTT消息失败:', e);
	}
}

// 更新轨迹线
function updatePolyline() {
	if (historyPoints.value.length < 2) {
		polyline.value = [];
		return;
	}

	polyline.value = [{
		points: historyPoints.value.map(point => ({
			latitude: point.latitude,
			longitude: point.longitude
		})),
		color: "#007aff",
		width: 4,
		arrowLine: true,
		borderColor: '#ffffff',
		borderWidth: 1,
		dottedLine: false
	}];
}

// 回到原点
function moveToOrigin() {
	if (hasLocation.value) {
		currentLatitude.value = originLatitude.value;
		currentLongitude.value = originLongitude.value;
	}
}

// 切换历史轨迹显示
function toggleHistory() {
	showHistory.value = !showHistory.value;
	showHistory.value ? updatePolyline() : (polyline.value = []);
}

// 清除历史轨迹
function clearHistory() {
	if (confirm('确定要清除所有轨迹记录吗？')) {
		historyPoints.value = [];
		polyline.value = [];
		historyManager.clearHistoryData();
		showHistory.value = false;
	}
}

// 初始化
onMounted(async () => {
	// 连接MQTT
	if (!connectStatus.value) {
		try {
			await mqtt_connect();
		} catch (err) {
			console.error('连接MQTT失败:', err);
			mapError.value = 'MQTT连接失败，无法接收位置数据';
			setTimeout(() => mapError.value = '', 5000);
		}
	}

	// 加载历史数据
	const historyData = historyManager.queryHistoryData({ field: 'LOCATION' });
	if (historyData.length > 0) {
		historyPoints.value = historyData
			.map(item => {
				if (item.LOCATION) {
					const [lat, lng] = item.LOCATION.split(',').map(Number);
					return lat && lng ? {
						latitude: lat,
						longitude: lng,
						timestamp: item.timestamp
					} : null;
				}
				return null;
			})
			.filter(Boolean)
			.sort((a, b) => a.timestamp - b.timestamp); // 按时间排序

		// 限制历史点数量
		if (historyPoints.value.length > MAX_HISTORY_POINTS) {
			historyPoints.value = historyPoints.value.slice(-MAX_HISTORY_POINTS);
		}
	}

	// 监听MQTT消息
	if (mqttClient.value) {
		mqttClient.value.on('message', handleMqttMessage);
	} else {
		console.warn('MQTT客户端未初始化');
	}
});

// 清理资源
onUnmounted(() => {
	if (mqttClient.value) {
		mqttClient.value.off('message', handleMqttMessage);
	}
});

// 监听历史点变化，自动更新轨迹（如果显示中）
watch(historyPoints, () => {
	if (showHistory.value) {
		updatePolyline();
	}
});
</script>

<style scoped>
.map-container {
	position: relative;
	width: 100%;
	height: 100vh;
	overflow: hidden;
}

.map {
	width: 100%;
	height: 100%;
}

/* 加载状态样式 */
.loading-overlay {
	position: absolute;
	top: 0;
	left: 0;
	right: 0;
	bottom: 0;
	background-color: rgba(255, 255, 255, 0.8);
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	z-index: 100;
}

.loading-spinner {
	width: 60rpx;
	height: 60rpx;
	border: 6rpx solid #f3f3f3;
	border-top: 6rpx solid #007aff;
	border-radius: 50%;
	animation: spin 1s linear infinite;
	margin-bottom: 20rpx;
}

@keyframes spin {
	0% { transform: rotate(0deg); }
	100% { transform: rotate(360deg); }
}

/* 控制按钮样式 */
.control-panel {
	position: absolute;
	bottom: 30rpx;
	left: 50%;
	transform: translateX(-50%);
	display: flex;
	gap: 15rpx;
	z-index: 90;
}

.history-btn, .reset-btn, .clear-btn {
	padding: 15rpx 30rpx;
	border-radius: 50rpx;
	font-size: 28rpx;
	line-height: 1;
}

.history-btn {
	background-color: #007aff;
	color: white;
}

.reset-btn {
	background-color: #4cd964;
	color: white;
}

.clear-btn {
	background-color: #ff3b30;
	color: white;
}

/* 错误提示 */
.error-toast {
	position: absolute;
	top: 50rpx;
	left: 50%;
	transform: translateX(-50%);
	background-color: rgba(255, 59, 48, 0.9);
	color: white;
	padding: 15rpx 30rpx;
	border-radius: 10rpx;
	font-size: 28rpx;
	z-index: 100;
}
</style>