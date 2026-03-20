/*  MQTT管理工具  */
import { ref } from 'vue';
import mqtt from 'mqtt/dist/mqtt.js';
import alarmManager from './alarmManager';  
import config from '../page_config/config.js';

const mqtt_config = ref({});
mqtt_config.value = { ...config.mqtt };

/**
 * MQTT客户端实例
 */
export const mqttClient = ref(null);

/**
 * 连接状态
 */
export const connectStatus = ref(false); // true: 已连接, false: 未连接

/**
 * 单片机状态
 */
export const Stm32Status = ref(false); // true: 已连接, false: 未连接

/**
 * 连接MQTT服务器
 * @returns {Promise<object>} 连接结果
 */
export function mqtt_connect() {
  return new Promise((resolve, reject) => {
    try {
      // 若已连接则先断开
      if (mqttClient.value && connectStatus.value) {
        mqttClient.value.end();
      }

      // 连接选项
      const options = {
        clientId: mqtt_config.value.clientId,
        username: mqtt_config.value.username,
        password: mqtt_config.value.password,
        keepalive: mqtt_config.value.keepalive,
        clean: mqtt_config.value.clean,
        reconnectPeriod: mqtt_config.value.reconnectPeriod,
        connectTimeout: mqtt_config.value.connectTimeout,
        qos: mqtt_config.value.qos // 默认QoS等级
      };

      // 根据平台选择协议和连接方式
      let connectUrl = '';
      // #ifdef H5
      connectUrl = `ws://${mqtt_config.value.url}/mqtt`;
      // #endif
      // #ifdef APP-PLUS
      connectUrl = `wx://${mqtt_config.value.url}/mqtt`; // APP端使用mqtt协议
      // #endif

      mqttClient.value = mqtt.connect(connectUrl, options);

      // 连接成功
      mqttClient.value.on('connect', (connack) => {
        connectStatus.value = true;
        console.log('MQTT连接成功!');
        // 定义一个定时器变量，用于检测超时
        let mqttTimeoutTimer;
        
        // 设置超时检查函数
        const resetTimeout = () => {
          // 清除现有定时器
          if (mqttTimeoutTimer) {
            clearTimeout(mqttTimeoutTimer);
          }
          
          // 设置定时器 用来判断单片机离线状态
          mqttTimeoutTimer = setTimeout(() => {
            Stm32Status.value = false;
          }, mqtt_config.value.stm32_Olnine_time); 
        };
        
        // 初始设置一次定时器
        resetTimeout();
        
        // 连接成功后订阅主题
        mqtt_subscribe();
        
        // 添加全局消息监听
        mqttClient.value.on('message', (topic, message) => {
          try {
            // 打印接收到的消息
            console.log('接收到MQTT消息 - 主题:', topic, '内容:', message.toString());
            
            const data = JSON.parse(message.toString());
            Stm32Status.value = data.code == topic ? true : false;
            alarmManager.checkAndAlarm(data);
            
            // 收到消息后重置超时定时器保证单片机状态为在线
            resetTimeout();
          } catch (error) {
            console.error('全局解析MQTT消息失败:', error);
            // 打印原始消息以便调试
            console.log('接收到的原始消息:', message.toString());
          }
        });
        
        resolve(connack);
      });

      // 连接错误
      mqttClient.value.on('error', (error) => {
        connectStatus.value = false;
        reject(error);
      });

      // 断开连接
      mqttClient.value.on('close', () => {
        connectStatus.value = false;
      });

      // 重连中
      mqttClient.value.on('reconnect', () => {
        console.log('MQTT重连中...');
        // 重连处理
      });
    } catch (error) {
      reject(error);
    }
  });
}

/**
 * 订阅主题
 */
export function mqtt_subscribe() {
  if (!mqttClient.value || !connectStatus.value) {
    return;
  }

  // 订阅接收数据的主题
  mqttClient.value.subscribe(
    mqtt_config.value.topic,
    { qos: mqtt_config.value.qos },
    (err) => {
      // 订阅结果处理
      if (err) {
        console.error('订阅主题失败:', err);
      } else {
        console.log('订阅主题成功');
      }
    }
  );
}

/**
 * 发送消息到控制主题
 * @param {string|object} message 要发送的消息（对象会转为JSON字符串）
 */
export function mqtt_publish(message) {
  if (!mqttClient.value || !connectStatus.value) {
    return;
  }

  // 若消息是对象，转为JSON字符串
  const payload = typeof message === 'object' ? JSON.stringify(message) : message;

  mqttClient.value.publish(
    mqtt_config.value.topic_ctrl,
    payload,
    { qos: mqtt_config.value.qos },
    (err) => {
      // 发布结果处理
      if (err) {
        console.error('payload+发布主题失败:', err);
      } else {
        console.log(payload+ "发布主题成功");
      }
    }
  );
}

/**
 * 断开MQTT连接
 */
export function mqtt_disconnect() {
  if (mqttClient.value && connectStatus.value) {
    mqttClient.value.end();
    connectStatus.value = false;
  }
}
    