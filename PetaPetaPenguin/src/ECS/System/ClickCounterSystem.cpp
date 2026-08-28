//-------------------------------------------------------------
//! @file   ClickCounterSystem.cpp
//! @brief  ClickCounterSystemクラスの実装
//-------------------------------------------------------------
#include <PetaPetaPenguin/ECS/Systems/ClickCounterSystem.hpp>
#include <PetaPetaPenguin/ECS/Components/ClickCounterComponent.hpp>
#include <PetaPetaPenguin/Data/ClickCounterSaveData.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/DraggableComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>

#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Engine/Asset/Texture/TextureAsset.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/Core/Input/KeyCodes.hpp>
#include <Tsukino/Core/Math/MathHelper.hpp>
#include <Tsukino/Core/typedef.hpp>

#include <entt/entt.hpp>

#include <string>
// 名前空間 : PetaPetaPenguin::ECS
namespace PetaPetaPenguin::ECS {
    //-------------------------------------------------------------
    //! @brief システムの更新
    //-------------------------------------------------------------
    void ClickCounterSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        (void)deltaTime;

        Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        if(!ctx)
            return;

        auto& input = ctx->inputSystem;

        registry
            .View<ClickCounterComponent, Tsukino::BuiltIn::ECS::TransformComponent, Tsukino::BuiltIn::ECS::DraggableComponent,
                  Tsukino::BuiltIn::ECS::SpriteComponent>()
            .each([&](auto /*entity*/, ClickCounterComponent& counter, Tsukino::BuiltIn::ECS::TransformComponent& counterTransform,
                      Tsukino::BuiltIn::ECS::DraggableComponent& counterDraggable, Tsukino::BuiltIn::ECS::SpriteComponent& counterSprite) {
                if(!registry.IsValid(counter.anchorEntity) || !registry.HasComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counter.anchorEntity))
                    return;

                auto& anchorTransform = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counter.anchorEntity);
                auto* anchorDraggable = registry.try_get<Tsukino::BuiltIn::ECS::DraggableComponent>(counter.anchorEntity);

                //-------------------------------------------------------------
                // 追従（リーダー・フォロワー方式）
                // どちらか一方がドラッグ中のフレームだけ、もう一方の位置をオフセット分ずらして追従させる
                //-------------------------------------------------------------
                if(counterDraggable.isDragging) {
                    anchorTransform.position = counterTransform.position - counter.anchorOffset;
                    anchorTransform.dirty    = true;
                } else if(anchorDraggable && anchorDraggable->isDragging) {
                    counterTransform.position = anchorTransform.position + counter.anchorOffset;
                    counterTransform.dirty    = true;
                }

                //-------------------------------------------------------------
                // カウント表示テキストの追従（常に灰色スプライトへ位置を同期）
                //-------------------------------------------------------------
                if(registry.HasComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counter.textEntity)) {
                    auto& textTransform    = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counter.textEntity);
                    textTransform.position = counterTransform.position;
                    textTransform.dirty    = true;
                }

                //-------------------------------------------------------------
                // クリック検知
                // InteractionSystemのisDraggingに便乗せず、自前で当たり判定を行う。
                // （isDraggingは「ドラッグ開始した瞬間」しか真にならないため、
                //   ドラッグ機構側の状態変化と間接的に結びつけるより、ここで
                //   直接IsPointInRectする方が挙動を追いやすく確実）
                //-------------------------------------------------------------
                if(input->IsKeyPressed(Tsukino::Input::KeyCode::LButton)) {
                    auto textureAsset =
                        std::static_pointer_cast<Tsukino::Asset::TextureAsset>(ctx->assetManager->Get(counterSprite.textureHandle));

                    if(textureAsset) {
                        i32 mouseX, mouseY;
                        input->GetMousePosition(&mouseX, &mouseY);

                        hlslpp::float2 mousePos    = {static_cast<float>(mouseX), static_cast<float>(mouseY)};
                        hlslpp::float2 counterPos  = {counterTransform.position.x, counterTransform.position.y};
                        hlslpp::float2 counterSize = {static_cast<float>(textureAsset->width) * counterTransform.scale.x,
                                                       static_cast<float>(textureAsset->height) * counterTransform.scale.y};

                        if(Tsukino::Core::Math::IsPointInRect(mousePos, counterPos, counterSize)) {
                            counter.count++;

                            if(registry.HasComponent<Tsukino::BuiltIn::ECS::FontComponent>(counter.textEntity)) {
                                auto& font = registry.GetComponent<Tsukino::BuiltIn::ECS::FontComponent>(counter.textEntity);
                                font.text  = std::to_wstring(counter.count);
                            }

                            PetaPetaPenguin::Data::ClickCounterSaveData saveData;
                            saveData.count = counter.count;
                            saveData.Save();
                        }
                    }
                }
            });
    }
}    // namespace PetaPetaPenguin::ECS
