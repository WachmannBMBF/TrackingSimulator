<template>
  <div class="container-fluid control-panel pt-4">
    <nav class="navbar navbar-light bg-light mb-3">
      <div class="container-fluid">
        <span class="navbar-brand">Control Panel</span>
        <div>
          <ul class="nav nav-tabs justify-content-center">
            <li class="nav-item" @click="mode = 'settings'">
              <a class="nav-link" :class="{active: mode == 'settings'}">Settings</a>
            </li>
            <li class="nav-item" @click="mode = 'events'">
              <a class="nav-link" :class="{active: mode == 'events'}">Events</a>
            </li>
          </ul>
        </div>
      </div>
    </nav>

    <div v-show="mode == 'settings'">
      <div class="row">
        <div class="col-12"><h3>Status</h3></div>
        <div class="col-6">
          <div class="col-12"><span>{{ state > 0 ? "✅" : "❌" }}</span> Map loaded</div>
          <div class="col-12"><span>{{ state > 1 ? "✅" : "❌" }}</span> Placed Routers</div>
          <div class="col-12"><span>{{ state > 2 ? "✅" : "❌" }}</span> Set Strategy</div>
          <div class="col-12"><span>{{ state > 3 ? "✅" : "❌" }}</span> Set Attacker</div>
        </div>
        <div class="col-6">
          <div class="form-check form-switch">
            <input class="form-check-input" type="checkbox" role="switch" id="showAttacker" v-model="showAttacker">
            <label class="form-check-label" for="showAttacker">Show Attacker</label>
          </div>
        </div>
      </div>

      <hr>

      <div class="row">
        <div class="col-12"><h3>Map</h3></div>
        <form @submit.prevent="loadOsm">
          <label for="osm_upload" class="form-label">Load map from OSM export:</label><br>
          <input type="file" class="form-control" id="osm_upload"><br>
          <div class="row align-items-center">
            <div class="col-6">
              <button class="btn btn-secondary w-100" type="submit">Load file</button>
            </div>
            <div class="col-6">
              <button class="btn btn-secondary w-100" type="button" @click="loadDefault">Or load default map</button>
            </div>
            <div class="col-12">
              <button class="btn btn-secondary w-100 mt-3" type="button" @click="loadLarge">Or load large map (might be
                very slow)
              </button>
            </div>
          </div>
        </form>
      </div>

      <hr>

      <div class="row">
        <div class="col-12"><h3>Router</h3></div>
        <div class="col-12 mb-3">
          <label class="form-label">Amount</label>
          <input class="form-control" type="number" min="0" step="1" v-model="routerCount">
        </div>
        <div class="col-12 mb-3">
          <label class="form-label">Reception Distance</label>
          <input class="form-control" type="number" min="1" step="1" v-model="routerRadius">
        </div>
        <div class="col-12">
          <button type="button" class="btn btn-primary w-100" @click="$emit('placeRouters', routerCount, routerRadius)">
            Place routers
          </button>
        </div>
      </div>

      <hr>

      <div class="row">
        <div class="col-12"><h3>Strategy</h3></div>
        <div class="col-12 mb-4">
          <ul class="nav nav-tabs">
            <li class="nav-item" @click="strategy = 'sample'">
              <a class="nav-link" :class="{active: strategy == 'sample'}">Static</a>
            </li>
            <li class="nav-item" @click="strategy = 'sliding-window'">
              <a class="nav-link" :class="{active: strategy == 'sliding-window'}">Radius</a>
            </li>
            <li class="nav-item" @click="strategy = 'k-smartest-neighbors'">
              <a class="nav-link" :class="{active: strategy == 'k-smartest-neighbors'}">k-smartest-neighbors</a>
            </li>
          </ul>
        </div>

        <div class="col-12" v-show="strategy == 'sample'">
          <div class="col-12 mb-3">
            <label class="form-label">Activation Distance</label>
            <input class="form-control" type="number" min="1" step="1" v-model="smplActivationDistance">
          </div>
        </div>

        <div class="col-12" v-show="strategy == 'sliding-window'">
          <div class="col-12 mb-3">
            <label class="form-label">Activation Distance</label>
            <input class="form-control" type="number" min="1" step="1" v-model="swActivationDistance">
          </div>
          <div class="col-12 mb-3">
            <label class="form-label">Maximum Activation Time</label>
            <input class="form-control" type="number" min="1" step="1" v-model="swActivationTime">
          </div>
        </div>

        <div class="col-12" v-show="strategy == 'k-smartest-neighbors'">
          <div class="col-12 mb-3">
            <label class="form-label">K</label>
            <input class="form-control" type="number" min="1" step="1" v-model="ksnK">
          </div>
          <div class="col-12 mb-3">
            <label class="form-label">Maximum Activation Distance</label>
            <input class="form-control" type="number" min="1" step="1" v-model="ksnDistance">
          </div>
          <div class="col-12 mb-3">
            <div class="form-check">
              <input class="form-check-input" type="checkbox" v-model="ksnLazy">
              <label class="form-check-label">Lazy generation of neighborhood lists</label>
            </div>
          </div>
        </div>

        <div class="col-12">
          <button type="button" class="btn btn-primary w-100" @click="setStrategy">
            Set Strategy <span v-show="strategySetSuccessful">✅️</span>
          </button>
        </div>
      </div>

      <hr>

      <div class="row">
        <div class="col-12"><h3>Attacker</h3></div>
        <div class="col-12 mb-3">
          <label class="form-label">Transmission Probability per tick</label>
          <input class="form-control" type="number" min="0" max="100" step="0.1" v-model="attackerProb">
        </div>
        <div class="col-12 mb-3">
          <label class="form-label">Speed</label>
          <input class="form-control" type="number" min="1" step="1" v-model="attackerSpeed">
        </div>
        <div class="col-12 mb-3">
          <label class="form-label">Minimum Path Length (as percentage of map diagonal)</label>
          <input class="form-control" type="number" min="0" step="1" v-model="attackerMinPathLength">
        </div>
        <div class="col-12">
          <button type="button" class="btn btn-primary w-100"
                  @click="$emit('placeAttacker', attackerSpeed, attackerProb, attackerMinPathLength)">
            Place attacker
          </button>
        </div>
      </div>

      <hr>

      <div class="row">
        <div class="col-12 d-flex justify-content-between align-items-center">
          <h3>Simulation (Single Run)</h3>
          <div class="mb-2">
            <span class="badge bg-primary">t={{ currentTick }}</span>
          </div>
        </div>
        <div class="col-12 mb-3">
          <label class="form-label">Tick Count (set to -1 to run until end)</label>
          <input class="form-control" type="number" min="1" step="1" v-model="tickCount">
        </div>
        <div class="col-12 mb-3">
          <label class="form-label">Tick Delay</label>
          <input class="form-control" type="number" min="0" step="1" v-model="delay">
        </div>
        <div class="col-12">
          <button type="button" class="btn w-100 mb-2" :disabled="state < 4"
                  :class="{'btn-success': !running, 'btn-danger': !!running}"
                  @click="(!running && $emit('tick', tickCount, delay)) || (running && $emit('stop'))">
            {{ !running ? "Run simulation" : "Stop simulation" }}
          </button>
          <button type="button" class="btn btn-secondary w-100" :disabled="state < 4" @click="$emit('resetRun')">
            Reset current run
          </button>
        </div>
      </div>

      <hr>

      <div class="row">
        <div class="col-12 d-flex justify-content-between align-items-center">
          <h3>Simulation (Multiple Runs)</h3>
          <div class="mb-2">
            <span class="badge bg-primary">r={{ currentRun }}</span>
          </div>
        </div>
        <div class="col-12 mb-3">
          <label class="form-label">Runs</label>
          <input class="form-control" type="number" min="1" step="1" v-model="runCount">
        </div>
        <div class="col-12 mb-3">
          <div class="form-check">
            <input class="form-check-input" type="checkbox" v-model="replaceAttacker">
            <label class="form-check-label">Place attacker differently between runs</label>
          </div>
        </div>
        <div class="col-12 mb-3">
          <label class="form-label">Strategies</label>
          <select class="form-control" multiple v-model="strategySelection">
            <option value="sample">Static</option>
            <option value="sliding-window">Radius Actication</option>
            <option value="k-smartest-neighbors">k-smartest-neighbors</option>
          </select>
        </div>
        <div class="col-12">
          <button type="button" class="btn w-100 mb-2" :disabled="state < 4"
                  :class="{'btn-success': !running, 'btn-danger': !!running}"
                  @click="(!running && $emit('run', runCount, replaceAttacker, getStrategySelection())) || (running && $emit('stop'))">
            {{ !running ? "Run simulation" : "Stop simulation" }}
          </button>
        </div>
      </div>
      <div class="mt-5"></div>
    </div>
    <div v-show="mode == 'events'">
      <div class="row">
        <div class="col-12"><h3>Events</h3></div>
        <div class="col-12 d-flex flex-column-reverse">
          <div class="alert d-flex justify-content-between align-items-center" v-for="event of events"
               @mouseenter="$emit('highlight', event)" @mouseleave="$emit('unhighlight', event)"
               :class="eventClasses(event)">
            <span class="w-100">
              <strong>{{ eventName(event) }}</strong>
              <span v-if="event.type == EventType().SimulationFinished">
                <br>
                A{{ event.metrics.activity.toFixed(3) }} / D{{ event.metrics.detection.toFixed(3) }} /
                L{{ event.metrics.last_tracking.toFixed(3) }} / P{{ event.metrics.path.toFixed(3) }}
              </span>
              <span v-else-if="event.type == EventType().MultiRunFinished">
                <br>
                <button type="button" class="btn btn-primary" @click="exportCsv(event.metrics)">Export CSV</button>
                <br>
                <line-chart v-for="graph of metricGraphs(event.metrics)" :key="graph.name" :data="graph.data"
                            xtitle="Run" :ytitle="graph.name" legend="top" class="mt-3 pe-2"/>
              </span>
            </span>
            <span v-if="'tick' in event" class="badge bg-primary">t={{ event.tick }}</span>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script lang="ts">
import {defineComponent} from "vue";
import type {SimulationEvent, Strategy} from '@/types';
import {EdgeType, EventType} from '@/types';

type Strategies = "sliding-window" | "sample" | "k-smartest-neighbors";

export default defineComponent({
  name: "ControlPanel",
  data() {
    return {
      routerCount: 70,
      routerRadius: 20,
      attackerSpeed: 5,
      attackerProb: 25,
      attackerMinPathLength: 80,
      tickCount: 100,
      delay: 30,
      runCount: 10,
      replaceAttacker: true,
      showAttacker: true,
      mode: "settings",
      strategy: "sliding-window" as Strategies,
      strategySelection: [] as Strategies[],
      swActivationDistance: 200,
      swActivationTime: 125,
      smplActivationDistance: 450,
      ksnK: 3,
      ksnDistance: 800,
      ksnLazy: true,
      strategySetSuccessful: false,
    };
  },
  props: {
    currentTick: Number,
    currentRun: Number,
    events: {
      type: Array as () => SimulationEvent[],
      default: []
    },
    state: {
      type: Number,
      required: true
    },
    running: Boolean,
    refresh: Boolean
  },
  emits: [
    "mapParsed",
    "placeRouters",
    "placeAttacker",
    "tick",
    "run",
    "highlight",
    "unhighlight",
    "setStrategy",
    "resetRun",
    "stop",
    "showAttacker"
  ],
  watch: {
    showAttacker(neew) {
      this.$emit("showAttacker", neew);
    }
  },
  methods: {
    EventType(): typeof EventType {
      return EventType;
    },
    eventName(e: SimulationEvent): string {
      switch (e.type) {
        case EventType.RouterInactive:
          return `Router ${e.router} inactive`;
        case EventType.RouterActive:
          return `Router ${e.router} active`;
        case EventType.RouterMisses:
          return `Router ${e.router} misses attacker`;
        case EventType.RouterDetects:
          return `Router ${e.router} detects attacker`;
        case EventType.SimulationFinished:
          return `Simulation finished`;
        case EventType.MultiRunFinished:
          return `All runs finished`;
        default:
          return "Unknown Event";
      }
    },
    eventClasses(event: SimulationEvent) {
      return {
        'alert-primary': event.type == EventType.RouterActive || event.type == EventType.RouterInactive,
        'alert-danger': event.type == EventType.RouterMisses,
        'alert-success': event.type == EventType.RouterDetects,
        'alert-info': event.type == EventType.SimulationFinished || event.type == EventType.MultiRunFinished,
      };
    },
    metricGraphs(data: Map<string, { activity: number[], detection: number[], last_tracking: number[], path: number[] }>):
        any[] {
      const graphs = [];
      const activityData = [] as { name: string, data: number[][] }[];
      const detectionData = [] as { name: string, data: number[][] }[];
      const lastTrackingData = [] as { name: string, data: number[][] }[];
      const pathData = [] as { name: string, data: number[][] }[];
      data.forEach((val, name) => {
        activityData.push({name, data: val.activity.map((v, i) => [i + 1, v])});
        detectionData.push({name, data: val.detection.map((v, i) => [i + 1, v])});
        lastTrackingData.push({name, data: val.last_tracking.map((v, i) => [i + 1, v])});
        pathData.push({name, data: val.path.map((v, i) => [i + 1, v])});
      });
      graphs.push({
        name: "Activity",
        data: activityData
      }, {
        name: "Detection",
        data: detectionData
      }, {
        name: "Last Tracking",
        data: lastTrackingData
      }, {
        name: "Path",
        data: pathData
      });
      return graphs;
    },
    setStrategy() {
      this.strategySetSuccessful = true;
      setTimeout(() => this.strategySetSuccessful = false, 3000);
      this.$emit("setStrategy", this.getStrat(this.strategy));
    },
    getStrat(stratName: Strategies): Strategy {
      switch (stratName) {
        case "sample":
          return {
            type: stratName,
            activationDistance: this.smplActivationDistance
          };
        case "sliding-window":
          return {
            type: stratName,
            activationDistance: this.swActivationDistance,
            activationTime: this.swActivationTime
          };
        case "k-smartest-neighbors":
          return {
            type: stratName,
            k: this.ksnK,
            distance: this.ksnDistance,
            lazy: this.ksnLazy
          };
      }
    },
    getStrategySelection(): Strategy[] {
      const strategies: Strategy[] = [];
      for (const strategy of this.strategySelection) {
        strategies.push(this.getStrat(strategy));
      }
      return strategies;
    },
    exportCsv(data: Map<string, { activity: number[], detection: number[], last_tracking: number[], path: number[] }>) {
      const csvParts = ["strategy,activity,detection,last_tracking,path\n"];
      data.forEach((v, k) => {
        for (let i = 0; i < v.activity.length; i++) {
          csvParts.push(`${k},${v.activity[i]},${v.detection[i]},${v.last_tracking[i]},${v.path[i]}\n`);
        }
      });
      const blob = new Blob(csvParts, {type: "text/csv"});
      const uri = URL.createObjectURL(blob);
      open(uri, '_blank');
      setTimeout(() => URL.revokeObjectURL(uri), 5000);
    },
    loadOsm() {
      const input = document.getElementById("osm_upload") as HTMLInputElement;
      if (input.files && input.files.length > 0) {
        const file = input.files[0];
        const reader = new FileReader();
        reader.addEventListener("load", e => {
          const xml = e.target?.result;
          if (typeof xml === "string") {
            const doc = new DOMParser().parseFromString(xml, "application/xml");
            this.parseOsm(doc);
          }
        });
        reader.readAsText(file);
      }
    },
    loadDefault() {
      this.loadFixed("map.osm");
    },
    loadLarge() {
      this.loadFixed("villages.osm");
    },
    loadFixed(file: string) {
      fetch(file).then(f => f.blob()).then(blob => {
        const reader = new FileReader();
        return new Promise<Document>((resolve, reject) => {
          reader.addEventListener("load", e => {
            const xml = e.target?.result;
            if (typeof xml === "string") {
              const doc = new DOMParser().parseFromString(xml, "application/xml");
              resolve(doc);
            }
            reject(e);
          });
          reader.readAsText(blob);
        });
      }).then(doc => this.parseOsm(doc)).catch(e => console.warn(`Cannot load default map: ${e}`));
    },
    parseOsm(xml: Document) {
      const nodes = new Map<number, [number, number]>();
      const edges: [number, number, EdgeType][] = [];

      const boundsNode = xml.querySelector("osm > bounds");
      let bounds: number[] | undefined = undefined;
      if (boundsNode) {
        let minlat, minlon, maxlat, maxlon: string | undefined;
        minlat = boundsNode.attributes.getNamedItem("minlat")?.value;
        minlon = boundsNode.attributes.getNamedItem("minlon")?.value;
        maxlat = boundsNode.attributes.getNamedItem("maxlat")?.value;
        maxlon = boundsNode.attributes.getNamedItem("maxlon")?.value;
        if (!minlat) minlat = "0";
        if (!minlon) minlon = "0";
        if (!maxlat) maxlat = "0";
        if (!maxlon) maxlon = "0";
        bounds = [minlon, maxlat, maxlon, minlat].map(parseFloat);
      }

      for (const node of xml.querySelectorAll("osm > node")) {
        const attr = node.attributes;
        const id = attr.getNamedItem("id")?.value;
        const lat = attr.getNamedItem("lat")?.value;
        const lon = attr.getNamedItem("lon")?.value;
        if (id && lat && lon) {
          nodes.set(parseInt(id), [parseFloat(lon), parseFloat(lat)]);
        }
      }

      for (const type in EdgeType) {
        const edgeType = type as EdgeType;
        for (const tag of xml.querySelectorAll(`osm > way > tag[k=highway][v=${edgeType}]`)) {
          const way = tag.parentNode;
          if (way) {
            const nds = [] as number[];
            for (const nd of way.querySelectorAll("nd")) {
              const val = nd.attributes.getNamedItem("ref")?.value;
              if (val) {
                nds.push(parseInt(val));
              }
            }
            for (let i = 1; i < nds.length; i++) {
              edges.push([nds[i - 1], nds[i], edgeType]);
            }
          }
        }
      }

      this.$emit("mapParsed", nodes, edges, bounds);
    },
    avg(arr: number[]): number {
      let sum = 0;
      arr.forEach(x => sum += x);
      return sum / arr.length;
    }
  }
});
</script>

<style lang="scss" scoped>
.control-panel {
  border-left: 5px solid lightgrey;
  height: 100%;
  max-height: 100vh;
  overflow-y: auto;
  margin-right: 0;

  .nav-item {
    cursor: pointer;
  }

  .badge {
    height: fit-content;
  }
}
</style>
