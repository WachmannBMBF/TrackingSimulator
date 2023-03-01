<template>
  <circle :cx="x" :cy="y" :r="0.01"/>
</template>

<script lang="ts">
import {defineComponent} from "vue";

export default defineComponent({
  name: "Attacker",
  props: {
    x: {
      type: Number,
      required: true
    },
    y: {
      type: Number,
      required: true
    },
  }
});
</script>

<style lang="scss" scoped>
@import "@/style/map";
circle {
  fill: black;
  stroke: none;

  &.highlight {
    fill: $router-highlight;
    stroke: $router-highlight;
    stroke-width: 0.005;
  }
}
</style>
