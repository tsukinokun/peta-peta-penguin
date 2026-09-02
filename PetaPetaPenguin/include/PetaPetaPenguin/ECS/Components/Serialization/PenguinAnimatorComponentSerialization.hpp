//-------------------------------------------------------------
//! @file   PenguinAnimatorComponentSerialization.hpp
//! @brief  PenguinAnimatorComponent / PenguinSkinDefinitionのcerealシリアライズ定義
//! @author 山﨑愛
//-------------------------------------------------------------
#pragma once
#include <PetaPetaPenguin/ECS/Components/PenguinAnimatorComponent.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>

// 名前空間 : PetaPetaPenguin::Data
// (ADLで発見されるよう、PenguinSkinDefinition自身の名前空間で定義する)
namespace PetaPetaPenguin::Data {

    //--------------------------------------------------------------
    //! @brief  PenguinSkinDefinitionのcerealシリアライズ定義
    //--------------------------------------------------------------
    template <class Archive>
    void save(Archive& archive, const PenguinSkinDefinition& skin) {
        archive(cereal::make_nvp("name", skin.name),
                cereal::make_nvp("centerTexs", skin.centerTexs),
                cereal::make_nvp("leftTexs", skin.leftTexs),
                cereal::make_nvp("rightTexs", skin.rightTexs));
    }

    //--------------------------------------------------------------
    //! @brief  PenguinSkinDefinitionのcerealデシリアライズ定義
    //--------------------------------------------------------------
    template <class Archive>
    void load(Archive& archive, PenguinSkinDefinition& skin) {
        archive(skin.name, skin.centerTexs, skin.leftTexs, skin.rightTexs);
    }

}    // namespace PetaPetaPenguin::Data

// 名前空間 : PetaPetaPenguin::ECS
// (ADLで発見されるよう、PenguinAnimatorComponent自身の名前空間で定義する)
namespace PetaPetaPenguin::ECS {

    //--------------------------------------------------------------
    //! @brief  PenguinAnimatorComponentのcerealシリアライズ定義
    //--------------------------------------------------------------
    template <class Archive>
    void save(Archive& archive, const PenguinAnimatorComponent& animator) {
        archive(cereal::make_nvp("blinkInterval", animator.blinkInterval),
                cereal::make_nvp("centerEntity", animator.centerEntity),
                cereal::make_nvp("leftHandEntity", animator.leftHandEntity),
                cereal::make_nvp("rightHandEntity", animator.rightHandEntity),
                cereal::make_nvp("currentSkin", animator.currentSkin));
    }

    //--------------------------------------------------------------
    //! @brief  PenguinAnimatorComponentのcerealデシリアライズ定義
    //--------------------------------------------------------------
    template <class Archive>
    void load(Archive& archive, PenguinAnimatorComponent& animator) {
        archive(animator.blinkInterval, animator.centerEntity, animator.leftHandEntity, animator.rightHandEntity, animator.currentSkin);
    }

}    // namespace PetaPetaPenguin::ECS
