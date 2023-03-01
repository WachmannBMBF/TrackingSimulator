<template>
  <div class="overflow-hidden">
    <VueZoomable>
      <svg xmlns="http://www.w3.org/2000/svg" :viewBox="`0 0 ${height} ${width}`" preserveAspectRatio="xMidYMid meet"
           id="visualizationSvg">
        <g v-if="active">
          <Road v-for="(e, i) in edges" :key="i" :x1="nodesGet(e[0], 0)" :y1="nodesGet(e[0], 1)"
                :x2="nodesGet(e[1], 0)" :y2="nodesGet(e[1], 1)" :type="e[2]"/>
          <Router v-for="router in refreshedRouters" :key="router.id"
                  :radius="router.outerRadius" :radius-inner="router.innerRadius"
                  :x="edgeFraction(router.edge, router.fraction)[0]"
                  :y="edgeFraction(router.edge, router.fraction)[1]"
                  :mode="router.mode"/>
          <AttackerPath v-if="path" v-show="!hideAttacker" :points="scaledPath" :refresh="refresh"/>
          <Attacker v-if="attacker" v-show="!hideAttacker" :x="edgeFraction(attacker.edge, attacker.fraction)[0]"
                    :y="edgeFraction(attacker.edge, attacker.fraction)[1]"/>
          <Attacker v-if="highlightAttacker" :x="highlightAttacker[0]" :y="highlightAttacker[1]" class="highlight"/>
        </g>
      </svg>
    </VueZoomable>
  </div>
</template>

<script lang="ts">
import {defineComponent} from "vue";
import {VueZoomable} from "vue-zoomable";
import Attacker from '@/components/map/Attacker.vue';
import Intersection from "@/components/map/Intersection.vue";
import Road from "@/components/map/Road.vue";
import Router from "@/components/map/Router.vue";
import {Router as RouterType, Attacker as AttackerType, EdgeType} from "@/types";
import AttackerPath from '@/components/map/AttackerPath.vue';

export default defineComponent({
  name: "Visualization",
  components: {AttackerPath, Attacker, Intersection, Road, Router, VueZoomable},
  props: {
    nodes: {
      type: Map as { new(): Map<number, [number, number]> },
      required: true
    },
    edges: {
      type: Array as () => [number, number, EdgeType][],
      required: true
    },
    bounds: {
      type: Array as () => number[],
      required: false
    },
    routers: {
      type: Array as () => RouterType[],
      required: true
    },
    refresh: {
      type: Boolean,
      required: false,
      default: false
    },
    attacker: {
      type: AttackerType,
      required: false
    },
    highlightAttacker: {
      type: Array as () => number[],
      required: false
    },
    attackerStart: {
      type: Array as () => ([number, number] | number)[],
      required: false
    },
    path: {
      type: Array as () => [number, number][],
      required: false
    },
    width: {
      type: Number,
      required: true
    },
    height: {
      type: Number,
      required: true
    },
    hideAttacker: {
      type: Boolean,
      required: false,
      default: false
    }
  },
  computed: {
    active(): boolean {
      return !!this.width && !!this.height;
    },
    refreshedRouters(): RouterType[] {
      this.refresh;
      return this.routers;
    },
    scaledPath(): [number, number][] | undefined {
      this.refresh;
      if (!this.path || !this.attackerStart) return;

      const scaledPath = [] as [number, number][];
      scaledPath.push(this.edgeFraction(this.attackerStart[0] as [number, number], this.attackerStart[1] as number));

      for (const p of this.path) {
        scaledPath.push([p[0], p[1]]);
      }

      return scaledPath;
    }
  },
  methods: {
    nodesGet(node: number, index: number): number {
      return this.nodes.get(node)![index];
    },
    edgeFraction(edge: [number, number], fraction: number): [number, number] {
      const helper = (index: 0 | 1) => this.nodes.get(edge[0])![index] + fraction * (this.nodes.get(edge[1])![index] - this.nodes.get(edge[0])![index]);
      return [helper(0), helper(1)];
    }
  }
});
</script>

<style lang="scss" scoped>
svg {
  width: 100%;
  margin: 10px;
  max-height: calc(100vh - 20px);
  max-width: 100%;
  cursor: grab;
}
</style>
