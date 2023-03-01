#ifndef CHASE_SIMULATOR_EVALUATOR_HPP
#define CHASE_SIMULATOR_EVALUATOR_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <cstring>

#include <boost/program_options.hpp>

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "Simulator.hpp"

using namespace std;
using namespace watchman::simulator;
namespace po = boost::program_options;

// Types

struct way {
    vector<long long> nodes;
    char type[32] = {0};
};

struct attackerParameters {
    int v1, v2, target;
    float fraction;
    int alpha_router_index;
    float tx_prob;
};

struct staticParameters {
    float distance;
};

struct radiusParameters {
    int time;
    float distance;
};

struct serParameters {
    float distance;
};

struct sgrParameters {
    float distance;
};

struct kSNParameters {
    int k;
    float maxDist;
};

struct randomRadiusParameters : radiusParameters {
    float fraction;
};

struct randomStreetdistanceParameters : radiusParameters {
    float fraction;
};

typedef struct {
    const char *map;
    int routerCount;
    struct attackerParameters att;
    const char *strategy;
    union {
        struct staticParameters staticParameters;
        struct radiusParameters radiusParameters;
        struct serParameters serParameters;
        struct sgrParameters sgrParameters;
        struct kSNParameters kSNParameters;
        struct randomRadiusParameters randomRadiusParameters;
        struct randomStreetdistanceParameters randomStreetdistanceParameters;
    };
} RunConfig;

typedef struct parsedOsm {
    float bounds[4];
    vector<pair<float, float>> nodes;
    vector<pair<int, int>> edges;
} ParsedOsm;

typedef struct programOptions {
    string outputFile;
    vector<string> maps;
    int routerMin, routerMax, routerStep;
    int pTxMin, pTxMax, pTxStep;
    int serRangeMin, serRangeMax = -1, serRangeStep;
    int sgrRangeMin, sgrRangeMax = -1, sgrRangeStep;
    int ksnKMin, ksnKMax = -1, ksnKStep;
    int ksnDistMin, ksnDistMax = -1, ksnDistStep;
    int num_iterations;
    int seed = 0;
    bool dryRun;
} ProgramOptions;

// Constants

const char *STATIC = "static";
const char *RADIUS = "radius";
const char *SER = "ser";
const char *SGR = "sgr";
const char *KSN = "ksn";
const char *RANDOM_RADIUS = "random_radius";
const char *RANDOM_STREETDISTANCE = "random_streetdistance";

// Functions

ParsedOsm parseOsm(const char *filename);
void runSimulator(Simulator &simulator);
void saveResult(Simulator &simulator, const RunConfig &runConfig);
ProgramOptions parseProgramOptions(int argc, char **argv);

#endif //CHASE_SIMULATOR_EVALUATOR_HPP
