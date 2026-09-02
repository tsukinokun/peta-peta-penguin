//-------------------------------------------------------------
//! @file    PenguinGameScene.cpp
//! @brief   ペンギンゲームのシーンの実装
//! @author  山﨑愛
//-------------------------------------------------------------
#include <PetaPetaPenguin/Scene/PenguinGameScene.hpp>

#include <PetaPetaPenguin/ECS/Components/PenguinAnimatorComponent.hpp>
#include <PetaPetaPenguin/ECS/Components/Serialization/PenguinAnimatorComponentSerialization.hpp>
#include <PetaPetaPenguin/ECS/Systems/PenguinAnimationSystem.hpp>
#include <PetaPetaPenguin/ECS/Components/ClickCounterComponent.hpp>
#include <PetaPetaPenguin/ECS/Components/Serialization/ClickCounterComponentSerialization.hpp>
#include <PetaPetaPenguin/ECS/Systems/ClickCounterSystem.hpp>
#include <PetaPetaPenguin/Data/ClickCounterSaveData.hpp>

#include <Tsukino/EngineIntegration/EngineAPI.hpp>
#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/Engine/ECS/Prefab/PrefabFactory.hpp>
#include <Tsukino/EngineIntegration/Scene/GameSceneManager.hpp>

#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/EngineIntegration/ECS/System/TransformSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/CameraSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SpriteRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/FontRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/InteractionSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/EffectSystem.hpp>

#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/Core/Input/KeyCodes.hpp>
#include <Tsukino/Core/Window.hpp>

#include <Tsukino/Renderer/Renderer.hpp>

#include <entt/entt.hpp>
#include <hlsl++.h>
// 名前空間 : PetaPetaPenguin
namespace PetaPetaPenguin {
    void PenguinGameScene::OnInitialize(Tsukino::EngineIntegration::EngineAPI& api) {
        // コンテキストをレジストリから取得
        Tsukino::EngineIntegration::EngineContext* context = m_scene.GetRegistry().GetContext<Tsukino::EngineIntegration::EngineContext*>();
        //-------------------------------------------------------------
        // レジストリを取得
        //-------------------------------------------------------------
        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();
        //-------------------------------------------------------------
        // イベントバスを取得
        //-------------------------------------------------------------
        Tsukino::ECS::EventBus& eventBus = m_scene.GetEventBus();
        //--------------------------------------------------------------
        // クリアカラーを透明に設定
        //--------------------------------------------------------------
        context->renderer->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        //--------------------------------------------------------------
        // このアプリは常にアクティブ
        //--------------------------------------------------------------
        context->window->SetUpdateMode(Tsukino::Core::Window::UpdateMode::AlwaysResident);
        context->window->SetUpdateMode(Tsukino::Core::Window::UpdateMode::ActiveOnly);
        //--------------------------------------------------------------
        // フルスクリーンモードにする
        //--------------------------------------------------------------
        context->window->SetFullscreen(true);

        //--------------------------------------------------------------
        // システムの生成と追加
        //--------------------------------------------------------------
        // enumを使って優先度を管理
        enum class SystemPriority : int {
            Transform        = 0,
            PenguinAnimation = 1,
            Interaction      = 2,
            ClickCounter     = 3,
            Camera           = 5,
            TransformPreDraw = 8,
            Font             = 9,
            Sprite           = 10,
        };

        // Transformは一番最初に計算する（ゲームロジックが前フレームではなく当該フレームの
        // ワールド行列を読めるようにするため）
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::TransformSystem>(), static_cast<int>(SystemPriority::Transform));
        // ペンギンアニメーションシステムの追加
        // （テクスチャの差し替えなので、必ず描画システムより前で走らせる）
        m_scene.AddSystem(std::make_shared<PetaPetaPenguin::ECS::PenguinAnimationSystem>(), static_cast<int>(SystemPriority::PenguinAnimation));
        // InteractionはTransformの後に計算する
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::InteractionSystem>(), static_cast<int>(SystemPriority::Interaction));
        // クリックカウンターはInteractionが状態を確定させた後に処理する
        m_scene.AddSystem(std::make_shared<PetaPetaPenguin::ECS::ClickCounterSystem>(), static_cast<int>(SystemPriority::ClickCounter));
        // カメラは描画前に更新する
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::CameraSystem>(), static_cast<int>(SystemPriority::Camera));
        //--------------------------------------------------------------
        // Transformの2パス目。
        // 描画システム（Font/Sprite）はどちらもworldMatrixを読むため、
        // 位置を書き換えるシステム（Interactionのドラッグなど）より後、かつ
        // 描画より前にもう一度ワールド行列を計算し直す必要がある。
        // これが無いと描画が常に1フレーム遅れ、ドラッグ中に文字と絵がずれる
        //--------------------------------------------------------------
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::TransformSystem>(), static_cast<int>(SystemPriority::TransformPreDraw));
        // フォント描画
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::FontRendererSystem>(), static_cast<int>(SystemPriority::Font));
        // スプライトなど描画用のコマンド生成は後で行う
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SpriteRenderSystem>(), static_cast<int>(SystemPriority::Sprite));

        //--------------------------------------------------------------
        // ゲーム固有コンポーネントをPrefabFactoryに登録する
        // （組み込みコンポーネントはエンジン側で登録済み）
        //--------------------------------------------------------------
        context->prefabFactory->RegisterComponent<PetaPetaPenguin::ECS::PenguinAnimatorComponent>("PenguinAnimatorComponent");
        context->prefabFactory->RegisterComponent<PetaPetaPenguin::ECS::ClickCounterComponent>("ClickCounterComponent");

        //--------------------------------------------------------------
        // Prefab(JSON)からエンティティ一式をまとめて生成する
        //--------------------------------------------------------------
        Tsukino::Engine::ECS::Prefab::PrefabFactory::PrefabInstance instance =
            context->prefabFactory->InstantiateGroup("PetaPetaPenguin/Assets/Prefabs/PenguinGame.Group.json", registry);

        //--------------------------------------------------------------
        // 実行時にしか決まらない値だけをここで上書きする
        //--------------------------------------------------------------
        // 2Dカメラのortho幅はウィンドウの縦ピクセル数に合わせる
        Tsukino::BuiltIn::ECS::CameraComponent& camera2D = registry.GetComponent<Tsukino::BuiltIn::ECS::CameraComponent>(instance.at("Camera2D"));
        camera2D.orthoSize                               = context->window->GetHeight();

        // 保存されたクリック数を復元する
        PetaPetaPenguin::Data::ClickCounterSaveData saved = PetaPetaPenguin::Data::ClickCounterSaveData::Load();

        PetaPetaPenguin::ECS::ClickCounterComponent& counter =
            registry.GetComponent<PetaPetaPenguin::ECS::ClickCounterComponent>(instance.at("ClickCounterBackground"));
        counter.count = saved.count;

        Tsukino::BuiltIn::ECS::FontComponent& counterFont =
            registry.GetComponent<Tsukino::BuiltIn::ECS::FontComponent>(instance.at("ClickCounterText"));
        counterFont.text = std::to_wstring(saved.count);
    }

    void PenguinGameScene::OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) {
        m_scene.Update(deltaTime);
    }

    void PenguinGameScene::OnExit() {
        // シーンの終了処理をここに記述
    }
};    // namespace PetaPetaPenguin
