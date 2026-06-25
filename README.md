# arimu-framework

🌏 [한국어](README.md) · [日本語](README.ja.md)

[EnTT](https://github.com/skypjack/entt) 위에 만든, C++20용 작은 **Bevy 스타일 ECS 프레임워크**.
시스템은 그냥 자유 함수이고, 무엇을 받을지는 그 매개변수의 *타입*
(`Res`, `ResMut`, `Query`, `EventReader`, `EventWriter`, `Commands`)으로 결정된다. 한 프레임은
고정된 페이즈 순서로, 활성 씬마다 시스템을 실행한다.

## 왜 만들었나

"제대로 된" 객체지향 방식 — 인터페이스, 의존성 주입(DI), 서비스 그래프를 배선해 조립하기 — 으로
게임을 계속 만들다 보니, 내 에너지의 대부분이 *아키텍처에 먹이를 주는 데* 쓰이고 정작 게임은 못
짜고 있다는 걸 깨달았다. DI 컨테이너, 라이프타임 관리, "이 의존성은 어디서 오는가" 하는 물음들이
정작 게임 로직에 들어가야 할 시간을 갉아먹었다. OOP 잡일이었고, 게임은 거의 진척이 없었다.

그래서 한 발 물러나 **ECS(Entity-Component-System)** 를 제대로 배우기로 했고, 그것을 *실제
게임에 써가며 배우기 위해* **arimu** 를 만들었다. 가설은 이렇다 — 데이터는 컴포넌트에 살고,
행동은 그 데이터를 처리하는 그냥 함수인 시스템에 살며, "의존성"은 **시스템이 매개변수 목록에서
요구하는 것** 이 된다. 타입으로 해결되고, 컨테이너도 주입 배선도 필요 없다. 프레임워크가 시스템에
선언한 그대로의 데이터만 넘겨주면 OOD 배관은 사라지고, 정작 중요한 부분 — 게임플레이 — 에 쓸
에너지가 남는다.

arimu는 그 결과물이다 — 처음부터 끝까지 읽어낼 수 있을 만큼 작고, 그 모델이 요점을 증명했기에
Bevy를 닮았으며, 스토리지는 빠르고 검증된 EnTT에 기대면서도 사용감은 내 것으로 유지했다.

이 프레임워크의 코어 설계는 처음부터 매뉴얼을 정독해 얻은 게 아니다. **AI에게 "최고의 ECS
아키텍처"를 제시하게 한 뒤 그 결과물을 리버스 엔지니어링하며**, 왜 이렇게 설계했는지를 거꾸로
캐묻는 방식으로 DOD(Data-Oriented Design) 패러다임을 최단기간에 흡수해 학습·구현했다. 즉
arimu는 "ECS를 배우는 과정" 자체를 AI와의 역설계로 압축한 결과물이기도 하다.

## 멘탈 모델

- **컴포넌트(Component)** 는 그냥 구조체 (EnTT가 저장).
- **리소스(Resource)** 는 읽고 쓰는 싱글턴 (`Res<T>` / `ResMut<T>`).
- **시스템(System)** 은 자유 함수. 매개변수가 의존성을 선언한다:

  ```cpp
  void Bump(Arimu::ResMut<Score> s) { s->v += 1; }   // 가변 Score를 요구하면 딱 그것만 받는다
  ```

- **쿼리(Query)** 는 주어진 컴포넌트 집합을 가진 엔티티를 순회한다 (`Query<Position, Velocity>`).
- **이벤트(Event)** 는 `EventWriter<E>` / `EventReader<E>` 를 통해 시스템 사이를 흐른다.
- **커맨드(Commands)** 는 구조 변경(spawn/despawn)을 지연 큐에 쌓아 안전한 지점에서 적용한다.
- **페이즈(Phase)** 는 각 프레임에 고정된 시스템 순서를 주고, **씬(Scene)** 은 서로 다른 시스템 집합을 실행하게 한다.

**렌더러/플랫폼 비종속.** 프레임워크는 창을 열지 않고 메인 루프도 갖지 않는다 — 시뮬레이션을 한
프레임씩 전진시킬 뿐이다(`App::Tick`). *당신의* 게임이 루프를 소유하고 `Tick()` 을 호출한다.
`arimu/` 안의 무엇도 어떤 렌더러에도 의존하지 않는다.

> **이 프레임워크 위에서 게임을 만드는 AI 어시스턴트는 [`USAGE_FOR_AI.md`](USAGE_FOR_AI.md) 를 읽을 것.**

## 한 프레임의 흐름

```
App::Tick(scene, dt)
  │
  └─ Schedule  — 활성 씬의 시스템을 고정된 페이즈 순서로 실행
       PreLogic → Input → FixedLogic → Logic → SceneTransition → Render → Cleanup
          │
          └─ System(…)   ← 매개변수 타입이 곧 의존성
          │               (Res / ResMut / Query / EventReader / EventWriter / Commands)
          │  read · write
          ▼
       World  — EnTT 레지스트리: Entity · Component · Resource
          ▲
          └─ Commands 가 쌓은 spawn/despawn 은 안전 지점에서 적용, Event 는 프레임 끝에 flush
```

## 폴더 안

```
arimu-framework/
├── arimu/              # 프레임워크 본체 (namespace Arimu, "arimu/App.hpp" 로 include) — 렌더러 의존 없음
├── third_party/entt/   # 벤더링된 단일 헤더 EnTT
├── examples/minimal/   # 튕기는 공 + 클릭 카운터 데모 (컴파일됨; 읽어볼 것)
├── CMakeLists.txt      # `arimu` 를 STATIC 라이브러리로 빌드
├── README.md           # 이 파일
└── USAGE_FOR_AI.md     # 매뉴얼 (멘탈 모델 + 규칙 + 쿡북)
```

## 요구사항

- **C++20** — 유일한 언어 요건. 프레임워크는 C++20-clean.
- **EnTT** — 벤더링됨, 설치할 것 없음.
- 렌더러·윈도잉·메인 루프 없음. 당신의 루프에서 구동한다.

## 설치 (벤더링)

1. `arimu-framework/` 폴더 전체를 게임 저장소에 복사.
2. 게임의 `CMakeLists.txt` 에 — 딱 두 줄:

   ```cmake
   add_subdirectory(arimu-framework)
   target_link_libraries(MyGame PRIVATE arimu)   # STATIC lib + include dirs + C++20, 모두 전이적
   ```

   C++ 표준과 `CMAKE_MSVC_RUNTIME_LIBRARY` 는 `add_subdirectory` *전* 에 설정해, `arimu` 가
   게임에 맞춰 빌드되게 한다 — `examples/minimal/CMakeLists.txt` 참고.

3. 코드에서:

   ```cpp
   #include "arimu/App.hpp"   // World, Schedule, System, Plugin, FixedTime 을 끌어옴
   ```

## 30초 맛보기

```cpp
struct Score { int v = 0; };                       // 리소스

void Bump(Arimu::ResMut<Score> s) { s->v += 1; }   // 시스템 — 매개변수가 곧 의존성

struct MyPlugin {                                  // 플러그인이 셋업을 묶는다
    static void Build(Arimu::App& app) {
        app.GetWorld().InsertResource<Score>();
        app.AddSystem(Bump, /*scene*/0, Arimu::Phase::Logic, "Bump");
    }
};

void Main() {                                      // 당신의 게임이 루프를 소유
    Arimu::App app;
    app.AddPlugin<MyPlugin>();
    while (/* 당신의 플랫폼 루프 */ true) {
        app.Tick(/*sceneIndex*/0, /*dt*/ 0.016f);
    }
}
```

완전히 실행 가능한 버전은 [`examples/minimal/Main.cpp`](examples/minimal/Main.cpp) 를,
전체 가이드는 [`USAGE_FOR_AI.md`](USAGE_FOR_AI.md) 를 참고.

## 사용처

arimu는 [`aima-framework`](https://github.com/jungminna03/aima-framework) 안에 벤더링된 ECS
코어로, aima는 그 위를 크로스플랫폼 호스트 루프 · SDL3 플랫폼 계층 · 핫리로드 · 추상 렌더러
인터페이스로 감싼다.

---

<sub>📝 이 README는 AI가 제작·수정하고 있습니다.</sub>
