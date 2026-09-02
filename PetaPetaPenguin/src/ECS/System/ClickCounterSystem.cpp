//-------------------------------------------------------------
//! @file   ClickCounterSystem.cpp
//! @brief  ClickCounterSystemクラスの実装
//! @author 山﨑愛
//-------------------------------------------------------------
#include <PetaPetaPenguin/ECS/Systems/ClickCounterSystem.hpp>
#include <PetaPetaPenguin/ECS/Components/ClickCounterComponent.hpp>
#include <PetaPetaPenguin/Data/ClickCounterSaveData.hpp>

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
	//!
	//! @note  以前はここでペンギン本体とカウンターの位置を相互に追従させていたが
	//!        （リーダー・フォロワー方式）、TransformComponent::parent による
	//!        本物の親子関係に置き換えたため、位置に関する処理は一切不要になった。
	//!        相対位置の維持はTransformSystemがワールド行列を伝播させて行う
	//-------------------------------------------------------------
	void ClickCounterSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
		(void)deltaTime;

		Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
		if (!ctx)
			return;

		auto& input = ctx->inputSystem;

		//-------------------------------------------------------------
		// クリック検知（何らかの入力があった瞬間にカウント）
		//-------------------------------------------------------------
		if (!input->AnyKeyPressed())
			return;

		registry.View<ClickCounterComponent>().each([&](ClickCounterComponent& counter) {
			counter.count++;

			if (registry.HasComponent<Tsukino::BuiltIn::ECS::FontComponent>(counter.textEntity)) {
				auto& font = registry.GetComponent<Tsukino::BuiltIn::ECS::FontComponent>(counter.textEntity);
				font.text = std::to_wstring(counter.count);
			}

			PetaPetaPenguin::Data::ClickCounterSaveData saveData;
			saveData.count = counter.count;
			saveData.Save();
		});
	}
}    // namespace PetaPetaPenguin::ECS
