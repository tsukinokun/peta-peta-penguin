# PetaPetaPenguin

TsukinoEngine（`External/TsukinoEngine`、submodule）を使った 2D ゲーム。

エンジンの規約・モジュール構成・ビルドコマンドは下記を参照:

@External/TsukinoEngine/CLAUDE.md

## ディレクトリ構成

```
premake5.lua                     ワークスペース定義（.build/ に vs2022 を生成）
open.bat                         premake 再生成 → .sln を開く
PetaPetaPenguin/
  include/PetaPetaPenguin/
    ECS/Components/              コンポーネント（素の struct）
    ECS/Components/Serialization/  cereal の save/load
    ECS/Systems/                 システム（ISystem 実装）
    Data/                        セーブデータ等
  src/
    ECS/System/                  システムの実装（include 側は Systems、src 側は System。単数形）
    Scene/PenguinGameScene.cpp   システム登録・コンポーネント登録・Prefab 生成
    WinMain.cpp
  Assets/Prefabs/                Prefab の JSON
  SaveData/                      実行時に書かれる
External/TsukinoEngine/          submodule
```

## 機能を追加する手順

**この6ステップが全体。これ以外を探索する必要はない。**

1. **コンポーネント** — `include/PetaPetaPenguin/ECS/Components/Xxx.hpp`
   **基底クラス無し・仮想関数無し・マクロ無しの素の struct**。名前空間は `PetaPetaPenguin::ECS`。
   他エンティティ参照は `Tsukino::ECS::EntityRef`、アセット参照は `Tsukino::Asset::AssetRef`。

2. **シリアライズ** — `include/PetaPetaPenguin/ECS/Components/Serialization/XxxSerialization.hpp`
   `save` / `load` を**コンポーネントと同じ名前空間**に自由関数として定義する（ADL で解決される）。
   コンポーネント本体のヘッダは cereal に依存させない（だから別ファイルに分ける）。

   ```cpp
   template <class Archive>
   void save(Archive& archive, const XxxComponent& xxx) {
       archive(cereal::make_nvp("textEntity", xxx.textEntity));
   }
   template <class Archive>
   void load(Archive& archive, XxxComponent& xxx) {
       archive(xxx.textEntity);
   }
   ```

   **`make_nvp` に書いたフィールドだけが JSON に出る。** 実行時にしか決まらない値
   （カウンタ、タイマーなど）は意図的に書かず、struct の既定値のまま残す。

3. **システム** — `include/.../ECS/Systems/XxxSystem.hpp` + `src/ECS/System/XxxSystem.cpp`
   `Tsukino::ECS::ISystem` を継承し `void Update(Tsukino::ECS::Registry&, float deltaTime)` を1つ override するだけ。
   `registry.View<T>().each(...)` で走査し、`registry.GetContext<Tsukino::EngineIntegration::EngineContext*>()` で
   renderer / window / inputSystem / assetManager / prefabFactory などを取る。

4. **登録** — `src/Scene/PenguinGameScene.cpp`
   - `context->prefabFactory->RegisterComponent<PetaPetaPenguin::ECS::XxxComponent>("XxxComponent");`
   - `m_scene.AddSystem(std::make_shared<...XxxSystem>(), static_cast<int>(SystemPriority::Xxx));`

   **登録漏れはビルドが通り、実行時に警告が出るだけ**（`Unknown component type written in Prefab`）。
   最も頻出する罠なので、必ずこの2行を確認する。

5. **Prefab JSON** — `Assets/Prefabs/<Entity>/`
   コンポーネント JSON を置き、同フォルダの `Prefab.json` と `Assets/Prefabs/PenguinGame.Group.json` に登録する。

6. **premake 再生成** — **ソースファイルを新規追加したときだけ** `open.bat` を実行する。
   `premake5.lua` の `files` はワイルドカード（`src/**.cpp`）なので premake5.lua 自体の編集は不要。
   **JSON だけの変更なら再ビルドも再生成も不要。**

## Prefab JSON の構造

3階層になっている。

### 1. グループ（`Assets/Prefabs/PenguinGame.Group.json`）
名前 → Prefab パス。**ここに書いた名前が `#EntityRef` の解決スコープ**になる。

```json
{ "Entities": [ { "key": "Camera2D", "value": "PetaPetaPenguin/Assets/Prefabs/Camera2D/Prefab.json" } ] }
```

### 2. Prefab 索引（`<Entity>/Prefab.json`）
登録名 → コンポーネント JSON パス。

```json
{ "Components": [ { "key": "SpriteComponent", "value": "PetaPetaPenguin/Assets/Prefabs/ClickCounterBackground/Sprite.json" } ] }
```

### 3. コンポーネント本体（`<Entity>/Sprite.json` など）
ルートキーは**登録名そのもの**。

```json
{ "ClickCounterComponent": { "textEntity": "#ClickCounterText" } }
```

### 型の対応

| C++ | JSON |
|---|---|
| `hlslpp::float2` | `{"x","y"}` |
| `hlslpp::float3` | `{"x","y","z"}` |
| `hlslpp::float4` / `quaternion` | `{"x","y","z","w"}` |
| `enum class` | 素の整数（`"blendMode": 0`） |
| `Tsukino::ECS::EntityRef` | 文字列 `"#EntityName"`（グループ全体の生成後に解決される） |
| `Tsukino::Asset::AssetRef` | パス文字列（`"PetaPetaPenguin/Assets/UI/bg.png"`、即座に AssetManager で解決） |
| `TransformComponent::parent` | 同じく `"#EntityName"` |

- フィールド名は `make_nvp` の第1引数と**完全一致**する必要がある
- パスは**作業ディレクトリ相対**。Debug ではリポジトリルート `C:\peta`、Release では exe の隣
- **エンジン組み込み**コンポーネントのフィールド一覧は `External/TsukinoEngine/Docs/components.md` を見る
  （ヘッダを2つ読むより速い）。公開 API の索引は同 `Docs/api-digest.md`
- **ゲーム固有**のコンポーネント（`ClickCounterComponent`, `PenguinAnimatorComponent`）は
  components.md には載らない。`PetaPetaPenguin/include/PetaPetaPenguin/ECS/Components/` を直接見る

## ビルドと実行

```
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" .build\PetaPetaPenguin.sln /p:Configuration=Debug /p:Platform=x64 /m /nologo /v:q /clp:ErrorsOnly;NoSummary
```

- `/v:q /clp:ErrorsOnly;NoSummary` を**必ず付ける**。成功時の出力は 0 行、失敗時はエラー行だけになる。
  付けないと数千行が出力される
- `.build/` を作り直した直後は NuGet の復元が必要。premake が吐くのは旧形式の `packages.config` なので
  `-t:restore` だけでは足りず `-p:RestorePackagesConfig=true` が要る:

```
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" .build\PetaPetaPenguin.sln -t:restore -p:RestorePackagesConfig=true -p:Configuration=Debug -p:Platform=x64 -nologo -v:m
```

- 実行ファイル: `bin\Debug\PetaPetaPenguin.exe`（作業ディレクトリはリポジトリルート）
- **このアプリはフルスクリーンで起動する**（`PenguinGameScene::OnInitialize` の `SetFullscreen(true)`）。
  安易に起動すると画面を占有するので注意する
- `.build/`, `bin/`, `bin-int/` は gitignore 済み

## 注意点

- **`External/TsukinoEngine/External/` は読まない・grep しない**（Effekseer / Jolt / entt / cereal / hlslpp）
- システムの実行順は `PenguinGameScene.cpp` の `SystemPriority` enum で管理している。
  `TransformSystem` は**2回**登録されている（ゲームロジック用に先頭、描画直前にもう一度）。
  これが無いと描画が1フレーム遅れ、ドラッグ中に文字と絵がずれる
- ログはデフォルトで `OutputDebugStringA` にしか出ない。実行結果を確認したいときは
  `Tsukino::Core::Log::SetLogFile("Logs/Tsukino.log")` を有効にする
- エンジン（submodule）を編集した場合、ゲーム側リポジトリの submodule ポインタ更新も必要
