# Tracking Simulator

Simulation of a sensor-net-based approach for tracking burglaries.

## [Simulation](./library)

The core of the simulation is captured in a library written in C++.
You can compile it either to WebAssembly or as a standalone binary.

**WASM Build** (requires [Emscripten](https://emscripten.org/))
```sh
cd library
mkdir build_wasm
cd build_wasm
emcmake cmake .. # optional with -DCMAKE_BUILD_TYPE=Debug for development
make
```

**Standalone Build**
```sh
cd library
mkdir build
cd build
cmake .. # optional with -DCMAKE_BUILD_TYPE=Debug for development
make
```

## [Frontend](./frontend)

The Web frontend is a GUI for the simulation written in Vue.
It requires a WebAssembly build of the library located in `library/build_wasm`.

The following snippets use `pnpm` as package manager, but can be replaced with other JS package managers as well.

**Development Server**
```sh
cd frontend
pnpm i
pnpm run dev
```

**Production Build**
```sh
cd frontend
pnpm i
pnpm run build
```
*Note: Since the library is multi-threaded you need to deliver the files with the [COOP and COEP headers](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/SharedArrayBuffer#security_requirements)*

