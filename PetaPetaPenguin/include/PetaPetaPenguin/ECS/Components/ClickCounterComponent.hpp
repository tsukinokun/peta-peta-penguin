//-------------------------------------------------------------
//! @file   ClickCounterComponent.hpp
//! @brief  ClickCounterComponentクラスの宣言
//! @author 山﨑愛
//-------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/EntityRef/EntityRef.hpp>
// 名前空間 : PetaPetaPenguin::ECS
namespace PetaPetaPenguin::ECS {
    //-------------------------------------------------------------
    //! @struct ClickCounterComponent
    //! @brief  クリックカウンター（灰色スプライト）に必要なComponent
    //-------------------------------------------------------------
    struct ClickCounterComponent {
        int count = 0;

        //-------------------------------------------------------------
        // カウント数を表示するFontComponentエンティティ
        //
        // 位置の追従は持たない。ペンギン本体との相対位置は
        // TransformComponent::parent（Prefabの "parent": "#PenguinCenter"）で
        // 表現しており、TransformSystemがワールド行列を伝播させる
        //-------------------------------------------------------------
        Tsukino::ECS::EntityRef textEntity;
    };
}    // namespace PetaPetaPenguin::ECS
