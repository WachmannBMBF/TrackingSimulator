<template>
  <SimulatorLayout>
    <template v-slot:main>
      <Visualization :nodes="nodes" :edges="edges" :bounds="bounds" :routers="routers" :path="path" :attacker="attacker"
                     :attackerStart="attackerStart" :refresh="refreshVisualization" :width="canvasSize()[0]"
                     :height="canvasSize()[1]" :highlight-attacker="highlightAttacker" :hide-attacker="hideAttacker"/>
    </template>
    <template v-slot:sidebar>
      <ControlPanel @mapParsed="updateMap" @placeRouters="placeRouters" @placeAttacker="placeAttacker" @tick="tick"
                    @highlight="highlightEvent" @unhighlight="unhighlightEvent" @setStrategy="setStrategy"
                    @resetRun="resetRun" @stop="stopSimulation" @run="run" @showAttacker="v => hideAttacker = !v"
                    :current-tick="tickCount" :current-run="currentRun" :events="events" :running="running"
                    :state="configState"/>
    </template>
  </SimulatorLayout>
</template>

<script lang="ts">
import {defineComponent} from "vue";
import SimulatorLayout from "@/components/SimulatorLayout.vue";
import Visualization from "@/components/Visualization.vue";
import ControlPanel from "@/components/ControlPanel.vue";
import type {SimulationEvent, Strategy} from '@/types';
import {Attacker, EdgeType, EventType, Router} from '@/types';
import Chasimulator from '@/chasimulator';

export default defineComponent({
  name: "MainView",
  components: {ControlPanel, SimulatorLayout, Visualization},
  data() {
    return {
      nodes: new Map<number, [number, number]>(),
      edges: [] as [number, number, EdgeType][],
      bounds: undefined as number[] | undefined,
      routers: [] as Router[],
      refreshVisualization: false,
      attacker: undefined as Attacker | undefined,
      attackerStart: undefined as [[number, number], number] | undefined,
      highlightAttacker: undefined as [number, number] | undefined,
      path: [] as [number, number][],
      events: [] as SimulationEvent[],
      tickCount: 0,
      currentRun: 0,
      configState: 0,
      running: false,
      hideAttacker: false,
    };
  },
  methods: {
    updateMap(nodes: Map<number, [number, number]>, edges: [number, number, EdgeType][], bounds: number[] | undefined) {
      // Draw map
      this.bounds = bounds;
      if (!this.bounds) return;
      const [width, height] = this.canvasSize();

      let i = 0;
      const reindexer = new Map<number, number>();
      this.nodes.clear();
      nodes.forEach((coords, n) => {
        let inEdges = false;
        for (const edge of edges) {
          if (edge[0] == n || edge[1] == n) {
            inEdges = true;
            break;
          }
        }
        if (!inEdges) return;

        const transformedCoords = this.transformCoords(coords, width, height);
        if (transformedCoords[0] < 0 || transformedCoords[0] > width ||
            transformedCoords[1] < 0 || transformedCoords[1] > height)
          return; // Stay within bounds

        reindexer.set(n, i);
        this.nodes.set(i++, transformedCoords);
      });
      this.edges = [];
      for (const edge of edges) {
        const v1 = reindexer.get(edge[0]);
        const v2 = reindexer.get(edge[1]);
        if (v1 && v2) this.edges.push([v1, v2, edge[2]]); // Only use edges where the endpoints are within the bounds
      }
      this.refreshVisualization = !this.refreshVisualization;
      this.configState = 1;

      console.log("Map size:", this.nodes.size);
    },
    placeRouters(n: number, r: number) {
      if (!this.edges) return;
      if (this.configState >= 1) this.configState = 2;
      else return;

      Chasimulator.full_reset();
      this.nodes.forEach((coords, n) => {
        const [x, y] = coords;
        Chasimulator.add_vertex(n, x, y);
      });
      for (const edge of this.edges) {
        Chasimulator.add_edge(edge[0], edge[1]);
      }
      Chasimulator.build_graph();

      this.routers = [];
      for (let i = 0; i < n; i++) {
        const router = new Router();
        router.id = i;
        Chasimulator.random_weighted_edge_new();
        router.edge = [Chasimulator.random_weighted_edge_v1(), Chasimulator.random_weighted_edge_v2()];
        router.fraction = Math.random();
        router.mode = 0;
        [router.innerRadius, router.outerRadius] = [Math.min(0.005, this.scale(r) / 8), this.scale(r)];
        this.routers.push(router);
      }
      for (const router of this.routers) {
        Chasimulator.add_router(router.id, router.edge[0], router.edge[1], router.fraction, router.outerRadius);
        router.index = Chasimulator.get_router_id_by_index(router.id);
      }

      this.events = [];
      this.refreshVisualization = !this.refreshVisualization;
    },
    placeAttacker(speed: number, txProb: number, min_path_length: number, raw = false) {
      if (!this.routers) return;
      if (this.configState >= 3) this.configState = 4;
      else return;

      Chasimulator.reset();

      this.tickCount = 0;
      this.events = [];
      for (const router of this.routers) {
        router.mode = 0;
      }

      const [width, height] = this.canvasSize();
      let success = false;

      for (let tries = 0; tries < 100; tries++) {
        // Build attacker
        this.attacker = new Attacker();
        const randomRouter = this.routers[Math.floor(Math.random() * this.routers.length)];
        this.attacker.edge = randomRouter.edge;
        this.attacker.fraction = randomRouter.fraction;
        this.attacker.speed = raw ? speed : this.scale(speed);
        this.attacker.txProb = raw ? txProb : txProb / 100;
        this.attacker.alphaRouterIndex = randomRouter.index;
        this.attacker.min_path_length = min_path_length / 100 * (width + height) / 2;

        // Find random target (uniformly sampled from all nodes)
        this.attacker.target = 0;
        let count = 0;
        const keyIterator = this.nodes.keys();
        for (const key of keyIterator) {
          if (Math.random() < 1 / ++count) this.attacker.target = key;
        }

        // Get attacker path
        success = Chasimulator.set_attacker(this.attacker.edge[0], this.attacker.edge[1], this.attacker.target,
            this.attacker.fraction, this.attacker.speed, this.attacker.txProb, this.attacker.alphaRouterIndex, this.attacker.min_path_length);
        if (!success) continue;

        this.path = [];
        const path_length = Chasimulator.get_path_length();
        for (let i = 0; i < path_length; i++) {
          const node = Chasimulator.get_path_node(i);
          this.path.push(this.nodes.get(node)!);
          this.attackerStart = [this.attacker.edge, this.attacker.fraction];
        }
        if (this.path.length) break;
      }
      if (!success || !this.path.length) {
        // todo GUI warning message
        alert("Found no path from attacker to target with sufficient length");
      }

      if (!raw) {
        this.updateRouters();
        this.tickCount = 1;
        this.refreshVisualization = !this.refreshVisualization;
      }
    },
    setStrategy(strat: Strategy): void {
      if (this.configState >= 2) this.configState = 3;
      else return;
      switch (strat.type) {
        case "sample":
          Chasimulator.set_sample_strategy(this.scale(strat.activationDistance));
          break;
        case "sliding-window":
          Chasimulator.set_sliding_window_strategy(this.scale(strat.activationDistance), strat.activationTime);
          break;
        case "k-smartest-neighbors":
          Chasimulator.set_k_smartest_neighbors_strategy(strat.k, this.scale(strat.distance), strat.lazy);
          break;
      }
    },
    highlightEvent(event: SimulationEvent): void {
      if (event.type != EventType.SimulationFinished && event.type != EventType.MultiRunFinished) {
        this.routers[event.router].mode |= 1;
        this.refreshVisualization = !this.refreshVisualization;
      }
      if (event.type == EventType.RouterDetects || event.type == EventType.RouterMisses) {
        this.highlightAttacker = event.position;
      }
    },
    unhighlightEvent(event: SimulationEvent): void {
      if (event.type != EventType.SimulationFinished && event.type != EventType.MultiRunFinished) {
        this.routers[event.router].mode &= ~1;
        this.refreshVisualization = !this.refreshVisualization;
      }
      if (event.type == EventType.RouterDetects || event.type == EventType.RouterMisses) {
        this.highlightAttacker = undefined;
      }
    },
    resetRun(updateVisualization = true): void {
      Chasimulator.reset();
      this.tickCount = 0;
      this.events = [];
      if (this.attacker && this.attackerStart) {
        this.attacker.edge = this.attackerStart[0];
        this.attacker.fraction = this.attackerStart[1];
        Chasimulator.set_attacker(this.attacker.edge[0], this.attacker.edge[1], this.attacker.target,
            this.attacker.fraction, this.attacker.speed, this.attacker.txProb, this.attacker.alphaRouterIndex, 0);
        this.configState = 4;
      }
      if (updateVisualization) {
        this.updateRouters();
        this.tickCount = 1;
        this.refreshVisualization = !this.refreshVisualization;
      }
    },
    stopSimulation(): void {
      this.running = false;
    },
    canvasSize(): [number, number] {
      if (!this.bounds) return [1, 1];
      const xSize = Math.abs(this.bounds[0] - this.bounds[2]);
      const ySize = Math.abs(this.bounds[1] - this.bounds[3]);
      if (xSize < ySize) return [.75, ySize / xSize];
      else return [xSize / ySize * 0.75, 1];
    },
    async tick(count?: number, delay?: number): Promise<void> {
      if (!count) count = 1;
      this.running = true;
      let infinite = false;
      if (count < 0) infinite = true;

      for (let i = 0; (i < count || infinite) && this.running && !Chasimulator.is_done(); i++) {
        // Perform a single tick and update the UI
        Chasimulator.do_tick();
        while (Chasimulator.pop_event()) {
          this.events.push({
            type: Chasimulator.event_is_router_detects() ? EventType.RouterDetects : EventType.RouterMisses,
            tick: Chasimulator.event_get_tick(),
            router: Chasimulator.event_router_get_index(),
            position: this.attacker ? this.edgeFraction(this.attacker.edge, this.attacker.fraction) : [0, 0]
          });
        }
        this.tickCount++;

        if (this.attacker) {
          this.attacker.edge = [Chasimulator.get_attacker_v1(), Chasimulator.get_attacker_v2()];
          this.attacker.fraction = Chasimulator.get_attacker_fraction();
        }

        this.updateRouters();

        if ((i < count - 1 || infinite) && delay) // Only sleep if this is not the last iteration
          await new Promise<void>(r => setTimeout(() => r(), delay));
      }
      this.running = false;
      if (Chasimulator.is_done()) {
        this.events.push({
          type: EventType.SimulationFinished,
          tick: this.tickCount,
          metrics: {
            activity: Chasimulator.metric_activity(),
            detection: Chasimulator.metric_detection(),
            last_tracking: Chasimulator.metric_last_tracking(),
            path: Chasimulator.metric_path()
          }
        });
      }
    },
    async run(runs: number, replaceAttacker: boolean, strategies: Strategy[]) {
      this.events = [];
      this.running = true;

      const metrics = new Map<string, {
        activity: number[],
        detection: number[],
        last_tracking: number[],
        path: number[]
      }>();
      for (const strategy of strategies) {
        metrics.set(strategy.type, {activity: [], detection: [], last_tracking: [], path: []});
      }

      for (this.currentRun = 0; this.currentRun < runs; this.currentRun++) {
        Chasimulator.reset();
        if (replaceAttacker) {
          this.placeAttacker(this.attacker?.speed ?? this.scale(10), this.attacker?.txProb ?? 0.1, this.attacker?.min_path_length ?? 0, true);
        }

        for (const strategy of strategies) {
          this.setStrategy(strategy);
          this.resetRun(false);
          this.tickCount++;

          while (!Chasimulator.is_done() && this.running) {
            Chasimulator.do_tick();
            this.tickCount++;
            if (((Chasimulator.get_tick() >> 4) & 1) == 0) {
              await new Promise<void>(r => setTimeout(() => r(), 0)); // We don't want to completely starve the event loop}
            }
          }
          if (Chasimulator.is_done()) {
            metrics.get(strategy.type)?.activity.push(Chasimulator.metric_activity());
            metrics.get(strategy.type)?.detection.push(Chasimulator.metric_detection());
            metrics.get(strategy.type)?.last_tracking.push(Chasimulator.metric_last_tracking());
            let p = -1;
            try {
              p = Chasimulator.metric_path();
            } catch {}
            metrics.get(strategy.type)?.path.push(p);
            if (p < 0) break;
          }
        }
      }

      this.running = false;
      this.events.push({
        type: EventType.MultiRunFinished,
        metrics
      });
    },
    updateRouters() {
      for (const router of this.routers) {
        const active = Chasimulator.get_router_active_by_index(router.index);
        if (active && !(router.mode & 2)) {
          this.events.push({
            type: EventType.RouterActive,
            tick: this.tickCount,
            router: router.index
          });
          router.mode |= 2;
        } else if (!active && router.mode & 2) {
          this.events.push({
            type: EventType.RouterInactive,
            tick: this.tickCount,
            router: router.index
          });
          router.mode &= ~2;
        }
      }
    },
    scale(unit: number): number {
      // Scales a fixed unit in dependency of the size of the map.
      if (!this.bounds) return 0;
      const ref = this.bounds[2] - this.bounds[0];
      return unit / ref / 40000 * this.canvasSize()[1];
    },
    transformCoords(coords: [number, number], width: number, height: number): [number, number] {
      if (!this.bounds) return [0, 0];

      let lat, lon, minlat, minlon, maxlat, maxlon;
      [lat, lon] = coords;
      [minlat, minlon, maxlat, maxlon] = this.bounds;
      return [(lat - minlat) / (maxlat - minlat) * width, (lon - minlon) / (maxlon - minlon) * height];
    },
    scaleCoords(coords: [number, number]): [number, number] {
      if (!this.bounds) return [0, 0];
      let lat, lon, minlat, minlon, maxlat, maxlon;
      [lat, lon] = coords;
      [minlat, minlon, maxlat, maxlon] = this.bounds;
      return [lat / Math.abs(maxlat - minlat), lon / Math.abs(maxlon - minlon)];
    },
    edgeFraction(edge: [number, number], fraction: number): [number, number] {
      const helper = (index: 0 | 1) => this.nodes.get(edge[0])![index] + fraction * (this.nodes.get(edge[1])![index] - this.nodes.get(edge[0])![index]);
      return [helper(0), helper(1)];
    }
  },
});
</script>

<style lang="scss">
body {
  overflow: hidden;
}
</style>
