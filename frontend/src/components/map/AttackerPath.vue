<template>
  <path :d="path"/>
</template>

<script lang="ts">
import {defineComponent} from "vue";

export default defineComponent({
  name: "AttackerPath",
  props: {
    refresh: {
      type: Boolean,
      required: false,
      default: true
    },
    points: {
      type: Array as () => [number, number][],
      required: false,
      default: []
    }
  },
  computed: {
    path () {
      this.refresh;
      return `M ${this.points.map(p => `${p[0]} ${p[1]}`).join(" L ")}`;
    }
  }
});
</script>

<style lang="scss" scoped>
@import "@/style/map";
path {
  fill: none;
  stroke: black;
  stroke-width: 0.004;
}
</style>
