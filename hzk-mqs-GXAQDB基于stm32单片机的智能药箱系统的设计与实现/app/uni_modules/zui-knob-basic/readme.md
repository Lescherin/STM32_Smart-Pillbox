# zui-knob-basic 基础款旋钮

一款旋钮模拟组件。旋转开关，调节器。可用于控制台灯色温控制、空调温度控制、音量控制。


特性：

1. 支持 v-model 绑定
2. 支持限位设置，控制转动范围
3. 支持转动增量值
4. 支持转动方向事件
5. 支持值映射。将角度值映射到指定范围，并可设置转换精度



## 🍓 立即使用

```VUE
<zui-knob-basic @change="onChange" @direction="onDirectionChange" />
```


## 在线演示

**[💻 点我在浏览器里预览 https://uni.imgozi.cn/zui-knob-basic/](https://uni.imgozi.cn/zui-knob-basic/?utm_source=uni-plugin-market&utm_medium=readme&utm_campaign=zui-knob-basic&utm_id=uni-plugin)**

PS: 启动浏览器预览需要打开手机模器

**[📱 扫码体验](https://uni.imgozi.cn/zui-knob-basic/?utm_source=uni-plugin-market&utm_medium=readme&utm_campaign=zui-knob-basic&utm_id=uni-plugin)**

<img src="https://uni.imgozi.cn/zui-knob-basic/static/preview-qr.png" width="128" />


## 🍊 参数

| 参数 | 类型 | 说明 |
|---|---|:---|
| v-model | number | 双向绑定，值范围由 valueRange 确定。默认：[0, 1]，精度：0.001 |
| valueRange | [number, number, number] | 值映射范围，默认：[0, 1, 0.001]<br />前2个值为值范围，无大小顺序要求；最后一个值为值精度 |
| range | [number, number] | 限位设置，限制旋转范围。数值为 [0, 360]。第一个数值小于第二个数值时，按顺时针限制范围，反之按逆时针限定范围 |
| knobSize | number  | 旋钮尺寸 |
| position | number | 当前位置。[0, 360] |
| theme | object | 样式设置。对象数据结构见下方 |
| eventFreq | number | 事件触发频率。默认 10 毫秒一次 |

### theme

```javascript
{
  base: '底图.png',
  handle: '旋钮.png',
}
```





## 🍒 事件

以下所列事件按触发顺序列出

### @active=onActive() => void

开始操作时触发

```javascript
// 无参数
```

### @change=on​Change(chg) => void

摇杆推动时触发，事件触发间隔可通过参数 `eventFreq` 调整

旋转角度计量以右方为起点，顺时针方向

```javascript
{
  direction: 'top',   // 方向
  degree: 0,   // 角度, [0, 360]
  deleta: 720,  // 累计旋转度数
}
```

`direction` 取值范围: `'clockwise' | 'counterclockwise'`

### @direction=onDirection​Change(chg) => void

当方向发生变化时触发

```
{
  from: 'top', // 变化之前的方向
  to: 'top-right', // 变化之后的方向, 即当前方向
}
```

### @deactive=onDeactive() => void

结束操作时触发

```javascript
// 无参数
```





## 🍓 支持

如果组件对您有帮助，请不吝打赏。肥宅快乐水🥤是创作动力！🥤🥤🥤

<span class="banner">
<span class="surport">
<a class="btn btn-support " data-toggle="modal" data-target="#support_modal" style="border: 1px solid #ec4d4d;letter-spacing: 1px;">
  🍓🍇🍉  喜欢就打赏一下  🍒🍑🥭
</a>
</span>
</span>
