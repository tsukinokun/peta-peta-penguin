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

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
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
		if (!ctx)
			return;

		auto& input = ctx->inputSystem;

		registry
			.View<ClickCounterComponent, Tsukino::BuiltIn::ECS::TransformComponent, Tsukino::BuiltIn::ECS::DraggableComponent,
			Tsukino::BuiltIn::ECS::SpriteComponent>()
			.each([&](auto /*entity*/, ClickCounterComponent& counter, Tsukino::BuiltIn::ECS::TransformComponent& counterTransform,
				Tsukino::BuiltIn::ECS::DraggableComponent& counterDraggable, Tsukino::BuiltIn::ECS::SpriteComponent& /*counterSprite*/) {
					if (!registry.IsValid(counter.anchorEntity) || !registry.HasComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counter.anchorEntity))
						return;

					auto& anchorTransform = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counter.anchorEntity);
					auto* anchorDraggable = registry.try_get<Tsukino::BuiltIn::ECS::DraggableComponent>(counter.anchorEntity);

					//-------------------------------------------------------------
					// 追従（リーダー・フォロワー方式）
					// どちらか一方がドラッグ中のフレームだけ、もう一方の位置をオフセット分ずらして追従させる
					//-------------------------------------------------------------
					if (counterDraggable.isDragging) {
						anchorTransform.position = counterTransform.position - counter.anchorOffset;
						anchorTransform.dirty = true;
					}
					else if (anchorDraggable && anchorDraggable->isDragging) {
						counterTransform.position = anchorTransform.position + counter.anchorOffset;
						counterTransform.dirty = true;
					}

					//-------------------------------------------------------------
					// カウント表示テキストの追従（常に灰色スプライトへ位置を同期）
					//-------------------------------------------------------------
					if (registry.HasComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counter.textEntity)) {
						auto& textTransform = registry.GetComponent<Tsukino::BuiltIn::ECS::TransformComponent>(counter.textEntity);
						textTransform.position = counterTransform.position;
						textTransform.dirty = true;
					}

					//-------------------------------------------------------------
					// クリック検知（何らかの入力があった瞬間にカウント）
					//-------------------------------------------------------------
					if (input->AnyKeyPressed()) {
						counter.count++;

						if (registry.HasComponent<Tsukino::BuiltIn::ECS::FontComponent>(counter.textEntity)) {
							auto& font = registry.GetComponent<Tsukino::BuiltIn::ECS::FontComponent>(counter.textEntity);
							font.text = std::to_wstring(counter.count);
						}

						PetaPetaPenguin::Data::ClickCounterSaveData saveData;
						saveData.count = counter.count;
						saveData.Save();
					}
				});
	}
}    // namespace PetaPetaPenguin::ECS
