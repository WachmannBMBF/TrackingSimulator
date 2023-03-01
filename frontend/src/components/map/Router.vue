<template>
  <circle class="inner" :class="{highlight: mode & 1, active: mode & 2}" :cx="x" :cy="y" :r="radiusInner"/>
  <circle class="outer" :class="{highlight: mode & 1, active: mode & 2}" :cx="x" :cy="y" :r="radius"/>
</template>

<script lang="ts">
import {defineComponent} from "vue";

export default defineComponent({
  name: "Router",
  props: {
    radius: {
      type: Number,
      required: true
    },
    radiusInner: {
      type: Number,
      required: true
    },
    x: {
      type: Number,
      required: true
    },
    y: {
      type: Number,
      required: true
    },
    mode: {
      type: Number,
      default: 0
    }
  }
});
</script>

<style lang="scss" scoped>
@import "@/style/map";
circle {
  stroke: none;

  &.inner {
    opacity: 0.9;
  }
  &.outer {
    opacity: 0.2;
  }
  &.inner, &.outer {
    fill: $router-primary;
  }
  &.inner.active, &.outer.active {
    fill: $router-active;
  }
  &.inner.highlight, &.outer.highlight {
    fill: $router-highlight;
  }

}
</style>
