<template>
	<view class="content">
		<!-- 饼环示例1-开始 -->
		<view class="chart-con">
			<view class="chart-wrap">
				<view class="title">
					<view class="ver-line"></view>
					<view class="title-desc">访问UV统计<text class="t-sm">(示例1)</text></view>
				</view>
				<view class="ring-chart">
					<l-echart ref="ringChart"></l-echart>
				</view>
			</view>
		</view>
		<!-- 饼环示例1-结束 -->
		<!-- 饼环示例2-开始 -->
		<view class="chart-con gap">
			<view class="chart-wrap">
				<view class="title">
					<view class="ver-line"></view>
					<view class="title-desc">访问UV统计<text class="t-sm">(示例2)</text></view>
				</view>
				<view class="ring-chart">
					<l-echart ref="pieChartDemo2"></l-echart>
				</view>
			</view>
		</view>
		<!-- 饼环示例2-结束 -->
	</view>
</template>

<script>
	import * as echarts from 'echarts/core';
	import {
		PieChart
	} from 'echarts/charts';
	import {
		GridComponent
	} from 'echarts/components';
	import {
		CanvasRenderer
	} from 'echarts/renderers';
	// 注册必须的组件
	echarts.use([
		GridComponent,
		PieChart,
		CanvasRenderer
	]);
	export default {
		data() {
			return {
				// 访问来源统计数据
				accessSourceData: [
					{ name: '手机APP', value: 2000 },
					{ name: '小程序', value: 1500 },
					{ name: 'H5网页', value: 900 },
					{ name: 'PC端', value: 300 },
					{ name: '其它', value: 2001 }
				]
			}
		}, 
		mounted() {
			//加载饼环数据--示例1
			this.loadRingData();
			//加载饼环数据--示例2
			this.loadPieChartDemo2();
		},
		methods: {

			// 加载饼环数据（示例1）
			loadRingData() {
				// 使用预定义的数据
				const data = this.accessSourceData;

				//这里option配置参考文档：https://echarts.apache.org/zh/option.html
				this.$refs.ringChart.init(echarts, chart => {
					chart.setOption({
						series: [{
							type: 'pie',
							// selectedMode: 'single',
							// selectedOffset: 0,
							// hoverAnimation: false,
							radius: [50, 80],
						left: 'center',
						width: '98%',
							itemStyle: {
								borderColor: '#fff',
								borderWidth: 0
							},
							//格式化label显示
							label: {
								alignTo: 'edge',
								formatter: '{name|{b}}\n{time|{c} 人}',
								minMargin: 5,
								edgeDistance: 20,
								lineHeight: 24,
								rich: {
									time: {
										fontSize: 10,
										color: '#999'
									}
								}
							},
							//指向label的线设置
							labelLine: {
								length: 15,
								length2: 0,
								maxSurfaceAngle: 80
							},
							//布局样式
							labelLayout: function(params) {
								const isLeft = params.labelRect.x < chart.getWidth() / 2;
								const points = params.labelLinePoints;

								points[2][0] = isLeft ?
									params.labelRect.x :
									params.labelRect.x + params.labelRect.width;
								return {
									labelLinePoints: points
								};
							},

							//数据
							data: data
						}]
					});

					//监听图标项点击事件
					chart.on("click", function(e) {
						for (var i = 0; i < data.length; i++) {
							if (i != e.dataIndex) {
								chart.dispatchAction({
									type: 'downplay', //取消高亮显示;
									seriesIndex: 0,
									dataIndex: i
								});
							} else {
								chart.dispatchAction({
									type: 'highlight', //突出高亮显示;
									seriesIndex: 0,
									dataIndex: i
								});
							}
						}
					});
				});
			},
			// 加载饼环数据（示例2）
			loadPieChartDemo2() {
				// 使用预定义的数据
				const data = this.accessSourceData;

				//这里option配置参考文档：https://echarts.apache.org/zh/option.html
				this.$refs.pieChartDemo2.init(echarts, chart => {
					chart.setOption({
						series: [{
							silent:true,
							name: '访问来源',
							type: 'pie',
							radius: '70%',
							data: data,
							//格式化label显示
							label: {
								alignTo: 'edge',
								formatter: '{name|{b}}\n{time|{c} 人}',
								minMargin: 5,
								edgeDistance: 20,
								lineHeight: 24,
								rich: {
									time: {
										fontSize: 10,
										color: '#999'
									}
								}
							},
							//指向label的线设置
							labelLine: {
								length: 15,
								length2: 0,
								maxSurfaceAngle: 80
							},
							//布局样式
							labelLayout: function(params) {
								const isLeft = params.labelRect.x < chart.getWidth() / 2;
								const points = params.labelLinePoints;
							
								points[2][0] = isLeft ?
									params.labelRect.x :
									params.labelRect.x + params.labelRect.width;
								return {
									labelLinePoints: points
								};
							},
							emphasis: {
								itemStyle: {
									shadowBlur: 10,
									shadowOffsetX: 0,
									shadowColor: 'rgba(0, 0, 0, 0.5)'
								}
							}
						}]
					});

				});
			},
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

	.t-sm {
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

			.ring-chart {
				height: 400rpx;
				width: 100%;
				box-sizing: border-box;
				padding: 0rpx 28rpx;
				margin-top: 32rpx;
			}
		}
	}


	.gap {
		margin-top: 30rpx;
	}
</style>