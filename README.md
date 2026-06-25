# arimu-framework

🌏 [English](README.md) · [日本語](README.ja.md)

A small **Bevy-style ECS framework** for C++20, built on [EnTT](https://github.com/skypjack/entt).
Systems are plain free functions; what they receive is decided by their parameter *types*
(`Res`, `ResMut`, `Query`, `EventReader`, `EventWriter`, `Commands`). A frame runs systems in a
fixed phase order, per active scene.

## Why this exists

I kept building games the "proper" object-oriented way — interfaces, dependency injection,
wiring graphs of services together — and noticed I was spending most of my energy *feeding the
architecture* instead of writing the game. The DI container, the lifetime management, the "where
does this dependency come from" questions ate the hours that should have gone into actual game
logic. It was OOP busywork, and the game barely moved.

So I stepped back to learn **ECS (Entity-Component-System)** properly, and built **arimu** to
learn it *by using it* on real games. The bet: data lives in components, behavior lives in
systems that are just functions over that data, and "dependencies" become **what a system asks
for in its parameter list** — resolved by type, no container, no injection wiring. When the
framework hands a system exactly the data it declared and nothing else, the OOD plumbing
disappears and there's energy left for the part that matters: the gameplay.

arimu is the result — small enough to read end to end, Bevy-shaped because that model proved the
point, and EnTT-backed so the storage is fast and battle-tested while the ergonomics stay mine.

## Mental model

- **Components** are plain structs (stored by EnTT).
- **Resources** are singletons you read/write (`Res<T>` / `ResMut<T>`).
- **Systems** are free functions. Their parameters declare their dependencies:

  ```cpp
  void Bump(Arimu::ResMut<Score> s) { s->v += 1; }   // asks for mutable Score, gets exactly that
  ```

- **Queries** iterate entities that have a given component set (`Query<Position, Velocity>`).
- **Events** flow between systems via `EventWriter<E>` / `EventReader<E>`.
- **Commands** queue deferred structural changes (spawn/despawn) applied at a safe point.
- **Phases** give each frame a fixed system order; **scenes** let you run different system sets.

**Renderer/platform agnostic.** The framework does not open a window or own the main loop — it
only advances the simulation one frame at a time (`App::Tick`). *Your* game owns its loop and
calls `Tick()`. Nothing in `arimu/` depends on any renderer.

> **For an AI assistant building a game on this framework, read [`USAGE_FOR_AI.md`](USAGE_FOR_AI.md).**

## What's in here

```
arimu-framework/
├── arimu/              # the framework (namespace Arimu, include as "arimu/App.hpp") — NO renderer dependency
├── third_party/entt/   # vendored single-header EnTT
├── examples/minimal/   # bouncing-ball + click-counter demo (compiles; read it)
├── CMakeLists.txt      # builds `arimu` as a STATIC library
├── README.md           # this file
└── USAGE_FOR_AI.md     # the manual (mental model + rules + cookbook)
```

## Requirements

- **C++20** — the only language requirement. The framework is C++20-clean.
- **EnTT** — vendored, nothing to install.
- No renderer, no windowing, no main loop. You drive it from your own loop.

## Install (vendoring)

1. Copy the whole `arimu-framework/` folder into your game repo.
2. In your game's `CMakeLists.txt` — just two lines:

   ```cmake
   add_subdirectory(arimu-framework)
   target_link_libraries(MyGame PRIVATE arimu)   # STATIC lib + include dirs + C++20, all transitive
   ```

   Set your C++ standard and `CMAKE_MSVC_RUNTIME_LIBRARY` *before* `add_subdirectory` so `arimu`
   is built to match your game — see `examples/minimal/CMakeLists.txt`.

3. In code:

   ```cpp
   #include "arimu/App.hpp"   // pulls in World, Schedule, System, Plugin, FixedTime
   ```

## 30-second taste

```cpp
struct Score { int v = 0; };                       // a resource

void Bump(Arimu::ResMut<Score> s) { s->v += 1; }   // a system — its params ARE its dependencies

struct MyPlugin {                                  // a plugin groups setup
    static void Build(Arimu::App& app) {
        app.GetWorld().InsertResource<Score>();
        app.AddSystem(Bump, /*scene*/0, Arimu::Phase::Logic, "Bump");
    }
};

void Main() {                                      // YOUR game owns the loop
    Arimu::App app;
    app.AddPlugin<MyPlugin>();
    while (/* your platform loop */ true) {
        app.Tick(/*sceneIndex*/0, /*dt*/ 0.016f);
    }
}
```

See [`examples/minimal/Main.cpp`](examples/minimal/Main.cpp) for a complete, runnable version
and [`USAGE_FOR_AI.md`](USAGE_FOR_AI.md) for the full guide.

## Used by

arimu is the ECS core vendored inside [`aima-framework`](https://github.com/jungminna03/aima-framework),
which wraps it with a cross-platform host loop, SDL3 platform layer, hot-reload, and an abstract
renderer interface.
