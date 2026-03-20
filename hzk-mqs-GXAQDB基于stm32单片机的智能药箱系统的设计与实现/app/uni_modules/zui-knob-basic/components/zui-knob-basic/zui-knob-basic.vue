<template>
  <view class="zui-knob-basic">
    <view
      ref="eleWrapper"
      :class="{ 'zui-knob-basic-wrapper': 1, debug }"
      :style="style"
    >
      <div class="background">
        <image class="base-img" :src="theme.base" mode="aspectFit"></image>
      </div>
      <div
        class="handle"
        @touchstart.prevent="dragStart"
        @touchmove.prevent="dragMoving"
        @touchcancel.prevent="dragCancel"
        @touchend.prevent="dragEnd"
      >
        <image class="knob-img" :src="theme.handle" mode="aspectFit"></image>
        <!-- <image class="hint-img" :src="theme.hint" mode="aspectFit"></image> -->
      </div>
      <view v-if="debug" class="debug-frame">
        <view class="anchor"></view>
        <view class="real-anchor"></view>
        <view class="handle"></view>
      </view>
    </view>
  </view>
</template>

<script>
const TWO_PI = 2 * Math.PI;

export default {
  props: {
    /**
     * 组件值, [0, 1]
     */
    modelValue: {
      type: Number,
      value: 0,
    },

    /**
     * 值映射范围. 将 modelValue 值映射到指定范围, 并可以指定精度
     *
     * [值1, 值2, 精度]
     *
     * 值1, 值2 不能相对, 大小无顺序限制
     */
    valueRange: {
      type: Array,
      default: [0, 1, 0.001],
      validator: (val) => {
        if (!val) return true;
        if (!(val instanceof Array)) return false;
        if (val.length < 3) return false;

        if (typeof val[0] !== "number") return false;
        if (typeof val[1] !== "number") return false;
        if (val[0] === val[1]) return false;

        if (typeof val[2] !== "number") return false;
        if (!/^(10*|0?\.0*1|1e-[1-9]\d*)$/.test(String(val[2]))) return false;

        return true;
      },
    },

    /**
     * 设置组件旋转的起始位置
     *
     * [start, end]
     *
     * start < end, 顺时针范围
     * start > end, 逆时针范围
     */
    range: {
      type: Array,
      default: undefined,
      validator: (val) => {
        if (!val) return true;
        if (!val instanceof Array) return false;
        if (typeof val[0] !== "number" || typeof val[1] !== "number")
          return false;

        return val[0] >= 0 && val[0] <= 360 && val[1] >= 0 && val[1] <= 360;
      },
    },

    /**
     * 组件尺寸
     */
    size: {
      type: Number,
      default: 150,
    },

    /**
     * 旋钮大小
     *
     * [0, 1]  以组件尺寸大小的百分比
     * [44, ]  像素大小
     */
    knobSize: {
      type: Number,
      default: 0.8,
    },

    /**
     * 默认位置
     */
    position: {
      type: Number,
      default: 0,
    },

    /**
     * 事件触发频率
     *
     * 单位: 毫秒
     */
    eventFreq: {
      type: Number,
      default: 10,
    },

    theme: {
      type: Object,
      default: () => ({}),
    },

    /**
     * 开启 debug 模式
     */
    debug: Boolean,
  },

  data() {
    return {
      centerX: 0,
      centerY: 0,
      //
      fixUniHead: 0,

      direction: "none",

      // 防止事件过于频繁触发
      _eventLock: false,

      rotateDeleta: 0,
      currentPos: this.position,
      draging: false,
      prevRotate: 0,
      attachSide: "none",
    };
  },

  computed: {
    /**
     * 是否有限定范围, 默认 360 度
     */
    hasRange() {
      const val = this.range;
      if (!val) return false;
      if (!val instanceof Array) return false;
      if (typeof val[0] !== "number" || typeof val[1] !== "number")
        return false;

      return val[0] >= 0 && val[0] <= 360 && val[1] >= 0 && val[1] <= 360;
    },

    knobSizeFixed() {
      if (this.knobSize <= 1) return this.knobSize * this.size;

      return this.knobSize;
    },

    startFixed() {
      const range = this.range || [0, 360];
      return range[0];
    },

    endFixed() {
      const range = this.range || [0, 360];
      if (range[1] < range[0]) return range[1] + 360;
      return range[1];
    },

    positionFixed() {
      let pos = this.currentPos + this.rotateDeleta;
      if (this.hasRange) {
        if (pos < this.startFixed) {
          pos = this.startFixed;
        } else if (pos > this.endFixed) {
          pos = this.endFixed;
        }
      }
      return pos;
    },

    style() {
      const style = {
        width: `${this.size}px`,
        height: `${this.size}px`,
        "--zui-knob-basic-size": `${this.size}px`,
        "--zui-knob-handle-rotate": `rotate(${this.positionFixed}deg)`,
        "--zui-knob-handle-size": `${this.knobSizeFixed}px`,
      };

      return Object.keys(style)
        .map((key) => `${key}:${style[key]}`)
        .join(";");
    },
  },

  watch: {
    range() {
      this.prevRotate = 0;
      this.rotateDeleta = 0;
      this.$nextTick(() => {
        this.updatePosition();
      });
    },
    modelValue(mn, mo) {
      if (this.draging) return;

      this.syncModelValue();
    },
    position(np) {
      const anglePos = (this.endFixed - this.startFixed) * np + this.startFixed;
      this.currentPos = anglePos;
      this.$nextTick(() => {
        this.updatePosition();
      });
    },
  },

  mounted() {
    this.updateCenter();
    this.syncModelValue();
  },

  methods: {
    syncModelValue() {
      const dist = this.valueRange[1] - this.valueRange[0];
      let pos = (this.modelValue - this.valueRange[0]) / dist;
      pos = pos < 0 ? 1 + pos : Math.abs(pos);
      const anglePos =
        (this.endFixed - this.startFixed) * pos + this.startFixed;

      this.currentPos = anglePos;
      this.$nextTick(() => {
        this.updatePosition();
      });
    },
    updateCenter() {
      // #ifdef H5
      const rect = this.$refs.eleWrapper.$el.getBoundingClientRect();
      this.centerX = rect.left + rect.width / 2;
      this.centerY = rect.top + rect.height / 2;

      if (typeof document !== "undefined") {
        const head = document.querySelector(".uni-page-head");
        this.fixUniHead = !!head ? 44 : 0;
      }
      // #endif

      // #ifndef H5
      const query = uni.createSelectorQuery().in(this);
      query
        .select(".zui-knob-basic-wrapper")
        .fields({ size: true, rect: true })
        .exec((rst) => {
          if (!rst) return;
          if (!rst[0]) return;
          this.centerX = rst[0].left + rst[0].width / 2;
          this.centerY = rst[0].top + rst[0].height / 2;
        });
      // #endif
    },

    updatePosition() {
      if (this.hasRange) {
        this.currentPos = this.positionFixed;
      }
    },

    tickCount(count) {
      if (count === 0) {
        this.skipCalc = false;
        return;
      }

      this.$nextTick(() => {
        this.tickCount(count--);
      });
    },

    calucateAngle(nx, ny) {
      const dx = nx - this.centerX;
      const dy = ny - this.centerY;
      const radius = Math.sqrt(dx * dx + dy * dy);
      const radian = (Math.atan2(dy, dx) + TWO_PI) % TWO_PI;
      const degree = ((radian * 180) / Math.PI + 360) % 360;

      const state = {
        radius,
        radian,
        degree,
      };

      this.rotate = degree;
      this.radius = radius;

      if (this.rotateDeleta === 0) {
        this.prevRotate = degree;
        this.rotateDeleta = 0.00000001;
      }
      let diff = degree - this.prevRotate;
      // 跨 0 度轴处理
      if (diff < -90) {
        diff += 360;
      } else if (diff > 90) {
        diff -= 360;
      }
      this.rotateDeleta = this.rotateDeleta + diff;
      this.prevRotate = degree;

      state.deleta = this.rotateDeleta;
      state.diff = diff;
      state.direction = diff >= 0 ? "clockwise" : "counterclockwise";
      return state;
    },

    dragStart(evt) {
      this.$emit("active");
      this.draging = true;
      this.prevRotate = this.rotate;
      this.rotateDeleta = 0;
      this.dragMoving(evt);
    },

    dragMoving(evt) {
      const change = this.calucateAngle(
        evt.touches[0].clientX,
        /**
         * H5 环境下, 如果启用了 uni 的标题栏, 则需要将 clientY + 44
         */
        evt.touches[0].clientY + this.fixUniHead
      );

      this.fireEvent(change);

      if (this.direction !== change.direction) {
        this.$emit("direction", {
          from: this.direction,
          to: change.direction,
        });
        this.direction = change.direction;
      }
    },

    dragCancel() {
      this.reset();
    },

    dragEnd() {
      this.reset();
    },

    reset() {
      this.currentPos = this.currentPos + this.rotateDeleta;
      this.rotateDeleta = 0;
      this.draging = false;
      this.$emit("deactive");
    },

    fireEvent(change) {
      if (this._eventLock) return;
      this._eventLock = true;

      let value = (this.positionFixed % 360) / 360;
      if (this.hasRange) {
        value =
          (this.positionFixed - this.startFixed) /
          (this.endFixed - this.startFixed);
      }

      const s = this.valueRange[0];
      const e = this.valueRange[1];
      const p = this.valueRange[2];
      const diff = e - s;

      value = value * diff + s;
      value = Math.round(value / p) * p;

      change.value = value;
      this.$emit("change", change);
      if (this.modelValue !== value) {
        this.$emit("update:modelValue", value);
      }

      setTimeout(() => {
        this._eventLock = false;
      }, this.eventFreq);
    },
  },
};
</script>

<style scoped>
.zui-knob-basic {
  position: relative;
  border-radius: 50%;
  display: flex;
}

.zui-knob-basic-wrapper {
  position: relative;
}

.zui-knob-basic-wrapper .background {
  position: relative;
  top: var(--zui-knob-basic-arrow-size);
  left: var(--zui-knob-basic-arrow-size);
  width: var(--zui-knob-basic-size);
  height: var(--zui-knob-basic-size);
  border-radius: 50%;
  border: 1px solid #ffffff;
  box-sizing: border-box;
}

.base-img {
  width: var(--zui-knob-basic-size);
  height: var(--zui-knob-basic-size);
}

.handle {
  border-radius: 50%;
  display: flex;
  justify-content: center;
  align-items: center;
  position: absolute;
  left: 0;
  top: 0;
  width: 100%;
  height: 100%;

  transform-origin: center;
  transform: var(--zui-knob-handle-rotate, rotate(0));
}

.knob-img {
  width: var(--zui-knob-handle-size);
  height: var(--zui-knob-handle-size);
}

.hint-img {
  width: var(--zui-knob-basic-hint-size);
  height: var(--zui-knob-basic-hint-size);
}

.debug-frame {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  border: 1px solid #f0f;
  border-radius: 50%;
  pointer-events: none;

  .anchor {
    position: absolute;
    top: 50%;
    left: 50%;
    width: 0;
    height: 0;
    transform: var(--zui-knob-handle-rotate, rotate(0));

    &:before {
      content: "";
      display: block;
      position: relative;
      z-index: 10;
      width: 12rpx;
      height: 12rpx;
      border-radius: 50%;
      background-color: #f0f;
      transform: translate(-50%, -50%);
    }

    &:after {
      content: "";
      display: block;
      position: absolute;
      z-index: 1;
      left: 0;
      top: 0;
      width: calc(var(--zui-knob-basic-size) / 2);
      height: 1px;
      background-color: #f0f;
      transform: translateY(-50%);
    }
  }

  .real-anchor {
    position: absolute;
    z-index: 100;
    top: var(--knob-real-anchor-cy);
    left: var(--knob-real-anchor-cx);
    width: 8px;
    height: 8px;
    border: 1px solid #00f;
  }

  .handle {
    display: flex;
    justify-content: center;
    align-items: center;
    position: absolute;
    left: 0;
    top: 0;
    width: 100%;
    height: 100%;
    transform-origin: center;
    transform: var(--zui-knob-handle-rotate, rotate(0));
    border-radius: 0;
    border: 1px solid #f0f;
  }
}
</style>
