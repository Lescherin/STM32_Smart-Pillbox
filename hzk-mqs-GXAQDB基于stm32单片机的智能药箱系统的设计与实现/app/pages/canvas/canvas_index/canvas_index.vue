<template>
	<view class="content">
		<view class="t-items">
			<view class="t-item" @click="toLeapChart('../canvas_pieChart/canvas_pieChart')">饼图</view>
			<view class="t-item" @click="toLeapChart('../canvas_lineChart/canvas_lineChart')">折线图</view>
			<view class="t-item" @click="toLeapChart('../canvas_barChart/canvas_barChart')">柱状图</view>
		</view>
		<!-- 折线示例-开始 -->
		<view class="chart-con gap">
			<view class="chart-wrap">
				<view class="title">
					<view class="ver-line"></view>
					<view class="title-desc">
						传感器数据
						<!-- 阈值显示 -->
						<text class="t-sm" v-if="currentThreshold">
							(阈值范围: {{ currentThreshold.min }} - {{ currentThreshold.max }})
						</text>
					</view>
				</view>
				<view class="line-chart-con">
					<!-- 传感器选择标签容器 - 支持横向滚动 -->
					<view class="tabs-container">
						<view class="fun-tabs">
							<view 
								v-for="(device, index) in devices" 
								:key="index"
								:class="{'tab-item':true, 'selected': selectedDeviceIndex === index + 1}" 
								@click="changeDevice(index + 1)"
							>
								<view class="item-name">{{ device.label || '未知传感器' }}</view>
								<view class="item-val">{{ device.key || '无标识' }}</view>
							</view>
						</view>
					</view>
					<l-echart class="line-chart" ref="lineChart"></l-echart>
					
					<!-- 图例说明 -->
					<view class="legend-container" v-if="currentThreshold">
						<view class="legend-item">
							<view class="legend-color normal-color"></view>
							<view class="legend-text">阈值范围内数据</view>
						</view>
						<view class="legend-item">
							<view class="legend-color abnormal-color"></view>
							<view class="legend-text">超出阈值数据</view>
						</view>
						<view class="legend-item">
							<view class="legend-color threshold-line"></view>
							<view class="legend-text">阈值参考线</view>
						</view>
					</view>
					
					<!-- NaN提示 -->
					<view class="nan-alert" v-if="hasNaN">
						数据中为非数值类型，已自动过滤
					</view>
				</view>
			</view>
		</view>
		<!-- 折线示例-结束 -->
	</view>
</template>

<script>
	import * as echarts from 'echarts/core';
	import { LineChart } from 'echarts/charts';
	import { TooltipComponent, GridComponent, TitleComponent, MarkLineComponent } from 'echarts/components';
	import { CanvasRenderer } from 'echarts/renderers';
	// 导入数据源和阈值管理器
	import config from '../../../page_config/config.js';
	import historyManager from '../../../utils/historyManager.js';
	import thresholdManager from '../../../utils/thresholdManager.js';
	
	// 注册必须的组件
	echarts.use([
		TooltipComponent,
		GridComponent,
		TitleComponent,
		MarkLineComponent,
		LineChart,
		CanvasRenderer
	]);
	
	// 数值格式化函数
	function formatLargeNumber(value) {
		if (value >= 1000000) {
			return (value / 1000000).toFixed(1) + 'm';
		} else if (value >= 1000) {
			return (value / 1000).toFixed(1) + 'k';
		}
		return value;
	}
	
	export default {
		data() {
			return {
				selectedDeviceIndex: 0,
				devices: [],
				hasNaN: false,
				currentThreshold: null,
				chartInstance: null
			}
		},
		mounted() {
			// 禁止页面上下滑动
			uni.pageScrollTo({
				scrollTop: 0,
				duration: 0
			});
			
			// 初始化阈值管理器
			thresholdManager.initThresholds();
			
			this.$nextTick(() => {
				this.loadDevices();
			})
		},
		methods: {
			// 跳转其他图表页面
			toLeapChart(path){
				uni.navigateTo({
					url: path
				})
			},
			
			// 加载传感器列表
			loadDevices() {
				this.devices = Array.isArray(config.devices) ? config.devices : [];
				
				// 如果有传感器，默认选中第一个
				if (this.devices.length > 0) {
					this.selectedDeviceIndex = 1;
					this.loadLineData();
				}
			},
			
			// 切换传感器
			changeDevice(index) {
				this.selectedDeviceIndex = index;
				this.loadLineData();
			},
			
			// 加载折线图数据
			loadLineData() {
				// 重置状态
				this.hasNaN = false;
				this.currentThreshold = null;
				
				// 获取当前选中的传感器
				const selectedDevice = this.devices[this.selectedDeviceIndex - 1];
				if (!selectedDevice || !selectedDevice.key) {
					return;
				}
				
				try {
					// 获取当前设备的阈值
					const thresholds = thresholdManager.getThresholds();
					this.currentThreshold = thresholds[selectedDevice.key] || {
						min: Number(selectedDevice.thresholdMin) || 0,
						max: Number(selectedDevice.thresholdMax) || 0
					};
					
					// 验证阈值是否有效
					if (isNaN(this.currentThreshold.min) || isNaN(this.currentThreshold.max)) {
						this.currentThreshold = { min: 0, max: 0 };
					}
					
					// 从历史数据管理器获取数据
					const results = historyManager.queryHistoryData({
						field: selectedDevice.key
					});
					
					if (results.length === 0) {
						this.showEmptyChart();
						return;
					}
					
					// 按时间排序
					results.sort((a, b) => new Date(a.datetime) - new Date(b.datetime));
					
					// 处理数据，过滤NaN值并标记超出阈值的数据
					let xData = [];
					let normalData = [];  // 正常范围内的数据点
					let abnormalData = []; // 超出阈值的数据点
					
					// 日期时间格式化函数
					const formatDateTime = (dateObj) => {
						return `${dateObj.getMonth() + 1}-${dateObj.getDate()} ${dateObj.getHours().toString().padStart(2, '0')}:${dateObj.getMinutes().toString().padStart(2, '0')}`;
					};
					
					results.forEach(item => {
						const value = item[selectedDevice.key];
						if (value !== null && value !== undefined && !isNaN(Number(value))) {
							const date = new Date(item.datetime || item.timestamp);
							const formattedTime = formatDateTime(date);
							const numValue = Number(value);
							
							xData.push(formattedTime);
							
							// 检查是否超出阈值范围
							if (numValue < this.currentThreshold.min || numValue > this.currentThreshold.max) {
								abnormalData.push(numValue);
							} else {
								normalData.push(numValue);
							}
						} else {
							this.hasNaN = true;
						}
					});
					
					// 如果所有数据都无效
					if (xData.length === 0) {
						this.showEmptyChart('无有效数据');
						return;
					}
					
					// 设置图表选项
					let option = {
						// 调整网格，增加底部空间以容纳更长的日期时间标签
						grid: {
							top: '10%',
							right: '8%',
							left: '8%',
							bottom: '30%'  // 增加底部空间，防止日期时间标签被截断
						},
						xAxis: {
							type: 'category',
							axisLabel: {
								color: '#a7a7a7',
								rotate: 60,  // 增大旋转角度，避免日期时间标签重叠
								fontSize: 8  // 减小字体大小
							},
							axisLine: {
								lineStyle: {
									color: '#f1f1f1',
								}
							},
							axisTick: {
								show: false,
							},
							data: xData
						},
						yAxis: {
							type: 'value',
							axisLabel: {
								color: '#a7a7a7',
								fontSize: 8,  // 减小字体大小
								formatter: function(value) {
									return formatLargeNumber(value);
								}
							},
							splitLine: {
								show: true,
								lineStyle: {
									type: 'dashed'
								}
							}
						},
						series: [
							// 阈值参考线系列
							{
								type: 'line',
								data: [],
								markLine: {
									symbol: 'none',
									animation: true,
									lineStyle: {
										type: 'dashed',
										width: 2,
										color: '#ff6b6b',
										opacity: 1
									},
									data: [
										{
											name: '最小值阈值',
											yAxis: this.currentThreshold.min,
											label: {
												show: false,  // 隐藏最小值阈值标签
												position: 'end',
												label: {
													color: '#ff6b6b',
													fontSize: 7
												},
												lineStyle: {
													type: 'dashed',
													color: '#ff6b6b'
												}
											}
										},
										{
											name: '最大值阈值',
											yAxis: this.currentThreshold.max,
											label: {
												show: false,  // 隐藏最大值阈值标签
												position: 'end',
												label: {
													color: '#ff6b6b',
													fontSize: 7
												},
												lineStyle: {
													type: 'dashed',
													color: '#ff6b6b'
												}
											}
										}
									]
								},
								z: 5
							},
							// 正常数据系列
							{
								name: '正常数据',
								data: normalData,
								type: 'line',
								smooth: true,
								showSymbol: true,
								symbolSize: 5,  // 减小符号大小
								lineStyle: {
									width: 2,
									color: '#4e9d77'
								},
								itemStyle: {
									color: '#4e9d77'
								},
								z: 1
							},
							// 异常数据系列（超出阈值）
							{
								name: '异常数据',
								data: abnormalData,
								type: 'line',
								smooth: true,
								showSymbol: true,
								symbolSize: 5,  // 减小符号大小
								lineStyle: {
									width: 2,
									color: '#ff6b6b',
									type: 'solid'
								},
								itemStyle: {
									color: '#ff6b6b',
									borderWidth: 2
								},
								z: 10
							}
						],
						tooltip: {
							trigger: 'axis',
							formatter: `${selectedDevice.label || '传感器'}: {b} \n 值: {c}`,
							label: {
								fontSize: 10  // 减小提示框字体大小
							}
						}
					};

					// 初始化或更新图表
					this.$refs.lineChart.init(echarts, chart => {
						this.chartInstance = chart;
						chart.setOption(option, true);
					});
					
				} catch (error) {
					uni.showToast({ title: '获取数据失败', icon: 'none' });
					this.showEmptyChart('获取数据失败');
				}
			},
			
			// 显示空图表
			showEmptyChart(message = '无数据') {
				this.$refs.lineChart.init(echarts, chart => {
					chart.setOption({
						title: {
							text: message,
							left: 'center',
							top: 'center',
							label: {
								color: '#999',
								fontSize: 12  // 调整空图表提示文字大小
							}
						},
						grid: {
							top: '10%',
							right: '8%',
							left: '8%',
							bottom: '30%'  // 保持一致的底部空间
						},
						xAxis: {
							type: 'category',
							data: []
						},
						yAxis: {
							type: 'value',
							axisLabel: {
								formatter: function(value) {
									return formatLargeNumber(value);
								}
							}
						},
						series: [{
							type: 'line',
							data: []
						}]
					});
				});
			}
		},
	}
</script>

<style lang="scss" scoped>
	/* 全局样式适配手机 */
	::v-deep body,
	::v-deep html {
		overflow: hidden !important;
		touch-action: none !important;
		height: 100% !important;
	}

	.content {
		width: 100%;
		height: 100vh;
		overflow: hidden;
		background-color: #f1f3f2;
		padding: 16rpx 0;
		box-sizing: border-box;
	}

	.t-sm {
		font-size: 20rpx;
		color: #6e6e6e;
		padding-left: 8rpx;
	}

	.chart-con {
		width: 100%;
		box-sizing: border-box;
		padding: 0 16rpx;
		flex-grow: 1;
	}

	.chart-wrap {
		width: 100%;
		box-sizing: border-box;
		background-color: #ffffff;
		padding: 24rpx 0;
		border-radius: 16rpx;
		height: 100%;
		display: flex;
		flex-direction: column;
	}

	.title {
		box-sizing: border-box;
		width: 100%;
		padding: 0 24rpx 16rpx;
		display: flex;
		flex-direction: row;
		justify-content: flex-start;
		align-items: center;
	}

	.ver-line {
		height: 24rpx;
		width: 6rpx;
		border-radius: 8rpx;
		background-color: #4e9d77;
	}

	.title-desc {
		font-size: 26rpx;
		color: #222222;
		margin-left: 18rpx;
		font-weight: bold;
	}

	.line-chart-con {
		width: 100%;
		box-sizing: border-box;
		padding: 0 16rpx;
		position: relative;
		flex-grow: 1;
		display: flex;
		flex-direction: column;
	}

	.tabs-container {
		width: 100%;
		overflow-x: auto;
		overflow-y: hidden;
		white-space: nowrap;
		padding-bottom: 8rpx;
		scrollbar-width: thin;
		scrollbar-color: #ddd #f5f5f5;

		&::-webkit-scrollbar {
			height: 4rpx;
		}
		&::-webkit-scrollbar-track {
			background: #f5f5f5;
			border-radius: 2rpx;
		}
		&::-webkit-scrollbar-thumb {
			background: #ddd;
			border-radius: 2rpx;
		}
		&::-webkit-scrollbar-thumb:hover {
			background: #bbb;
		}
	}

	.fun-tabs {
		margin-top: 8rpx;
		display: inline-flex;
		flex-direction: row;
		justify-content: flex-start;
		align-items: center;
		box-sizing: border-box;

		.tab-item {
			width: 160rpx;
			height: 100rpx;
			border-radius: 8rpx;
			padding-left: 16rpx;
			background: #ffffff;
			border: 1rpx solid #ececec;
			display: flex;
			flex-direction: column;
			justify-content: center;
			align-items: flex-start;
			box-sizing: border-box;
			margin-right: 12rpx;
			flex-shrink: 0;

			.item-name {
				color: #6e6e6e;
				font-size: 18rpx;
			}

			.item-val {
				color: #222222;
				font-size: 22rpx;
				font-weight: bold;
				margin-top: 16rpx;
			}
		}

		.tab-item:last-child {
			margin-right: 0;
		}

		.selected {
			background: #edf5f1 !important;
			border: 1rpx solid #4e9d77 !important;

			.item-name {
				color: #4e9d77 !important;
			}

			.item-val {
				color: #4e9d77 !important;
			}
		}
	}

	.line-chart {
		margin-top: 16rpx;
		flex-grow: 1;
		min-height: 400rpx;
	}
	
	// 图例样式
	.legend-container {
		display: flex;
		flex-direction: row;
		justify-content: center;
		align-items: center;
		margin-top: 12rpx;
		padding: 8rpx 0;
		flex-wrap: wrap;
		gap: 16rpx 24rpx;
	}
	
	.legend-item {
		display: flex;
		flex-direction: row;
		align-items: center;
	}
	
	.legend-color {
		width: 20rpx;
		height: 5rpx;
		margin-right: 8rpx;
	}
	
	.normal-color {
		background-color: #4e9d77;
	}
	
	.abnormal-color {
		background-color: #ff6b6b;
		height: 5rpx;
	}
	
	.threshold-line {
		background: repeating-linear-gradient(
			to right,
			#ff6b6b,
			#ff6b6b 3rpx,
			transparent 3rpx,
			transparent 6rpx
		);
		height: 5rpx;
	}
	
	.legend-text {
		font-size: 18rpx;  /* 减小图例文字大小 */
		color: #6e6e6e;
	}
	
	.nan-alert {
		color: #ff6b6b;
		font-size: 18rpx;  /* 减小提示文字大小 */
		margin-top: 8rpx;
		padding: 8rpx;
		background-color: #fff5f5;
		border-radius: 5rpx;
	}

	.gap {
		margin-top: 16rpx;
	}

	.t-items {
		width: calc(100% - 32rpx);
		box-sizing: border-box;
		display: flex;
		flex-direction: row;
		justify-content: space-evenly;
		align-items: center;
		background: #fff;
		margin: 8rpx auto;
		border-radius: 12rpx;

		.t-item {
			font-size: 24rpx;
			color: #222222;
			height: 70rpx;
			line-height: 70rpx;
			font-weight: bold;
			padding: 0 16rpx;
		}
	}
</style>
