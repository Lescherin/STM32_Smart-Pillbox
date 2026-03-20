// utils/thresholdManager.js
/*  阈值管理工具  */
import config from '../page_config/config.js';

export default {
  STORAGE_KEY: 'deviceThresholds',
  // 缓存阈值数据以提高性能
  cachedThresholds: null,
  // 边界检查的容差值（防止浮点精度问题）
  tolerance: 0.001,
  
  /**
   * 初始化阈值（如果本地没有则使用配置中的默认值）
   * @returns {Object} 阈值对象
   */
   initThresholds() {
     // 初始化阈值数据
     const saved = this.getThresholds();
     
     if (!saved || Object.keys(saved).length === 0) {
       const defaultThresholds = {};
       
       // 初始化所有设备的阈值
       if (Array.isArray(config.devices)) {
         config.devices.forEach(device => {
           if (!device || !device.key) return;
           
           // 确保转换为数字类型
           defaultThresholds[device.key] = {
             max: Number(device.thresholdMax) || 0,
             min: Number(device.thresholdMin) || 0
           };
         });
       }
       
       this.saveThresholds(defaultThresholds);
       this.cachedThresholds = defaultThresholds;
       return defaultThresholds;
     }
     
     this.cachedThresholds = saved;
     return saved;
   },
   
   /**
    * 保存阈值到本地存储
    * @param {Object} thresholds - 阈值对象
    * @returns {boolean} 是否保存成功
    */
   saveThresholds(thresholds) {
     try {
       // 确保所有阈值都是数字类型
       const sanitizedThresholds = {};
       for (const key in thresholds) {
         if (Object.prototype.hasOwnProperty.call(thresholds, key)) {
           const max = Number(thresholds[key].max) || 0;
           const min = Number(thresholds[key].min) || 0;
           
           // 确保最大值大于等于最小值
       if (max < min) {
         console.error(`设备${key}阈值设置无效: max=${max} < min=${min}`);
         continue; // 跳过无效的阈值设置
       }
           
           sanitizedThresholds[key] = {
             max,
             min
           };
         }
       }
       
       uni.setStorageSync(this.STORAGE_KEY, sanitizedThresholds);
       // 更新缓存
       this.cachedThresholds = sanitizedThresholds;
       return true;
     } catch (error) {
       console.error('保存阈值失败:', error);
       return false;
     }
   },
  
  /**
   * 从本地存储获取阈值
   * @returns {Object} 阈值对象
   */
  getThresholds() {
    // 优先从缓存获取
    if (this.cachedThresholds) {
      return this.cachedThresholds;
    }
    
    try {
      const thresholds = uni.getStorageSync(this.STORAGE_KEY) || {};
      this.cachedThresholds = thresholds;
      return thresholds;
    } catch (error) {
      console.error('获取阈值失败:', error);
      return {};
    }
  },
  
  /**
   * 更新特定设备的阈值
   * @param {string} deviceKey - 设备key
   * @param {Object} values - 包含max和min的对象
   * @returns {boolean} 是否更新成功
   */
  updateThreshold(deviceKey, values) {
    const thresholds = this.getThresholds();
    
    // 验证输入有效性
    if (!values || typeof values !== 'object') {
      return false;
    }
    
    // 确保max >= min
    const max = Number(values.max) || 0;
    const min = Number(values.min) || 0;
    
    // 确保最大值大于等于最小值
    if (max < min) {
      console.error(`设备${deviceKey}阈值设置无效: max=${max} < min=${min}`);
      return false; // 跳过无效的阈值设置
    }
    
    thresholds[deviceKey] = {
      max,
      min
    };
    
    return this.saveThresholds(thresholds);
  },
  
  /**
   * 获取单个设备的阈值
   * @param {string} deviceKey - 设备key
   * @returns {Object|null} 阈值对象
   */
  getDeviceThreshold(deviceKey) {
    const thresholds = this.getThresholds();
    const threshold = thresholds[deviceKey];
    
    if (!threshold) {
      return null;
    }
    
    return threshold;
  },
  
  /**
   * 检查值是否超出阈值范围
   * @param {string} deviceKey - 设备key
   * @param {any} value - 要检查的值
   * @returns {Object|null} 超出信息，包含type(over/under)和message
   */
  checkThreshold(deviceKey, value) {
    // 首先确保缓存已初始化
    if (!this.cachedThresholds) {
      this.initThresholds();
    }
    
    const threshold = this.getDeviceThreshold(deviceKey);
    const device = Array.isArray(config.devices) ? config.devices.find(d => d.key === deviceKey) : null;
    
    // 如果没有找到阈值配置或设备配置，直接返回null
    if (!threshold || !device) {
      return null;
    }
    
    // 尝试将值转换为数字进行比较
    let numValue;
    try {
      // 处理字符串类型的值，去除单位等非数字字符
      if (typeof value === 'string') {
        // 提取数字（支持负数和小数）
        const numericStr = value.match(/-?\d+(?:\.\d+)?/)?.[0];
        numValue = numericStr ? Number(numericStr) : NaN;
      } else {
        numValue = Number(value);
      }
      
      // 如果转换失败，不进行阈值检查
      if (isNaN(numValue)) {
        return null;
      }
    } catch (error) {
      console.error('阈值检查失败:', error);
      return null;
    }
    
    const max = Number(threshold.max);
    const min = Number(threshold.min);
    
    // 验证阈值有效性
    if (isNaN(max) || isNaN(min)) {
      return null;
    }
    
    // 检查边界条件
    const isEqualMax = Math.abs(numValue - max) <= this.tolerance;
    const isEqualMin = Math.abs(numValue - min) <= this.tolerance;
    
    // 使用容差值检查是否超出最大值
    if (numValue > max + this.tolerance) {
      return {
        type: 'over',
        message: device.alarmMessage?.over || `${device.label}超过阈值`,
        deviceKey,
        value: numValue,
        threshold: max,
        deviceName: device.label,
        deviceUnit: device.unit || '',
        isInRange: false,
        isOver: true,
        isUnder: false,
        isEqualMax: false,
        isEqualMin: false
      };
    }
    
    // 使用容差值检查是否低于最小值
    if (numValue < min - this.tolerance) {
      return {
        type: 'under',
        message: device.alarmMessage?.under || `${device.label}低于阈值`,
        deviceKey,
        value: numValue,
        threshold: min,
        deviceName: device.label,
        deviceUnit: device.unit || '',
        isInRange: false,
        isOver: false,
        isUnder: true,
        isEqualMax: false,
        isEqualMin: false
      };
    }
    
    // 正常范围内，包括边界值
    return {
      isInRange: true,
      isOver: false,
      isUnder: false,
      deviceKey,
      value: numValue,
      deviceName: device.label,
      deviceUnit: device.unit || '',
      isEqualMax: isEqualMax,
      isEqualMin: isEqualMin
    };
  },
  
  /**
   * 检查值是否在正常范围内（简化版检查）
   * @param {string} deviceKey - 设备key
   * @param {any} value - 要检查的值
   * @returns {boolean} 是否在正常范围内
   */
  isInNormalRange(deviceKey, value) {
    const result = this.checkThreshold(deviceKey, value);
    return result ? result.isInRange : false;
  },
  
  /**
   * 批量检查多个设备的值
   * @param {Object} dataObject - 包含多个设备值的对象
   * @returns {Object} 包含alarms和results的对象
   */
  checkMultipleThresholds(dataObject) {
    if (!dataObject || typeof dataObject !== 'object') {
      return { alarms: [], results: {} };
    }
    
    const alarms = [];
    const results = {};
    
    // 遍历所有设备值
    for (const deviceKey in dataObject) {
      if (Object.prototype.hasOwnProperty.call(dataObject, deviceKey)) {
        const deviceValue = dataObject[deviceKey];
        
        // 检查单个设备的值
        const result = this.checkThreshold(deviceKey, deviceValue);
        
        // 保存检查结果
        results[deviceKey] = result;
        
        // 如果超出阈值范围，添加到报警列表
        if (result && (result.isOver || result.isUnder)) {
          const alarmInfo = {
            deviceKey,
            value: result.value,
            type: result.type || (result.isOver ? 'over' : 'under'),
            message: result.message || (result.isOver ? '超过阈值' : '低于阈值'),
            threshold: result.threshold,
            deviceName: result.deviceName,
            deviceUnit: result.deviceUnit
          };
          
          alarms.push(alarmInfo);
        }
      }
    }
    
    return { alarms, results };
  },
  
  /**
   * 清除特定设备的阈值
   * @param {string} deviceKey - 设备key
   * @returns {boolean} 是否清除成功
   */
  clearThreshold(deviceKey) {
    try {
      const thresholds = this.getThresholds();
      if (thresholds[deviceKey]) {
        delete thresholds[deviceKey];
        return this.saveThresholds(thresholds);
      }
      return true;
    } catch (error) {
      return false;
    }
  },
  
  /**
   * 清除所有阈值
   * @returns {boolean} 是否清除成功
   */
  clearAllThresholds() {
    try {
      uni.removeStorageSync(this.STORAGE_KEY);
      this.cachedThresholds = null;
      return true;
    } catch (error) {
      return false;
    }
  }
};