<template>
	<view class="content">
		<!-- 折线示例1-开始 -->
		<view class="chart-con gap">
			<view class="chart-wrap">
				<view class="title">
					<view class="ver-line"></view>
					<view class="title-desc">收益分析<text class="t-sm">(示例1)</text></view>
				</view>
				<view class="line-chart-con">
					<l-echart class="line-chart" ref="lineChart"></l-echart>
				</view>
			</view>
		</view>
		<!-- 折线示例1-结束 -->
		
		<!-- 折线示例2-开始 -->
		<view class="chart-con gap">
			<view class="chart-wrap">
				<view class="title">
					<view class="ver-line"></view>
					<view class="title-desc">收益分析<text class="t-sm">(示例2多线)</text></view>
				</view>
				<view class="line-chart-con">
					<l-echart class="line-chart" ref="lineChart2"></l-echart>
				</view>
			</view>
		</view>
		<!-- 折线示例2-结束 -->
		
	</view>
</template>

<script>
	import * as echarts from 'echarts/core'; 
	import {
		LineChart
	} from 'echarts/charts';
	import {
		TooltipComponent,
		GridComponent,
	} from 'echarts/components';
	import {
		CanvasRenderer
	} from 'echarts/renderers';
	// 注册必须的组件
	echarts.use([
		TooltipComponent,
		GridComponent,
		LineChart,
		CanvasRenderer
	]);
	export default {
		data() {
			return {
				
			}
		},
		mounted() {
			//加载折线图数据
			this.loadLineData();
			this.loadLineData2();
		},
		methods: {
			
			//加载折线图数据
			loadLineData() {
				//这里请求服务器拿到数据
				let res = {
					//x轴数据
					xData: ['6/11', '6/12', '6/13', '6/14', '6/15', '6/16', '6/17'],
					//y轴数据
					yData: [50, 120, 150, 220, 180, 300, 130]
				}
				//这里option配置参考文档：https://echarts.apache.org/zh/option.html
				let option = {
					xAxis: {
						type: 'category',
						// x轴数据文字颜色
						axisLabel: {
							color: '#a7a7a7'
						},
						// x轴那天坐标轴线的颜色
						axisLine: {
							lineStyle: {
								color: '#f1f1f1',
							}
						},
						//x轴上面刻度线隐藏
						axisTick: {
							show: false,
						},
						//这里是x轴数据
						data: res.xData
					},
					//设置网格
					grid: {
						top: 40,
						bottom: 30,
					},
					//y轴设置
					yAxis: {
						type: 'value',
						//y轴标签文字颜色
						axisLabel: {
							color: '#a7a7a7'
						},
						// y轴分割线设置为虚线
						splitLine: {
							show: true,
							lineStyle: {
								type: 'dashed'
							}
						}
					},
					//设置提示为点击时
					tooltip: {
						trigger: 'axis',
						triggerOn: 'click',
						formatter: '{b} \n 数据: {c}'
					},
					//设置曲线的颜色
					color: ['#4e9d77'],
					series: [{
						//这里是数据
						data: res.yData,
						type: 'line',
						//设置为平滑，默认为折线
						smooth: true,
					}]
				};

				this.$refs.lineChart.init(echarts, chart => {
					chart.setOption(option);
				});
			},
			//加载折线图数据--示例2
			loadLineData2() {
				//这里请求服务器拿到数据
				let res = {
					//x轴数据
					xData: ['6/11', '6/12', '6/13', '6/14', '6/15', '6/16', '6/17'],
					//y轴数据
					yData: [50, 120, 150, 220, 180, 300, 130],
					yData1: [20, 820, 350, 120, 80, 30, 190],
					yData2: [120, 187, 950, 1120, 880, 330, 290],
				}
				//这里option配置参考文档：https://echarts.apache.org/zh/option.html
				let option = {
					xAxis: {
						type: 'category',
						// x轴数据文字颜色
						axisLabel: {
							color: '#a7a7a7'
						},
						// x轴那天坐标轴线的颜色
						axisLine: {
							lineStyle: {
								color: '#f1f1f1',
							}
						},
						//x轴上面刻度线隐藏
						axisTick: {
							show: false,
						},
						//这里是x轴数据
						data: res.xData
					},
					//设置网格
					grid: {
						top: 40,
						bottom: 30,
					},
					//y轴设置
					yAxis: {
						type: 'value',
						//y轴标签文字颜色
						axisLabel: {
							color: '#a7a7a7'
						},
						// y轴分割线设置为虚线
						splitLine: {
							show: true,
							lineStyle: {
								type: 'dashed'
							}
						}
					},
					//设置提示为点击时
					tooltip: {
						trigger: 'axis',
						triggerOn: 'click',
						formatter: '{b}:  \n {a0}: {c0} \n {a1}: {c1} \n {a2}: {c2}'
					},
					//设置曲线的颜色，这里是3条故这只3个颜色
					color: ['#4e9d77','#fac858', '#ee6666'],
					series: [{
						name:"指标1",
						//这里是数据
						data: res.yData,
						type: 'line',
						//设置为平滑，默认为折线
						smooth: true,
					},
					{
						name:"指标2",
						//这里是数据
						data: res.yData1,
						type: 'line',
						//设置为平滑，默认为折线
						smooth: true,
					},
					{
						name:"指标3",
						//这里是数据
						data: res.yData2,
						type: 'line',
						//设置为平滑，默认为折线
						smooth: true,
					}]
				};
			
				this.$refs.lineChart2.init(echarts, chart => {
					chart.setOption(option);
				});
			}
		},
	}
</script>

<style lang="scss" scoped>
	.content {
		width: 100%;
		min-height: 100vh;
		background-color: #f1f3f2;
		padding: 20rpx 0rpx 100rpx;
	}
	.t-sm{
		font-size: 22rpx;
		color: #6e6e6e;
		padding-left: 10rpx;
	}
	.chart-con {
		width: 100%;
		box-sizing: border-box;
		padding: 0rpx 28rpx;

		.chart-wrap {
			width: 100%;
			box-sizing: border-box;
			background-color: #ffffff;
			padding: 32rpx 0rpx;
			border-radius: 20rpx;

			.title {
				box-sizing: border-box;
				width: 100%;
				padding: 0rpx 28rpx;
				display: flex;
				flex-direction: row;
				justify-content: flex-start;
				align-items: center;
			}

			.ver-line {
				height: 30rpx;
				width: 8rpx;
				border-radius: 10rpx;
				background-color: #4e9d77;
			}

			.title-desc {
				font-size: 30rpx;
				color: #222222;
				margin-left: 22rpx;
				font-weight: bold;
			}

			
			.line-chart-con {
				width: 100%;
				box-sizing: border-box;
				padding: 0rpx 28rpx;

				.fun-tabs {
					margin-top: 42rpx;
					display: flex;
					flex-direction: row;
					justify-content: space-between;
					align-self: center;
					width: 100%;
					box-sizing: border-box;

					.tab-item {
						width: 200rpx;
						height: 120rpx;
						border-radius: 10rpx;
						padding-left: 20rpx;
						background: #ffffff;
						border: 1rpx solid #ececec;
						display: flex;
						flex-direction: column;
						justify-content: center;
						align-items: flex-start;
						box-sizing: border-box;

						.item-name {
							color: #6e6e6e;
							font-size: 20rpx;
						}

						.item-val {
							color: #222222;
							font-size: 24rpx;
							font-weight: bold;
							margin-top: 20rpx;
						}
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
					margin-top: 30rpx;
					height: 380rpx;
				}
			}
		}
	}


	.gap {
		margin-top: 30rpx;
	}
</style>
