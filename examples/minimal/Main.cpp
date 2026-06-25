// =============================================================================
// Arimu ECS framework — minimal example.
//
// A bouncing-ball + click-counter "game" that touches EVERY core primitive once:
//   Resource  : ClickCount, DemoFont, SceneCtl
//   Component : Position, Velocity
//   Event     : Clicked
//   Commands  : spawn a ball where you click (deferred structural change)
//   Query     : iterate balls
//   System    : free functions, auto-injected by parameter type
//   Plugin    : DemoPlugin bundles the resources + systems
//   Phase     : Input -> Logic -> Render order per frame
//
// Read this top-to-bottom alongside USAGE_FOR_AI.md. Every block is labelled
// with the cookbook section it demonstrates.
// =============================================================================
#include <Siv3D.hpp>
#include "arimu/App.hpp"

namespace demo {

// --- COMPONENTS (plain structs, no base class) -------------------------------
struct Position { s3d::Vec2 p; };
struct Velocity { s3d::Vec2 v; };

// --- RESOURCES (global singletons, one instance per type) --------------------
struct ClickCount { int32_t n = 0; };
struct DemoFont   { s3d::Font font; };

// SceneCtl exists only to satisfy App::Run's scene callbacks. This demo has a
// single scene (index 0), but the framework always asks the game "which scene
// is active?" — so we hand it a resource to answer from.
struct SceneCtl { uint8_t current = 0; };

// --- EVENT (one-frame message) -----------------------------------------------
struct Clicked { s3d::Vec2 at; };

constexpr int32_t W = 640;
constexpr int32_t H = 480;
constexpr double  R = 16.0;

// =============================================================================
// SYSTEMS — free functions. A parameter's TYPE is a request the framework fills:
//   EventWriter<T> / EventReader<T> / Res<T> / ResMut<T> / Query<...> / Commands
// =============================================================================

// [Phase::Input]  Siv3D input -> Event. Systems never touch other systems; they
// only read/write World. A left click becomes a Clicked event.
void CollectInput(Arimu::EventWriter<Clicked> out) {
    if (s3d::MouseL.down()) {
        out.Send({ s3d::Cursor::PosF() });
    }
}

// [Phase::Logic]  Consume the event to bump a resource counter.
void CountClicks(Arimu::EventReader<Clicked> in, Arimu::ResMut<ClickCount> count) {
    count->n += static_cast<int32_t>(in.Read().size());
}

// [Phase::Logic]  Consume the SAME event again (independent cursor) to spawn a
// ball. Structural change while we may be iterating views -> use Commands, which
// defers Create/AddComponent until the system returns.
void SpawnOnClick(Arimu::EventReader<Clicked> in, Arimu::Commands cmd) {
    for (const auto& c : in.Read()) {
        auto e = cmd.Create();
        cmd.AddComponent<Position>(e, Position{ c.at });
        cmd.AddComponent<Velocity>(e, Velocity{ s3d::RandomVec2(200.0) });
    }
}

// [Phase::Logic]  Integrate motion + bounce off the walls. Query<A, B> yields
// only entities that have BOTH components.
void MoveBalls(Arimu::Query<Position, Velocity> balls) {
    const double dt = s3d::Scene::DeltaTime();
    for (auto [e, pos, vel] : balls.each()) {
        pos.p += vel.v * dt;
        if (pos.p.x < R || pos.p.x > W - R) vel.v.x = -vel.v.x;
        if (pos.p.y < R || pos.p.y > H - R) vel.v.y = -vel.v.y;
        pos.p.x = s3d::Clamp(pos.p.x, R, W - R);
        pos.p.y = s3d::Clamp(pos.p.y, R, H - R);
    }
}

// [Phase::Render]  Read-only draw. Res<T> (const) for resources, Query for entities.
void DrawBalls(Arimu::Query<Position> balls) {
    for (auto [e, pos] : balls.each()) {
        s3d::Circle{ pos.p, R }.draw(s3d::HSV{ pos.p.x, 0.6, 0.95 });
    }
}

void DrawHud(Arimu::Res<DemoFont> font,
             Arimu::Res<ClickCount> count,
             Arimu::Query<Position> balls) {
    font->font(U"Clicks: {} | Balls: {}"_fmt(count->n, balls.size()))
        .draw(16, 12, s3d::ColorF{ 0.95 });
    font->font(U"click anywhere").drawAt(W / 2.0, H - 28.0, s3d::ColorF{ 0.6 });
}

// =============================================================================
// PLUGIN — installs everything. `static void Build(App&)` is the whole contract.
// =============================================================================
struct DemoPlugin {
    static void Build(Arimu::App& app) {
        using Arimu::Phase;
        auto& world = app.GetWorld();

        // Register resources (one instance per type, lives in World).
        world.InsertResource<ClickCount>();
        world.InsertResource<SceneCtl>();
        auto& f = world.InsertResource<DemoFont>();
        f.font = s3d::Font{ 24 };

        // Spawn a few starting balls. At Build time there is no system running,
        // so direct registry access is fine (no iteration-invalidation risk).
        auto& reg = world.Registry();
        for (int i = 0; i < 3; ++i) {
            auto e = reg.create();
            reg.emplace<Position>(e, Position{ { W / 2.0, H / 2.0 } });
            reg.emplace<Velocity>(e, Velocity{ s3d::RandomVec2(200.0) });
        }

        // Register systems into scene slot 0, in phase order.
        const uint8_t S = 0;
        app.AddSystem(CollectInput, S, Phase::Input,  "CollectInput");
        app.AddSystem(CountClicks,  S, Phase::Logic,  "CountClicks");
        app.AddSystem(SpawnOnClick, S, Phase::Logic,  "SpawnOnClick");
        app.AddSystem(MoveBalls,    S, Phase::Logic,  "MoveBalls");
        app.AddSystem(DrawBalls,    S, Phase::Render, "DrawBalls");
        app.AddSystem(DrawHud,      S, Phase::Render, "DrawHud");
    }
};

// --- SCENE POLICY (game-provided; framework never hardcodes scene policy) -----
uint8_t GetScene(const Arimu::World& w) { return w.GetResource<SceneCtl>().current; }
void    ApplyScene(Arimu::World&)       { /* single scene — nothing to switch */ }

} // namespace demo

// The GAME owns the platform: window setup + the main loop. The framework is
// renderer-agnostic; we just feed it one Tick() per frame with the active scene
// and this frame's delta time. Apply any pending scene change after Tick().
void Main() {
    s3d::Window::Resize(demo::W, demo::H);
    s3d::Window::SetTitle(U"Arimu Minimal");
    s3d::Scene::SetBackground(s3d::ColorF{ 0.07, 0.08, 0.12 });

    Arimu::App app;
    app.AddPlugin<demo::DemoPlugin>();

    while (s3d::System::Update()) {
        const uint8_t scene = demo::GetScene(app.GetWorld());
        app.Tick(scene, s3d::Scene::DeltaTime());
        demo::ApplyScene(app.GetWorld());
    }
}
