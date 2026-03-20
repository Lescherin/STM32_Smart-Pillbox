// utils/historyManager.js
/*  历史数据管理工具  */
export default {
  // 存储键名
  STORAGE_KEY: 'mqttHistory',
  // 最大存储记录数
  MAX_RECORDS: 1000,
  
  /**
   * 保存数据到历史记录
   * @param {Object} data - 要保存的MQTT数据对象
   * @returns {boolean} 是否保存成功
   */
  saveHistoryData(data) {
    try {
      // 获取现有历史记录，并强制转为数组（复制一份新数组避免原始数据异常）
      const originalHistory = this.getHistoryData();
      const history = Array.isArray(originalHistory) ? [...originalHistory] : [];
      
      // 创建带时间戳的记录
      const record = {
        ...data,
        timestamp: Date.now(),
        datetime: this.formatDateTime(Date.now())
      };
      
      // 添加新记录到头部（此时history一定是数组，且可修改）
      history.unshift(record);
    
      // 限制记录数量
      if (history.length > this.MAX_RECORDS) {
        history.splice(this.MAX_RECORDS);
      }
      
      // 保存到本地存储
      uni.setStorageSync(this.STORAGE_KEY, history);
      return true;
    } catch (error) {
      console.error('保存历史数据失败:', error);
      return false;
    }
  },
  
  /**
   * 获取所有历史记录
   * @returns {Array} 历史记录数组
   */
  getHistoryData() {
    try {
      return uni.getStorageSync(this.STORAGE_KEY) || [];
    } catch (error) {
      console.error('获取历史数据失败:', error);
      return [];
    }
  },
  
  /**
   * 按条件查询历史记录
   * @param {Object} options - 查询条件
   * @param {string} options.field - 要查询的字段
   * @param {number} options.startTime - 开始时间戳
   * @param {number} options.endTime - 结束时间戳
   * @returns {Array} 符合条件的记录
   */
  queryHistoryData(options = {}) {
    const { field, startTime, endTime } = options;
    const allHistory = this.getHistoryData();
    
    // 筛选包含指定字段的记录
    let results = allHistory.filter(record => record.hasOwnProperty(field));
    
    // 时间范围筛选
    if (startTime) {
      results = results.filter(record => record.timestamp >= startTime);
    }
    
    if (endTime) {
      results = results.filter(record => record.timestamp <= endTime);
    }
    
    return results;
  },
  
  /**
   * 格式化时间戳为可读字符串
   * @param {number} timestamp - 时间戳（毫秒）
   * @returns {string} 格式化后的时间字符串
   */
  formatDateTime(timestamp) {
    try {
      const date = new Date(timestamp);
      return `${date.getFullYear()}-${this.padZero(date.getMonth() + 1)}-${this.padZero(date.getDate())} ${this.padZero(date.getHours())}:${this.padZero(date.getMinutes())}:${this.padZero(date.getSeconds())}`;
    } catch (error) {
      console.error('格式化日期时间失败:', error);
      return '';
    }
  },
  
  /**
   * 数字补零
   * @param {number} num - 要补零的数字
   * @returns {string} 补零后的字符串
   */
  padZero(num) {
    return num < 10 ? `0${num}` : `${num}`;
  },
  
  /**
   * 清除历史数据
   * @param {number} days - 保留最近几天的数据，默认全部清除
   * @returns {boolean} 是否清除成功
   */
  clearHistoryData(days = 0) {
    try {
      if (days <= 0) {
        // 全部清除
        uni.removeStorageSync(this.STORAGE_KEY);
      } else {
        // 保留最近N天的数据
        const history = this.getHistoryData();
        const cutoffTime = Date.now() - (days * 24 * 60 * 60 * 1000);
        const remaining = history.filter(record => record.timestamp >= cutoffTime);
        uni.setStorageSync(this.STORAGE_KEY, remaining);
      }
      return true;
    } catch (error) {
      console.error('清除历史数据失败:', error);
      return false;
    }
  }
};
