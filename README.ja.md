# arimu-framework

🌏 [한국어](README.md) · [日本語](README.ja.md)

[EnTT](https://github.com/skypjack/entt) の上に作られた、C++20 向けの小さな **Bevy 風 ECS フレームワーク**。
システムはただのフリー関数で、何を受け取るかはその引数の *型*
（`Res`, `ResMut`, `Query`, `EventReader`, `EventWriter`, `Commands`）で決まる。1フレームは
固定されたフェーズ順で、アクティブなシーンごとにシステムを実行する。

## なぜ存在するのか

「正しい」オブジェクト指向のやり方 — インターフェース、依存性注入（DI）、サービスのグラフを
配線して組み上げる — でゲームを作り続けていて、自分のエネルギーの大半が *アーキテクチャに餌を
やる* ことに費やされ、肝心のゲームを書けていないと気づいた。DI コンテナ、ライフタイム管理、
「この依存はどこから来るのか」という問い。それらが本来ゲームロジックに注ぐべき時間を食い潰す。
OOP の雑務であり、ゲームはほとんど進まなかった。

そこで一歩下がって **ECS（Entity-Component-System）** をきちんと学ぶことにし、それを *実際の
ゲームで使いながら学ぶ* ために **arimu** を作った。賭けはこうだ — データはコンポーネントに宿り、
振る舞いはそのデータを処理するただの関数であるシステムに宿り、「依存」は **システムが引数リストで
要求するもの** になる。型で解決され、コンテナも注入の配線も要らない。フレームワークがシステムに
宣言した通りのデータだけを渡すと、OOD の配管は消え、本当に大事な部分 — ゲームプレイ — にエネルギーが残る。

arimu はその成果だ — 端から端まで読み切れるほど小さく、そのモデルが正しさを証明したから Bevy 風で、
ストレージは速く実績のある EnTT に支えられつつ、使い心地は自分のものに保っている。

このフレームワークのコア設計は、最初からマニュアルを読み込んで得たものではない。**AI に「最高の
ECS アーキテクチャ」を提示させ、その成果物をリバースエンジニアリングしながら**、なぜこう設計したのかを
逆に問い詰める方法で、DOD（Data-Oriented Design）パラダイムを最短期間で吸収して学習・実装した。
つまり arimu は「ECS を学ぶ過程」そのものを、AI との逆設計に圧縮した成果物でもある。

## メンタルモデル

- **Component** はただの構造体（EnTT が保存する）。
- **Resource** は読み書きするシングルトン（`Res<T>` / `ResMut<T>`）。
- **System** はフリー関数。その引数が依存を宣言する：

  ```cpp
  void Bump(Arimu::ResMut<Score> s) { s->v += 1; }   // 可変な Score を要求し、それだけを受け取る
  ```

- **Query** は指定したコンポーネント集合を持つエンティティを巡回する（`Query<Position, Velocity>`）。
- **Event** は `EventWriter<E>` / `EventReader<E>` を通じてシステム間を流れる。
- **Commands** は構造変更（spawn/despawn）を遅延キューに積み、安全な地点で適用する。
- **Phase** は各フレームに固定のシステム順を与え、**Scene** は異なるシステム集合を実行できる。

**レンダラー / プラットフォーム非依存。** フレームワークはウィンドウを開かず、メインループも持たない —
シミュレーションを1フレームずつ進めるだけ（`App::Tick`）。*あなたの* ゲームがループを所有して
`Tick()` を呼ぶ。`arimu/` の中身はどんなレンダラーにも依存しない。

> **このフレームワーク上でゲームを作る AI アシスタントは [`USAGE_FOR_AI.md`](USAGE_FOR_AI.md) を読むこと。**

## 1フレームの流れ

```
App::Tick(scene, dt)
  │
  └─ Schedule  — アクティブなシーンのシステムを固定フェーズ順で実行
       PreLogic → Input → FixedLogic → Logic → SceneTransition → Render → Cleanup
          │
          └─ System(…)   ← 引数の型がそのまま依存
          │               (Res / ResMut / Query / EventReader / EventWriter / Commands)
          │  read · write
          ▼
       World  — EnTT レジストリ: Entity · Component · Resource
          ▲
          └─ Commands が積んだ spawn/despawn は安全地点で適用、Event はフレーム末に flush
```

## 中身

```
arimu-framework/
├── arimu/              # フレームワーク本体（namespace Arimu、"arimu/App.hpp" で include）— レンダラー依存なし
├── third_party/entt/   # ベンダリングされた単一ヘッダ EnTT
├── examples/minimal/   # 跳ねるボール + クリックカウンタのデモ（コンパイル可能; 読むこと）
├── CMakeLists.txt      # `arimu` を STATIC ライブラリとしてビルド
├── README.md           # このファイル
└── USAGE_FOR_AI.md     # マニュアル（メンタルモデル + ルール + クックブック）
```

## 必要要件

- **C++20** — 唯一の言語要件。フレームワークは C++20-clean。
- **EnTT** — ベンダリング済み、インストール不要。
- レンダラー・ウィンドウ・メインループなし。自分のループから駆動する。

## インストール（ベンダリング）

1. `arimu-framework/` フォルダ全体を自分のゲームリポジトリにコピー。
2. ゲームの `CMakeLists.txt` に — たった2行：

   ```cmake
   add_subdirectory(arimu-framework)
   target_link_libraries(MyGame PRIVATE arimu)   # STATIC lib + include dirs + C++20、すべて推移的
   ```

   C++ 標準と `CMAKE_MSVC_RUNTIME_LIBRARY` は `add_subdirectory` の *前* に設定し、`arimu` が
   ゲームに合わせてビルドされるようにする — `examples/minimal/CMakeLists.txt` を参照。

3. コードでは：

   ```cpp
   #include "arimu/App.hpp"   // World, Schedule, System, Plugin, FixedTime を取り込む
   ```

## 30秒の味見

```cpp
struct Score { int v = 0; };                       // リソース

void Bump(Arimu::ResMut<Score> s) { s->v += 1; }   // システム — 引数が依存そのもの

struct MyPlugin {                                  // プラグインがセットアップをまとめる
    static void Build(Arimu::App& app) {
        app.GetWorld().InsertResource<Score>();
        app.AddSystem(Bump, /*scene*/0, Arimu::Phase::Logic, "Bump");
    }
};

void Main() {                                      // あなたのゲームがループを所有
    Arimu::App app;
    app.AddPlugin<MyPlugin>();
    while (/* 自分のプラットフォームループ */ true) {
        app.Tick(/*sceneIndex*/0, /*dt*/ 0.016f);
    }
}
```

完全で実行可能な版は [`examples/minimal/Main.cpp`](examples/minimal/Main.cpp) を、
詳しいガイドは [`USAGE_FOR_AI.md`](USAGE_FOR_AI.md) を参照。

## 使用例

arimu は [`aima-framework`](https://github.com/jungminna03/aima-framework) の中にベンダリングされた
ECS コアで、aima はそれをクロスプラットフォームのホストループ、SDL3 プラットフォーム層、
ホットリロード、抽象レンダラーインターフェースで包んでいる。

---

<sub>📝 この README は AI が制作・修正しています。</sub>
