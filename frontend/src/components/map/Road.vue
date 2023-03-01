<template>
  <path :d="path1"/>
  <path :d="path2"/>
</template>

<script lang="ts">
import {defineComponent} from "vue";
import {EdgeType} from "@/types";

export default defineComponent({
  name: "Road",
  props: {
    x1: {
      type: Number,
      required: true
    },
    y1: {
      type: Number,
      required: true
    },
    x2: {
      type: Number,
      required: true
    },
    y2: {
      type: Number,
      required: true
    },
    type: {
      type: String as () => EdgeType,
      required: false,
      default: EdgeType.tertiary
    }
  },
  methods: {
    computePath(first: boolean): string {
      const direction = [this.x2 - this.x1, this.y2 - this.y1];
      const norm = Math.sqrt(direction[0] ** 2 + direction[1] ** 2);
      const normedNormal = [direction[1] / norm, direction[0] / norm];

      let p1, p2;
      if (first) {
        p1 = [this.x1 + normedNormal[0] * this.width / 2, this.y1 - normedNormal[1] * this.width / 2];
        p2 = [this.x2 + normedNormal[0] * this.width / 2, this.y2 - normedNormal[1] * this.width / 2];
      } else {
        p1 = [this.x1 - normedNormal[0] * this.width / 2, this.y1 + normedNormal[1] * this.width / 2];
        p2 = [this.x2 - normedNormal[0] * this.width / 2, this.y2 + normedNormal[1] * this.width / 2];
      }
      return `M ${p1[0]} ${p1[1]} L ${p2[0]} ${p2[1]}`;
    }
  },
  computed: {
    width(): number {
      switch (this.type) {
        case EdgeType.primary:
          return 0.02;
        case EdgeType.secondary:
          return 0.015;
        case EdgeType.tertiary:
          return 0.01;
        case EdgeType.residential:
          return 0.005;
        case EdgeType.service:
          return 0.002;
      }
    },
    path1(): string {
      if (this.x1 == this.x2 && this.y1 == this.y2) return "";

      return this.computePath(true);
    },
    path2(): string {
      if (this.x1 == this.x2 && this.y1 == this.y2) return "";
      return this.computePath(false);
    },
    path3(): string {
      if (this.x1 == this.x2 && this.y1 == this.y2) return "";
      return `${this.computePath(false)} L ${this.computePath(true).replace("M", "").split("L").reverse().join(" L ")} Z`;
    }
  }
});
</script>

<style lang="scss" scoped>
@import "../../style/map";

path {
  stroke-width: 0.001;
  stroke: darkgrey;

  &.road {
    stroke-width: 0;
    fill: darkgray;
  }
}
</style>
