//-------------------------------------------------------------
//! @file   ClickCounterComponent.hpp
//! @brief  ClickCounterComponentクラスの宣言
//-------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Entity/Entity.hpp>
#include <hlsl++.h>
// 名前空間 : PetaPetaPenguin::ECS
namespace PetaPetaPenguin::ECS {
    //-------------------------------------------------------------
    //! @struct ClickCounterComponent
    //! @brief  クリックカウンター（灰色スプライト）に必要なComponent
    //-------------------------------------------------------------
    struct ClickCounterComponent {
        int count = 0;

        // カウント数を表示するFontComponentエンティティ
        Tsukino::ECS::Entity textEntity;

        // 追従先（ペンギン本体centerEntity）
        Tsukino::ECS::Entity anchorEntity;

        // anchorEntityからの相対位置（この位置関係を保ったまま追従し合う）
        hlslpp::float3 anchorOffset = {0.0f, 80.0f, 0.0f};
    };
}    // namespace PetaPetaPenguin::ECS
