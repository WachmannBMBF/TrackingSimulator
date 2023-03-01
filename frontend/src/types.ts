export class Router {
    id!: number;
    index!: number;
    edge!: [number, number];
    fraction!: number;
    innerRadius!: number;
    outerRadius!: number;
    mode = 0;
}

export class Attacker {
    edge!: [number, number];
    fraction!: number;
    speed!: number;
    txProb!: number;
    alphaRouterIndex!: number;
    target!: number;
    min_path_length!: number;
}

export enum EdgeType {
    primary = "primary",
    secondary = "secondary",
    tertiary = "tertiary",
    residential = "residential",
    service = "service"
}

export enum EventType {
    RouterDetects, RouterMisses, RouterActive, RouterInactive, SimulationFinished, MultiRunFinished
}

export type SimulationEvent = {
    type: EventType.RouterDetects | EventType.RouterMisses;
    tick: number;
    router: number;
    position: [number, number];
} | {
    type: EventType.RouterActive | EventType.RouterInactive;
    tick: number;
    router: number;
} | {
    type: EventType.SimulationFinished,
    tick: number,
    metrics: {
        activity: number,
        detection: number,
        last_tracking: number,
        path: number
    }
} | {
    type: EventType.MultiRunFinished,
    metrics: Map<string,{
        activity: number[],
        detection: number[],
        last_tracking: number[],
        path: number[]
    }>
};

export type Strategy = {
    type: "sample",
    activationDistance: number
} | {
    type: "sliding-window",
    activationTime: number,
    activationDistance: number
} | {
    type: "k-smartest-neighbors",
    k: number,
    distance: number,
    lazy: boolean
};
