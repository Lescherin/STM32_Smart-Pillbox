export default {
	index_config:{
		show_control_btn:true // 显示主页 手/自动控制按钮
	},
	
	// MQTT连接配置
	mqtt: {
		url: '120.27.235.176:8083',
		topic: 'hznyxxt2025114', // 接收数据的主题
		topic_ctrl: 'hznyxxt2025114_ctrl', // 发送控制指令的主题
		qos: 0, // QoS等级（0/1/2）
		clientId: 'uniapp-client-' + Math.random().toString(36).substring(2, 10), // 随机客户端ID
		username: 'root', // 用户名
		password: 'root', // 密码
		keepalive: 60, // 60秒心跳
		clean: true, // 断开连接后清除会话
		reconnectPeriod: 3000, // 3秒后自动重连
		connectTimeout: 10000, // 10秒连接超时
		stm32_Olnine_time:3000, // 此处在判定单片机多久未发送mqtt视为离线状态 单位毫秒
		controlInterval: 1000 // 控制指令间隔时间（毫秒）
	},
	
	// 设备联动配置：定义监测设备与控制设备的联动关系
	deviceLinkageConfig: {
		'temp': { // 温度设备
			'over': [ // 温度过高时触发的控制设备
				{ key: 'beep', action: 'on' }// 开启舵机
				// { key: 'jdq', action: 'off' }  // 关闭浇水
			],
			'under': [ // 温度过低时触发的控制设备
				{ key: 'beep', action: 'on' } // 关闭舵机
				// { key: 'jdq', action: 'on' }   // 开启浇水（如果需要）
			]
		},
		'humi': { // 湿度设备
			'over': [ // 温度过高时触发的控制设备
				{ key: 'beep', action: 'on' }// 开启舵机
				// { key: 'jdq', action: 'off' }  // 关闭浇水
			],
			'under': [ // 温度过低时触发的控制设备
				{ key: 'beep', action: 'on' } // 关闭舵机
				// { key: 'jdq', action: 'on' }   // 开启浇水（如果需要）
			]
		},
		'weight': { // 光照设备
			'over': [ // 温度过高时触发的控制设备
				// { key: 'beep', action: 'on' }, // 开启舵机
				// { key: 'jdq', action: 'off' }  // 关闭浇水
			],
			'under': [ // 温度过低时触发的控制设备
				{ key: 'weight', action: 'on' }// 关闭舵机
				// { key: 'jdq', action: 'on' }   // 开启浇水（如果需要）
			]                  // 光照不足时不触发控制设备
		}
		// ,
		// 'tcrt5000': { // 气体浓度设备
		// 	'over': [ // 气体浓度过高时触发的控制设备
		// 		// { key: 'sg90', action: 'on' }   // 开启舵机（可用于通风）
		// 	],
		// 	'under': []                      // 气体浓度过低时不触发控制设备
		// }
		// 'tds': { // 电导率设备
		// 	'over': [ // 电导率过高时触发的控制设备
		// 		{ key: 'jdq', action: 'off' }   // 关闭浇水
		// 	],
		// 	'under': [ // 电导率过低时触发的控制设备
		// 		{ key: 'jdq', action: 'on' }    // 开启浇水
		// 	]
		// }
	},

	// 按钮设备数据组，用来添加按钮card的 这个数组不会在历史数据以及阈值设置中循环出来
	devices_ctrl: [
		// {
		// 	label: '手动   /  自动',
		// 	icon: '/static/logo.png',
		// 	key: 'AUTO_STATE',
		// 	type: 'switch', // 开关类型
		// 	value: false, // 初始状态
		// 	// MQTT配置
		// 	mqttConfig: {
		// 		onMessage: 'auto=1', // 开启时发送的消息
		// 		offMessage: 'auto=0' // 关闭时发送的消息
		// 	}
		// }
		// ,{
		// 	label: '灯光控制',
		// 	icon: '/static/monkey.png',
		// 	key: 'LED_STATE',
		// 	type: 'switch', // 开关类型
		// 	value: false, // 初始状态
		// 	// MQTT配置
		// 	mqttConfig: {
		// 		onMessage: 'led=1', // 开启时发送的消息
		// 		offMessage: 'led=0' // 关闭时发送的消息
		// 	}
		// }
		{
			label: '舵机控制',
			icon: '/static/logo.png',
			key: 'sg90',
			type: 'switch', // 开关类型
			value: false, // 初始状态
			// MQTT配置
			mqttConfig: {
				onMessage: 'sg90:on', // 开启时发送的消息
				offMessage: 'sg90:off' // 关闭时发送的消息
			}
		},
		{
			label: '报警控制',
			icon: '/static/logo.png',
			key: 'beep',
			type: 'switch', // 开关类型
			value: false, // 初始状态
			// MQTT配置
			mqttConfig: {
				onMessage: 'beep:on', // 开启时发送的消息
				offMessage: 'beep:off' // 关闭时发送的消息
			}
		},

		// 可以添加更多按钮的数据项 
	],

	// 设备数据数组：icon字段改为static文件夹中图片的路径  这里面的标签卡会在历史数据和阈值设置中循环出来
	devices: [{
			label: '空气温度',
			icon: '/static/logo.png',
			value: '--',
			key: 'temp',
			unit:'℃',
			thresholdMax: 40, // 默认最大阈值 
			thresholdMin: 0, // 默认最小阈值
			alarmMessage: { // 报警时发送的消息
				over: 'beep:on',
				under: 'beep:on'
			}
		},
		{
			label: '空气湿度',
			icon: '/static/logo.png',
			value: '--',
			key: 'humi',
			unit:'%',
			thresholdMax: 10000,
			thresholdMin: 0,
			alarmMessage: {
				over: 'beep:on',
				under: 'beep:on'
			}
		},{
			label: '重量',
			icon: '/static/logo.png',
			value: '--',
			key: 'weight',
			unit:'g',
			thresholdMax: 100000,
			thresholdMin: 10,
			alarmMessage: {
				over: 'weight:off',
				under: 'weight:on'
			}
		},{
			label: '门状态检测',
			icon: '/static/logo.png',
			value: '--',
			key: 'tcrt5000',
			unit:'',
			thresholdMax: 100000, // 默认最大阈值 
			thresholdMin: 0, // 默认最小阈值
			alarmMessage: { // 报警时发送的消息
				over: '',
				under: ''
			}
		}

	],
	
	knobItems: [
	 //  {
	 //    icon: '/static/monkey.png', // 旋钮上面的图标
	 //    label: '音量调节',          // 显示名称
	 //    value: 0,                  // 初始值
	 //    range: [0, 100, 1],         // [最小值, 最大值, 步长]
	 //    unit: '%',                  // 单位
	 //    angleRange: [0, 360],       // 旋转角度范围（0-360度）
	 //    theme: {
	 //      base: '/static/cycle_btn_bkp.png',  // 旋钮底座图标
	 //      handle: '/static/btn_pic.png' // 旋钮手柄图标
	 //    },
		// // MQTT配置
		// mqttConfig: {
		// 	Message: 'ff', // 旋转时发送的key，旋转发送的值为旋转的值
		// }
	 //  },
	 //  {
	 //    icon: '/static/monkey.png',
	 //    label: '亮度调节',
	 //    value: 0,
	 //    range: [0, 100, 1],
	 //    unit: '%',
	 //    angleRange: [0, 360],
	 //    theme: {
	 //      base: '/static/cycle_btn_bkp.png',
	 //      handle: '/static/btn_pic.png'
	 //    },
		// // MQTT配置
		// mqttConfig: {
		// 	Message: 'ss', 
		// }
	 //  }
	]
}