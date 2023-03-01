#include "evaluator.hpp"

ParsedOsm parseOsm(const char *filename) {
    rapidxml::file<> file(filename);
    rapidxml::xml_document<> doc;
    doc.parse<0>(file.data());
    map<long long, rapidxml::xml_node<> *> nodes;
    auto *osm = doc.first_node("osm");

    const float lonToKm = 71.47, latToKm = 111.19;

    // Parse bounds
    auto *bounds = osm->first_node("bounds");
    auto *minlatAttr = bounds->first_attribute("minlat");
    float minlat = strtof(minlatAttr->value(), nullptr) * latToKm;
    auto *minlonAttr = bounds->first_attribute("minlon");
    float minlon = strtof(minlonAttr->value(), nullptr) * lonToKm;
    auto *maxlatAttr = bounds->first_attribute("maxlat");
    float maxlat = strtof(maxlatAttr->value(), nullptr) * latToKm;
    auto *maxlonAttr = bounds->first_attribute("maxlon");
    float maxlon = strtof(maxlonAttr->value(), nullptr) * lonToKm;

    // Parse nodes
    for (auto *node = osm->first_node("node"); node; node = node->next_sibling("node")) {
        auto node_id = strtoll(node->first_attribute("id")->value(), nullptr, 10);
        nodes[node_id] = node;
    }

    vector<struct way> ways;
    // Parse ways
    for (auto *way = osm->first_node("way"); way; way = way->next_sibling("way")) {
        struct way wayStruct;
        bool isHighway = false;
        char *highwayType;
        size_t highwayTypeLen;
        for (auto *tag = way->first_node("tag"); tag; tag = tag->next_sibling("tag")) {
            if (strncmp(tag->first_attribute("k")->value(), "highway", tag->first_attribute("k")->value_size()) != 0)
                continue;
            isHighway = true;
            highwayType = tag->first_attribute("v")->value();
            highwayTypeLen = tag->first_attribute("v")->value_size();
        }
        if (!isHighway) continue;

        if (strncmp(highwayType, "primary", highwayTypeLen) != 0 &&
            strncmp(highwayType, "secondary", highwayTypeLen) != 0 &&
            strncmp(highwayType, "tertiary", highwayTypeLen) != 0 &&
            strncmp(highwayType, "residential", highwayTypeLen) != 0 &&
            strncmp(highwayType, "service", highwayTypeLen) != 0)
            continue;

        for (auto *nd = way->first_node("nd"); nd; nd = nd->next_sibling("nd")) {
            auto ref = strtoll(nd->first_attribute("ref")->value(), nullptr, 10);
            wayStruct.nodes.push_back(ref);
        }
        ways.emplace_back(wayStruct);
    }

    vector<pair<float, float>> indexedNodes;
    vector<pair<int, int>> edges;
    int index = 0;
    map<long long, int> nodeMapping;

    for (auto &way: ways) {
        int prev = -1;
        for (auto nd: way.nodes) {
            if (nodeMapping.find(nd) == nodeMapping.end()) {
                nodeMapping[nd] = index++;
                indexedNodes.emplace_back(
                        make_pair(strtof(nodes[nd]->first_attribute("lat")->value(), nullptr) * latToKm,
                                  strtof(nodes[nd]->first_attribute("lon")->value(), nullptr) * lonToKm));
            }
            int curr = nodeMapping[nd];
            if (prev >= 0) {
                edges.emplace_back(make_pair(prev, curr));
            }
            prev = curr;
        }
    }

    if (!indexedNodes.empty()) { // The condition is only to prevent crashes on errornous maps
        // Compute better bounds
        minlat = maxlat = indexedNodes[0].first;
        minlon = maxlon = indexedNodes[0].second;

        for (const auto &node: indexedNodes) {
            if (minlat > node.first) minlat = node.first;
            if (maxlat < node.first) maxlat = node.first;
            if (minlon > node.second) minlon = node.second;
            if (maxlon < node.second) maxlon = node.second;
        }
    }

    return {
            .bounds = {minlat, minlon, maxlat, maxlon},
            .nodes = indexedNodes,
            .edges = edges
    };
}

void runSimulator(Simulator &simulator) {
    while (!simulator.isDone()) {
        simulator.doTick();
    }
}

void saveResult(Simulator &simulator, const RunConfig &runConfig, ofstream &file) {
    file << runConfig.map << ",";
    file << runConfig.routerCount << ",";
    file << runConfig.att.tx_prob << ",";
    file << runConfig.strategy << ",";
    if (runConfig.strategy == STATIC) {
        file << runConfig.staticParameters.distance << ",";
    } else {
        file << ",";
    }
    if (runConfig.strategy == RADIUS) {
        file << runConfig.radiusParameters.time << ",";
        file << runConfig.radiusParameters.distance << ",";
    } else {
        file << "," << ",";
    }
    if (runConfig.strategy == SER) {
        file << runConfig.serParameters.distance << ",";
    } else {
        file << ",";
    }
    if (runConfig.strategy == SGR) {
        file << runConfig.sgrParameters.distance << ",";
    } else {
        file << ",";
    }
    if (runConfig.strategy == KSN) {
        file << runConfig.kSNParameters.k << ",";
        file << runConfig.kSNParameters.maxDist << ",";
    } else {
        file << "," << ",";
    }
    if (runConfig.strategy == RANDOM_RADIUS) {
        file << runConfig.randomRadiusParameters.time << ",";
        file << runConfig.randomRadiusParameters.distance << ",";
        file << runConfig.randomRadiusParameters.fraction << ",";
    } else {
        file << "," << ",";
    }
    if (runConfig.strategy == RANDOM_STREETDISTANCE) {
        file << runConfig.randomStreetdistanceParameters.time << ",";
        file << runConfig.randomStreetdistanceParameters.distance << ",";
        file << runConfig.randomStreetdistanceParameters.fraction << ",";
    } else {
        file << "," << ",";
    }
    file << simulator.metricActivity() << ",";
    file << simulator.metricDetection() << ",";
    file << simulator.metricLastTracking() << ",";
    auto pathMetric = simulator.metricPath3();
    file << pathMetric.matching << ",";
    file << pathMetric.targetDiff << ",";
    file << pathMetric.lengthDiff << endl;
}

int main(int argc, char **argv) {

    ProgramOptions programOptions = parseProgramOptions(argc, argv);

    ofstream results(programOptions.outputFile, ofstream::app);

    int max_iteration_retries = programOptions.num_iterations / 2;
    float radius = 0.02; // 20m
    float speed = 0.005; // 5m/s

    const int seed = programOptions.seed;

    RunConfig runConfig;
    for (const auto &mapFile: programOptions.maps) {
        runConfig.map = mapFile.c_str();

        auto map = parseOsm(runConfig.map);
        cout << mapFile << endl;
        cerr << "  Edges: " << map.edges.size() << endl;
        cerr << "  Nodes: " << map.nodes.size() << endl;
        cerr << "  Bounds: " << map.bounds[0] << "," << map.bounds[2] << " | " << map.bounds[1] << "," << map.bounds[3]
             << endl;

        // todo there might be an error with the min_distance or the path finding, this is partly fixed but I still have a suspicious feeling
        float min_distance = 0.5 * sqrtf((map.bounds[0] - map.bounds[2]) * (map.bounds[0] - map.bounds[2]) +
                                         (map.bounds[1] - map.bounds[3]) * (map.bounds[1] - map.bounds[3]));

        auto simulator = Simulator(seed);
        {
            int i = 0;
            for (auto node: map.nodes) {
                simulator.addVertex(i++, node.first, node.second);
            }
            for (auto edge: map.edges) {
                simulator.addEdge(edge.first, edge.second);
            }
            simulator.buildGraph();
        }

        for (auto routerCount = programOptions.routerMin; routerCount <= programOptions.routerMax; routerCount += programOptions.routerStep) {
            runConfig.routerCount = routerCount;

            simulator.halfReset();
            for (int i = 0; i < routerCount; i++) {
                auto edge = simulator.random_weighted_edge();
                simulator.addRouter(i, edge.first, edge.second, simulator.random_float(), radius);
            }

            for (auto pTx = programOptions.pTxMin; pTx <= programOptions.pTxMax; pTx += programOptions.pTxStep) {
                runConfig.att.tx_prob = pTx / 100.0;
                int max_iteration_retries_ctr = max_iteration_retries;

                for (int iteration = 0; iteration < programOptions.num_iterations; iteration++) {
                    // Attacker parameters are fixed in this loop
                    runConfig.att.alpha_router_index = simulator.random_int(simulator.countRouters() - 1);
                    Router alphaRouter = simulator.getRouterByIndex(runConfig.att.alpha_router_index);
                    runConfig.att.v1 = alphaRouter.edge.first;
                    runConfig.att.v2 = alphaRouter.edge.second;
                    runConfig.att.fraction = alphaRouter.fraction;
                    runConfig.att.target = -1;

                    // Find possible target
                    for (int tries = 0; tries < 1024; tries++) {
                        int target = simulator.random_int(map.nodes.size() - 1);
                        if (simulator.setAttacker(runConfig.att.v1, runConfig.att.v2, target, runConfig.att.fraction,
                                                  speed, runConfig.att.tx_prob,
                                                  runConfig.att.alpha_router_index, min_distance)) {
                            runConfig.att.target = target;
                            break;
                        }
                    }

                    if (runConfig.att.target == -1) {
                        cerr << "Could not find a valid attacker path. min_distance=" << min_distance << endl;
                        cerr << "bounds=[" << map.bounds[0] << ", " << map.bounds[1] << ", " << map.bounds[2] << ", "
                             << map.bounds[3] << "]" << endl;
                        if (max_iteration_retries_ctr > 0) {
                            max_iteration_retries_ctr--;
                            iteration--;
                        } else {
                            min_distance = 0.99 * min_distance;
                        }
                        continue;
                    }

#define performRun(strategy) {        \
    if (programOptions.dryRun) cout << "Skipping because of dry run" << endl; \
    else {                            \
    simulator.reset();                \
    simulator.setStrategy(strategy);  \
    simulator.setAttacker(runConfig.att.v1, runConfig.att.v2, runConfig.att.target, runConfig.att.fraction, speed, runConfig.att.tx_prob, \
                          runConfig.att.alpha_router_index, 0); \
    runSimulator(simulator);          \
    saveResult(simulator, runConfig, results);         \
    }\
}

                    // SER Strategy
                    runConfig.strategy = SER;
                    for (int range = programOptions.serRangeMin; range <= programOptions.serRangeMax; range += programOptions.serRangeStep) {
                        float distance = range / 1000.0;
                        runConfig.serParameters.distance = distance;
                        performRun(new SlidingEuclideanRadiusStrategy(distance));
                    }

                    // SGR Strategy
                    runConfig.strategy = SGR;
                    for (int range = programOptions.sgrRangeMin; range <= programOptions.sgrRangeMax; range += programOptions.sgrRangeStep) {
                        float distance = range / 1000.0;
                        runConfig.sgrParameters.distance = distance;
                        performRun(new SlidingGraphRadiusStrategy(distance));
                    }

                    // kSN Strategy
                    runConfig.strategy = KSN;
                    for (int k = programOptions.ksnKMin; k <= programOptions.ksnKMax; k += programOptions.ksnKStep) {
                        runConfig.kSNParameters.k = k;

                        for (int dist = programOptions.ksnDistMin; dist <= programOptions.ksnDistMax; dist += programOptions.ksnDistStep) {
                            runConfig.kSNParameters.maxDist = dist / 1000.0;
                            performRun(new kSmartestNeighborsStrategy(k, (float) dist));
                        }
                    }

                    // Random Radius Strategy
                    // todo

                    // Random Streetdistance Strategy
                    // todo
                }
            }
        }
    }

    results.close();

}

ProgramOptions parseProgramOptions(int argc, char **argv) {
    ProgramOptions programOptions;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("dry,d", po::value<bool>(&programOptions.dryRun)->default_value(false), "perform a dry run")
        ("map,m", po::value<vector<string>>(&programOptions.maps), "osm map files")
        ("num-iterations,n", po::value<int>(&programOptions.num_iterations), "number of iterations for each configuration")
        ("seed,s", po::value<int>(&programOptions.seed)->default_value(0), "seed")
        ("output,o", po::value<string>(&programOptions.outputFile)->default_value("results.csv"), "file name of csv output");

    desc.add_options()
        ("router-min", po::value<int>(&programOptions.routerMin))
        ("router-max", po::value<int>(&programOptions.routerMax))
        ("router-step", po::value<int>(&programOptions.routerStep));

    desc.add_options()
        ("ptx-min", po::value<int>(&programOptions.pTxMin))
        ("ptx-max", po::value<int>(&programOptions.pTxMax))
        ("ptx-step", po::value<int>(&programOptions.pTxStep));

    desc.add_options()
        ("ser-range-min", po::value<int>(&programOptions.serRangeMin))
        ("ser-range-max", po::value<int>(&programOptions.serRangeMax), "choose max < min to disable ser")
        ("ser-range-step", po::value<int>(&programOptions.serRangeStep));

    desc.add_options()
        ("sgr-range-min", po::value<int>(&programOptions.sgrRangeMin))
        ("sgr-range-max", po::value<int>(&programOptions.sgrRangeMax), "choose max < min to disable sgr")
        ("sgr-range-step", po::value<int>(&programOptions.sgrRangeStep));

    desc.add_options()
        ("ksn-k-min", po::value<int>(&programOptions.ksnKMin))
        ("ksn-k-max", po::value<int>(&programOptions.ksnKMax))
        ("ksn-k-step", po::value<int>(&programOptions.ksnKStep));

    desc.add_options()
        ("ksn-dist-min", po::value<int>(&programOptions.ksnDistMin))
        ("ksn-dist-max", po::value<int>(&programOptions.ksnDistMax))
        ("ksn-dist-step", po::value<int>(&programOptions.ksnDistStep));

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << endl;
    }
    return programOptions;
}