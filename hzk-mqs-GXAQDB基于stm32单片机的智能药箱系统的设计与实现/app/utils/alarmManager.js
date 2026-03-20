// utils/alarmManager.js
/*  报警管理工具  */
import thresholdManager from './thresholdManager';
import { mqtt_publish } from './mqtt_util';
import config from '../page_config/config.js';

export default {
  // 防止重复报警的计时器
  alarmTimers: {},
  // 当前活跃的报警记录
  activeAlarms: {},
  // 控制指令队列
  controlCommandQueue: [],
  // 是否正在处理队列
  isProcessingQueue: false,
  // 控制指令间隔时间
  controlInterval: config.mqtt?.controlInterval || 1000,
  
  /**
   * 向队列添加控制命令
   * @param {string} message - 要发送的MQTT消息
   * @param {string} deviceLabel - 设备标签（用于日志）
   */
  addControlCommand(message, deviceLabel) {
    this.controlCommandQueue.push({ message, deviceLabel });
    if (!this.isProcessingQueue) {
      this.processControlQueue();
    }
  },

  /**
   * 处理控制命令队列，间隔指定时间发送每条命令
   */
  processControlQueue() {
    if (this.controlCommandQueue.length === 0) {
      this.isProcessingQueue = false;
      return;
    }

    this.isProcessingQueue = true;
    const { message, deviceLabel } = this.controlCommandQueue.shift();
    
    // 发送控制命令
    console.log(`发送控制命令到${deviceLabel}:`, message);
    mqtt_publish(message);
    
    // 等待指定的间隔时间后处理下一条命令
    setTimeout(() => {
      this.processControlQueue();
    }, this.controlInterval);
  },
  /**
   * 检查数据并触发相应的报警
   * @param {Object} data - MQTT接收的数据对象
   */
  checkAndAlarm(data) {
    // 获取当前页面路由，排除登录页
    const pages = getCurrentPages();
    const currentPage = pages[pages.length - 1];
    if (currentPage && currentPage.route === 'pages/login/login') {
      return;
    }
    
    // 验证数据有效性
    if (!data || typeof data !== 'object') {
      console.error('无效的MQTT数据格式:', data);
      return;
    }
    
    // 确保阈值管理器已初始化
    if (!thresholdManager.cachedThresholds) {
      thresholdManager.initThresholds();
    }
    
    // 检查每个设备数据
    Object.keys(data).forEach(deviceKey => {
      const value = data[deviceKey];
      const result = thresholdManager.checkThreshold(deviceKey, value);
      
      if (!result) {
        return;
      }
      
      // 只有当值超出阈值范围时才触发报警（isOver或isUnder为true）
      if (result.isOver || result.isUnder) {
        // 记录活跃报警
        this.activeAlarms[deviceKey] = result;
        // 触发报警（包括自动下发控制信息）
        this.triggerAlarm(result);
      } else if (this.activeAlarms[deviceKey] && result.isInRange) {
        // 如果之前有报警，现在恢复正常，可以发送恢复控制信息
        this.sendRecoveryMessage(deviceKey);
        // 清除活跃报警记录
        delete this.activeAlarms[deviceKey];
      }
    });
  },
  
  /**
   * 触发报警
   * @param {Object} alarmInfo - 报警信息
   */
  triggerAlarm(alarmInfo) {
    const { deviceKey, type } = alarmInfo;
    
    // 防止短时间内重复报警（1分钟内只报警一次）
    if (this.alarmTimers[deviceKey]) {
      return;
    }
    
    this.alarmTimers[deviceKey] = setTimeout(() => {
      delete this.alarmTimers[deviceKey];
    }, 10000); // 60秒后允许再次报警
    
    // 自动下发控制信息
    this.sendControlMessage(deviceKey, type);
    
    // 获取将触发的控制设备信息
    const triggeredDevices = this.getTriggeredDevicesInfo(deviceKey, type);
    
    // 显示报警弹窗
    this.showAlarmDialog(alarmInfo, triggeredDevices);
  },
  
  /**
   * 获取将被触发的控制设备信息
   * @param {string} deviceKey - 设备key
   * @param {string} type - 报警类型(over/under)
   * @returns {Array} - 控制设备信息数组
   */
  getTriggeredDevicesInfo(deviceKey, type) {
    const controlDevices = [];
    
    const linkageConfig = config.deviceLinkageConfig?.[deviceKey];
    if (!linkageConfig || !linkageConfig[type]) {
      return controlDevices;
    }
    
    for (const controlDeviceConfig of linkageConfig[type]) {
      const controlDevice = config.devices_ctrl?.find(ctrl => ctrl.key === controlDeviceConfig.key);
      if (controlDevice) {
        controlDevices.push({
          label: controlDevice.label,
          action: controlDeviceConfig.action
        });
      }
    }
    
    return controlDevices;
  },
  
  /**
   * 显示报警弹窗
   * @param {Object} alarmInfo - 报警信息
   * @param {Array} triggeredDevices - 触发的控制设备信息
   */
  showAlarmDialog(alarmInfo, triggeredDevices) {
    const { type, deviceKey, value, threshold } = alarmInfo;
    const device = config.devices?.find(d => d.key === deviceKey);
    const symbol = type === 'over' ? '↑' : '↓';
    
    // 构建控制设备触发信息
    let controlInfo = '';
    if (triggeredDevices && triggeredDevices.length > 0) {
      controlInfo = '\n\n系统已自动触发以下控制设备:\n';
      triggeredDevices.forEach(device => {
        controlInfo += `- ${device.label} (${device.action === 'on' ? '开启' : '关闭'})\n`;
      });
      controlInfo += '\n控制指令将间隔1秒发送，以确保硬件可靠执行。';
    } else {
      controlInfo = '\n\n系统未配置自动控制设备联动。';
    }
    
    uni.showModal({
      title: `【${triggeredDevices.length > 0 ? '自动控制已触发' : '报警提醒'}】${device?.label || deviceKey} 报警`,
      content: `${device?.label || deviceKey} ${symbol}\n当前值: ${value}\n设定阈值: ${threshold}\n${type === 'over' ? '超过' : '低于'}阈值: ${Math.abs(value-threshold)}\n${controlInfo}\n\n是否需要关闭报警？`,
      confirmText: '关闭报警',
      cancelText: '保持现状',
      success: (res) => {
        if (res.confirm) {
          // 发送关闭报警的控制信息
          this.sendCloseAlarmMessage(deviceKey);
        }
      }
    });
  },
  
  /**
   * 发送控制消息到MQTT
   * @param {string} deviceKey - 设备key
   * @param {string} type - 报警类型(over/under)
   */
  sendControlMessage(deviceKey, type) {
    const device = config.devices?.find(d => d.key === deviceKey);
    if (!device || !device.alarmMessage) return;
    
    // 获取对应的报警消息
    const alarmMessage = type === 'over' ? device.alarmMessage.over : device.alarmMessage.under;
    
    if (alarmMessage) {
      // 使用队列发送消息
      this.addControlCommand(alarmMessage, `${device.label} 报警消息`);
    }
    
    // 查找并触发对应的控制设备
    this.triggerControlDevice(deviceKey, type);
  },
  
  /**
   * 发送关闭报警的控制信息
   * @param {string} deviceKey - 设备key
   */
  sendCloseAlarmMessage(deviceKey) {
    const device = config.devices?.find(d => d.key === deviceKey);
    if (!device) return;
    
    // 构建关闭报警消息
    const closeMessage = `${device.key}:off`;
    // 使用队列发送消息
    this.addControlCommand(closeMessage, `${device.label} 关闭消息`);
    
    // 清除活跃报警记录
    delete this.activeAlarms[deviceKey];
    
    // 关闭对应的控制设备
    this.deactivateControlDevice(deviceKey);
  },
  
  /**
   * 发送恢复正常的控制信息
   * @param {string} deviceKey - 设备key
   */
  sendRecoveryMessage(deviceKey) {
    const device = config.devices?.find(d => d.key === deviceKey);
    if (!device) return;
    
    // 构建恢复正常消息
    const recoveryMessage = `${device.key}:off`;
    // 使用队列发送消息
    this.addControlCommand(recoveryMessage, `${device.label} 恢复消息`);
  },
  
  /**
   * 触发相关的控制设备
   * @param {string} deviceKey - 报警设备key
   * @param {string} type - 报警类型(over/under)
   */
  triggerControlDevice(deviceKey, type) {
    // 检查是否有相关联的控制设备
    if (!config.devices_ctrl || !Array.isArray(config.devices_ctrl)) {
      return;
    }
    
    // 获取联动配置
    const linkageConfig = config.deviceLinkageConfig?.[deviceKey];
    if (!linkageConfig || !linkageConfig[type]) {
      return;
    }
    
    // 获取需要触发的控制设备列表
    const controlDevices = linkageConfig[type];
    const triggeredDevices = [];
    
    // 遍历所有需要触发的控制设备
    for (const controlDeviceConfig of controlDevices) {
      const controlDevice = config.devices_ctrl.find(ctrl => ctrl.key === controlDeviceConfig.key);
      
      if (controlDevice && controlDevice.mqttConfig) {
        // 根据动作决定发送onMessage或offMessage
        const message = controlDeviceConfig.action === 'on' ? 
                       controlDevice.mqttConfig.onMessage : 
                       controlDevice.mqttConfig.offMessage;
        
        if (message) {
          // 添加到命令队列，间隔发送
          this.addControlCommand(message, controlDevice.label);
          
          triggeredDevices.push({
            deviceKey: controlDevice.key,
            action: controlDeviceConfig.action
          });
        }
      }
    }
    
    // 如果有设备被触发，记录完整的触发信息
    if (triggeredDevices.length > 0) {
      this.activeAlarms[`${deviceKey}_control`] = {
        deviceKey,
        controlDevices: triggeredDevices
      };
    }
  },
  
  /**
   * 停用相关的控制设备
   * @param {string} deviceKey - 报警设备key
   */
  deactivateControlDevice(deviceKey) {
    if (!config.devices_ctrl || !Array.isArray(config.devices_ctrl)) {
      return;
    }
    
    // 获取被触发的控制设备信息
    const controlAlarmInfo = this.activeAlarms[`${deviceKey}_control`];
    if (controlAlarmInfo && controlAlarmInfo.controlDevices) {
      // 遍历所有需要停用的控制设备
      for (const controlDeviceInfo of controlAlarmInfo.controlDevices) {
        // 查找对应的控制设备
        const controlDevice = config.devices_ctrl.find(ctrl => ctrl.key === controlDeviceInfo.deviceKey);
        
        // 如果是on动作，则发送off消息来停用；如果是off动作，则发送on消息来恢复
        const action = controlDeviceInfo.action === 'on' ? 'off' : 'on';
        const message = action === 'on' ? 
                       controlDevice?.mqttConfig?.onMessage : 
                       controlDevice?.mqttConfig?.offMessage;
        
        if (controlDevice && message) {
          // 添加到命令队列，间隔发送
          this.addControlCommand(message, controlDevice.label);
        }
      }
      
      // 移除总控制设备记录
      delete this.activeAlarms[`${deviceKey}_control`];
    } 
	// else {
 //      // 兼容旧的控制设备逻辑
 //      // 根据设备key关联对应的控制设备
 //      const deviceMapping = {
 //        'temp': ['FUN_STATE'],
 //        'humi': ['WATER_STATE', 'jdq'],
 //        'light': ['LED_STATE']
 //      };
      
 //      const possibleKeys = deviceMapping[deviceKey] || [];
 //      for (const key of possibleKeys) {
 //        const controlDevice = config.devices_ctrl.find(ctrl => ctrl.key === key);
 //        if (controlDevice && controlDevice.mqttConfig?.offMessage) {
 //          this.addControlCommand(controlDevice.mqttConfig.offMessage, controlDevice.label);
 //        }
 //      }
 //    }
  }
};