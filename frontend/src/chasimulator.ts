declare const Module: {
    ccall(func: string, returnType?: string, argumentTypes?: string[], arguments?: any[]): any;
    cwrap(func: string, returnType?: string, argumentTypes?: string[]): (...args: any) => any;
};

const debug = false;
if (debug) {
    const textarea = document.createElement("textarea");
    const body = document.querySelector("body") as HTMLBodyElement;
    textarea.id = "code";
    body.appendChild(textarea);
}

export default {
    add_vertex(vertex: number, x: number, y: number): void {
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `add_vertex(${vertex}, ${x}, ${y});\n`;
        }
        Module.ccall("add_vertex", undefined, ["number", "number", "number"], [vertex, x, y]);
    },
    add_edge(v1: number, v2: number): void {
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `add_edge(${v1}, ${v2});\n`;
        }
        Module.ccall("add_edge", undefined, ["number", "number"], [v1, v2]);
    },
    build_graph(): void {
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `build_graph();\n`;
        }
        Module.ccall("build_graph");
    },
    set_sample_strategy(distance: number): void {
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `set_sample_strategy(${distance});\n`;
        }
        Module.ccall("set_sample_strategy", undefined, ["number"], [distance]);
    },
    set_sliding_window_strategy(distance: number, time: number): void {
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `set_sliding_window_strategy(${distance}, ${time});\n`;
        }
        Module.ccall("set_sliding_window_strategy", undefined, ["number", "number"], [distance, time]);
    },
    set_k_smartest_neighbors_strategy(k: number, distance: number, lazy: boolean): void {
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `set_k_smartest_neighbors_strategy(${k}, ${distance}, ${lazy});\n`;
        }
        Module.ccall("set_k_smartest_neighbors_strategy", undefined, ["number", "number", "boolean"], [k, distance, lazy]);
    },
    set_attacker(v1: number, v2: number, target: number, fraction: number, speed: number, tx_prob: number, alpha_router_index: number, min_path_length: number): boolean {
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `set_attacker(${v1}, ${v2}, ${target}, ${fraction}, ${speed}, ${tx_prob}, ${alpha_router_index}, ${min_path_length});\n`;
        }
        return Module.ccall("set_attacker", "boolean",
            ["number", "number", "number", "number", "number", "number", "number", "number"],
            [v1, v2, target, fraction, speed, tx_prob, alpha_router_index, min_path_length]);
    },
    get_attacker_v1(): number {
        return Module.ccall("get_attacker_v1", "number");
    },
    get_attacker_v2(): number {
        return Module.ccall("get_attacker_v2", "number");
    },
    get_attacker_fraction(): number {
        return Module.ccall("get_attacker_fraction", "number");
    },
    get_attacker_x(): number {
        return Module.ccall("get_attacker_x", "number");
    },
    get_attacker_y(): number {
        return Module.ccall("get_attacker_y", "number");
    },
    get_attacker_speed(): number {
        return Module.ccall("get_attacker_speed", "number");
    },
    get_attacker_tx_prob(): number {
        return Module.ccall("get_attacker_tx_prob", "number");
    },
    add_router(id: number, v1: number, v2: number, fraction: number, radius: number): void {
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `add_router(${id}, ${v1}, ${v2}, ${fraction}, ${radius});\n`;
        }
        Module.ccall("add_router", undefined, ["number", "number", "number", "number", "radius"],
            [id, v1, v2, fraction, radius]);
    },
    get_router_id_by_index(index: number): number {
        return Module.ccall("get_router_id_by_index", "number", ["number"], [index]);
    },
    get_router_v1_by_index(index: number): number {
        return Module.ccall("get_router_v1_by_index", "number", ["number"], [index]);
    },
    get_router_v2_by_index(index: number): number {
        return Module.ccall("get_router_v2_by_index", "number", ["number"], [index]);
    },
    get_router_fraction_by_index(index: number): number {
        return Module.ccall("get_router_fraction_by_index", "number", ["number"], [index]);
    },
    get_router_x_by_index(index: number): number {
        return Module.ccall("get_router_x_by_index", "number", ["number"], [index]);
    },
    get_router_y_by_index(index: number): number {
        return Module.ccall("get_router_y_by_index", "number", ["number"], [index]);
    },
    get_router_radius_by_index(index: number): number {
        return Module.ccall("get_router_radius_by_index", "number", ["number"], [index]);
    },
    get_router_active_by_index(index: number): boolean {
        return Module.ccall("get_router_active_by_index", "number", ["number"], [index]);
    },
    get_router_active_since_by_index(index: number): number {
        return Module.ccall("get_router_active_since_by_index", "number", ["number"], [index]);
    },
    get_router_index_by_id(id: number): number {
        return Module.ccall("get_router_index_by_id", "number", ["number"], [id]);
    },
    count_routers(): number {
        return Module.ccall("count_routers", "number");
    },
    metric_activity(): number {
        return Module.ccall("metric_activity", "number");
    },
    metric_detection(): number {
        return Module.ccall("metric_detection", "number");
    },
    metric_last_tracking(): number {
        return Module.ccall("metric_last_tracking", "number");
    },
    metric_path(): number {
        return Module.ccall("metric_path", "number");
    },
    get_path_length(): number {
        return Module.ccall("get_path_length", "number");
    },
    get_path_node(index: number): number {
        return Module.ccall("get_path_node", "number", ["number"], [index]);
    },
    pop_event(): boolean {
        return Module.ccall("pop_event", "boolean");
    },
    event_is_router_detects(): boolean {
        return Module.ccall("event_is_router_detects", "boolean");
    },
    event_is_router_misses(): boolean {
        return Module.ccall("event_is_router_misses", "boolean");
    },
    event_router_get_index(): number {
        return Module.ccall("event_router_get_index", "number");
    },
    event_get_tick(): number {
        return Module.ccall("event_get_tick", "number");
    },
    random_weighted_edge_new(): void {
        Module.ccall("random_weighted_edge_new");
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `random_weighted_edge_new();\n`;
        }
    },
    random_weighted_edge_v1(): number {
        return Module.ccall("random_weighted_edge_v1", "number");
    },
    random_weighted_edge_v2(): number {
        return Module.ccall("random_weighted_edge_v2", "number");
    },
    do_tick(): void {
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `do_tick();\n`;
        }
        Module.ccall("do_tick");
    },
    get_tick(): number {
        return Module.ccall("get_tick", "number");
    },
    is_done(): boolean {
        return Module.ccall("is_done", "boolean");
    },
    reset(): void {
        Module.ccall("reset");
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value += `reset();\n`;
        }
    },
    full_reset(): void {
        Module.ccall("full_reset");
        if (debug) {
            const code = document.getElementById("code") as HTMLTextAreaElement;
            code.value = `full_reset();\n`;
        }
    }
};
