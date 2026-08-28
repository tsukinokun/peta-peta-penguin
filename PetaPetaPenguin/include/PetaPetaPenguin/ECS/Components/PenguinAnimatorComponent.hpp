//-------------------------------------------------------------
//! @file   PenguinAnimatorComponent.hpp
//! @brief  PenguinAnimatorComponentクラスの宣言
//-------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/EntityRef/EntityRef.hpp>

#include <PetaPetaPenguin/Data/PenguinSkinDefinition.hpp>
// 名前空間 : PetaPetaPenguin::ECS
namespace PetaPetaPenguin::ECS {
    //-------------------------------------------------------------
    //! @struct PenguinAnimatorComponent
    //! @brief  ペンギンのアニメーションに必要なComponent
    //-------------------------------------------------------------
    struct PenguinAnimatorComponent {
        Tsukino::ECS::EntityRef centerEntity;
        Tsukino::ECS::EntityRef leftHandEntity;
        Tsukino::ECS::EntityRef rightHandEntity;

        // 現在適用されているスキン定義
        // これを差し替えるだけで、全てのパーツが一瞬で切り替わる
        PetaPetaPenguin::Data::PenguinSkinDefinition currentSkin;

        // --- アニメーション状態 ---
        float blinkTimer    = 0.0f;
        float blinkInterval = 3.0f;

        bool isCenterFrame2 = false;
        bool isLeftFrame2   = false;
        bool isRightFrame2  = false;

        float       leftActionTimer  = 0.0f;
        float       rightActionTimer = 0.0f;
        const float actionDuration   = 0.15f;    // 手を上げている時間（秒）
        bool        lastHandWasLeft  = false;    // 交互にするためのフラグ
    };
}    // namespace PetaPetaPenguin::ECS
