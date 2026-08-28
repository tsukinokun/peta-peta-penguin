//-------------------------------------------------------------
//! @file    PenguinGameScene.cpp
//! @brief   ペンギンゲームのシーンの実装
//-------------------------------------------------------------
#include <PetaPetaPenguin/Scene/PenguinGameScene.hpp>

#include <PetaPetaPenguin/ECS/Components/PenguinAnimatorComponent.hpp>
#include <PetaPetaPenguin/ECS/Systems/PenguinAnimationSystem.hpp>
#include <PetaPetaPenguin/ECS/Components/ClickCounterComponent.hpp>
#include <PetaPetaPenguin/ECS/Systems/ClickCounterSystem.hpp>
#include <PetaPetaPenguin/Data/ClickCounterSaveData.hpp>

#include <Tsukino/EngineIntegration/EngineAPI.hpp>
#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/EngineIntegration/Scene/GameSceneManager.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DraggableComponent.hpp>
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
            Interaction      = 2,
            ClickCounter     = 3,
            Camera           = 5,
            Font             = 9,
            Sprite           = 10,
            PenguinAnimation = 11,
        };

        // Transformは一番最初に計算する
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::TransformSystem>(), static_cast<int>(SystemPriority::Transform));
        // InteractionはTransformの後に計算する
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::InteractionSystem>(), static_cast<int>(SystemPriority::Interaction));
        // クリックカウンターはInteractionが状態を確定させた後に処理する
        m_scene.AddSystem(std::make_shared<PetaPetaPenguin::ECS::ClickCounterSystem>(), static_cast<int>(SystemPriority::ClickCounter));
        // カメラは描画前に更新する
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::CameraSystem>(), static_cast<int>(SystemPriority::Camera));
        // フォント描画
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::FontRendererSystem>(), static_cast<int>(SystemPriority::Font));
        // スプライトなど描画用のコマンド生成は後で行う
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SpriteRenderSystem>(), static_cast<int>(SystemPriority::Sprite));
        // ペンギンアニメーションシステムの追加
        m_scene.AddSystem(std::make_shared<PetaPetaPenguin::ECS::PenguinAnimationSystem>(), static_cast<int>(SystemPriority::PenguinAnimation));

        //--------------------------------------------------------------
        // 2Dカメラエンティティの生成
        //--------------------------------------------------------------
        Tsukino::ECS::Entity cameraEntity2D = m_scene.CreateEntity();

        // TransformComponent (カメラの位置)
        Tsukino::BuiltIn::ECS::TransformComponent& camTransform2D = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(cameraEntity2D);
        camTransform2D.position                                   = hlslpp::float3(0.0f, 0.0f, -10.0f);    // 手前に引く

        // CameraComponent (投影設定)
        Tsukino::BuiltIn::ECS::CameraComponent& camera2D = registry.AddComponent<Tsukino::BuiltIn::ECS::CameraComponent>(cameraEntity2D);
        camera2D.projectionType                          = Tsukino::BuiltIn::ECS::CameraComponent::ProjectionType::Orthographic;
        camera2D.orthoSize                               = context->window->GetHeight();    // ウィンドウの縦ピクセル数を表示幅にする
        camera2D.isPrimary                               = false;                           // これをメインカメラにしない

        //--------------------------------------------------------------
        // ペンギンアニメーターエンティティの生成
        //--------------------------------------------------------------
        Tsukino::ECS::Entity penguinAnimatorEntity = m_scene.CreateEntity();

        auto& animator = registry.AddComponent<PetaPetaPenguin::ECS::PenguinAnimatorComponent>(penguinAnimatorEntity);

        // スキン定義のロード（本来はアセット管理システム経由が望ましい）
        // 一旦プロトタイプとして、ここでアセットハンドルをセットします
        auto* assetMgr = context->assetManager;

        // 初期状態のセットアップ
        animator.blinkInterval = 2.0f;    // まばたき間隔

        //--------------------------------------------------------------
        // ペンギンのスキン定義を作成
        //--------------------------------------------------------------
        {
            m_Skin.name          = "Normal";
            m_Skin.centerTexs[0] = assetMgr->Load(Tsukino::Core::Path("PetaPetaPenguin/Assets/Penguin/penguin_center_1.png"));
            m_Skin.centerTexs[1] = assetMgr->Load(Tsukino::Core::Path("PetaPetaPenguin/Assets/Penguin/penguin_center_2.png"));
            m_Skin.leftTexs[0]   = assetMgr->Load(Tsukino::Core::Path("PetaPetaPenguin/Assets/Penguin/penguin_left_1.png"));
            m_Skin.leftTexs[1]   = assetMgr->Load(Tsukino::Core::Path("PetaPetaPenguin/Assets/Penguin/penguin_left_2.png"));
            m_Skin.rightTexs[0]  = assetMgr->Load(Tsukino::Core::Path("PetaPetaPenguin/Assets/Penguin/penguin_right_1.png"));
            m_Skin.rightTexs[1]  = assetMgr->Load(Tsukino::Core::Path("PetaPetaPenguin/Assets/Penguin/penguin_right_2.png"));
            // スキン設定
            animator.currentSkin = &m_Skin;
        }

        //--------------------------------------------------------------
        // センター生成
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity entity = m_scene.CreateEntity();

            // TransformComponent の追加と初期化
            Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(entity);
            transform.position                                   = hlslpp::float3(100.0f, 100.0f, 0.0f);
            transform.rotation                                   = hlslpp::quaternion(0.0f, 0.0f, 0.0f, 1.0f);    // 無回転
            transform.scale                                      = hlslpp::float3(1.0f, 1.0f, 1.0f);
            transform.dirty                                      = true;          // 初回計算のためフラグを立てる
            transform.parent                                     = entt::null;    // 親なし

            // SpriteComponent の追加
            Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.AddComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
            sprite.tintColor                               = hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f);    // 白色
            sprite.uvRect                                  = hlslpp::float4(0.0f, 0.0f, 1.0f, 1.0f);

            animator.centerEntity = entity;    // センターエンティティをアニメーターに登録

            registry.AddComponent<Tsukino::BuiltIn::ECS::DraggableComponent>(entity);
        }

        //--------------------------------------------------------------
        // 左手エンティティ生成
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity entity = m_scene.CreateEntity();

            // TransformComponent の追加と初期化
            Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(entity);
            transform.position                                   = hlslpp::float3(100.0f, 100.0f, 0.0f);
            transform.rotation                                   = hlslpp::quaternion(0.0f, 0.0f, 0.0f, 1.0f);    // 無回転
            transform.scale                                      = hlslpp::float3(1.0f, 1.0f, 1.0f);
            transform.dirty                                      = true;          // 初回計算のためフラグを立てる
            transform.parent                                     = entt::null;    // 親なし

            // SpriteComponent の追加
            Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.AddComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
            sprite.tintColor                               = hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f);    // 白色
            sprite.uvRect                                  = hlslpp::float4(0.0f, 0.0f, 1.0f, 1.0f);
            sprite.sortOrder                               = 1;    // 中央のペンギンより手前に描画

            animator.leftHandEntity = entity;    // 左手エンティティをアニメーターに登録
            registry.AddComponent<Tsukino::BuiltIn::ECS::DraggableComponent>(entity);
        }

        //--------------------------------------------------------------
        // 右手エンティティ生成
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity entity = m_scene.CreateEntity();

            // TransformComponent の追加と初期化
            Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(entity);
            transform.position                                   = hlslpp::float3(100.0f, 100.0f, 0.0f);
            transform.rotation                                   = hlslpp::quaternion(0.0f, 0.0f, 0.0f, 1.0f);    // 無回転
            transform.scale                                      = hlslpp::float3(1.0f, 1.0f, 1.0f);
            transform.dirty                                      = true;          // 初回計算のためフラグを立てる
            transform.parent                                     = entt::null;    // 親なし

            // SpriteComponent の追加
            Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.AddComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
            sprite.tintColor                               = hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f);    // 白色
            sprite.uvRect                                  = hlslpp::float4(0.0f, 0.0f, 1.0f, 1.0f);
            sprite.sortOrder                               = 1;    // 中央のペンギンより手前に描画

            animator.rightHandEntity = entity;    // 右手エンティティをアニメーターに登録
            registry.AddComponent<Tsukino::BuiltIn::ECS::DraggableComponent>(entity);
        }

        //--------------------------------------------------------------
        // クリックカウンター（灰色スプライト＋カウント数テキスト）生成
        //--------------------------------------------------------------
        {
            // 保存されたクリック数を復元
            PetaPetaPenguin::Data::ClickCounterSaveData saved = PetaPetaPenguin::Data::ClickCounterSaveData::Load();

            // ペンギン本体（centerEntity）からの相対位置（ペンギンのすぐ下）
            const hlslpp::float3 anchorOffset = {0.0f, 80.0f, 0.0f};
            const Tsukino::BuiltIn::ECS::TransformComponent& centerTransform =
                registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(animator.centerEntity);
            const hlslpp::float3 counterPosition = centerTransform.position + anchorOffset;

            Tsukino::Asset::AssetHandle counterBgTex = assetMgr->Load(Tsukino::Core::Path("PetaPetaPenguin/Assets/UI/click_counter_bg.png"));

            // 灰色スプライトエンティティ
            Tsukino::ECS::Entity counterEntity = m_scene.CreateEntity();
            {
                Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counterEntity);
                transform.position                                   = counterPosition;
                transform.rotation                                   = hlslpp::quaternion(0.0f, 0.0f, 0.0f, 1.0f);
                transform.scale                                      = hlslpp::float3(1.0f, 1.0f, 1.0f);
                transform.dirty                                      = true;
                transform.parent                                     = entt::null;

                Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.AddComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(counterEntity);
                sprite.textureHandle                           = counterBgTex;
                sprite.tintColor                               = hlslpp::float4(1.0f, 1.0f, 1.0f, 1.0f);
                sprite.uvRect                                  = hlslpp::float4(0.0f, 0.0f, 1.0f, 1.0f);
                sprite.sortOrder                               = 0;

                // クリック判定（当たり判定）を受けるためにDraggableComponentが必要
                registry.AddComponent<Tsukino::BuiltIn::ECS::DraggableComponent>(counterEntity);
            }

            // カウント数テキストエンティティ
            Tsukino::ECS::Entity counterTextEntity = m_scene.CreateEntity();
            {
                Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counterTextEntity);
                transform.position                                   = counterPosition;
                transform.rotation                                   = hlslpp::quaternion(0.0f, 0.0f, 0.0f, 1.0f);
                transform.scale                                      = hlslpp::float3(1.0f, 1.0f, 1.0f);
                transform.dirty                                      = true;
                transform.parent                                     = entt::null;

                Tsukino::BuiltIn::ECS::FontComponent& font = registry.AddComponent<Tsukino::BuiltIn::ECS::FontComponent>(counterTextEntity);
                font.text                                  = std::to_wstring(saved.count);
                font.color                                 = hlslpp::float4(0.15f, 0.15f, 0.15f, 1.0f);
                font.horizontalAlign                       = Tsukino::BuiltIn::ECS::HorizontalAlign::Center;
                font.verticalAlign                         = Tsukino::BuiltIn::ECS::VerticalAlign::Middle;
                font.sortOrder                             = 1;    // 灰色スプライトより手前に描画
            }

            // クリックカウンターComponent（カウント状態と追従関係を保持）
            PetaPetaPenguin::ECS::ClickCounterComponent& counter = registry.AddComponent<PetaPetaPenguin::ECS::ClickCounterComponent>(counterEntity);
            counter.count                                        = saved.count;
            counter.textEntity                                   = counterTextEntity;
            counter.anchorEntity                                 = animator.centerEntity;
            counter.anchorOffset                                 = anchorOffset;
        }
    }

    void PenguinGameScene::OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) {
        m_scene.Update(deltaTime);
    }

    void PenguinGameScene::OnExit() {
        // シーンの終了処理をここに記述
    }
};    // namespace PetaPetaPenguin
