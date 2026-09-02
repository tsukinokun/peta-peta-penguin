//-------------------------------------------------------------
//! @file   ClickCounterComponentSerialization.hpp
//! @brief  ClickCounterComponentのcerealシリアライズ定義
//! @author 山﨑愛
//! @note   countは実行時にセーブデータから復元される値のため対象外とし、
//!         アタッチ時のデフォルト値(0)のまま残す。
//-------------------------------------------------------------
#pragma once
#include <PetaPetaPenguin/ECS/Components/ClickCounterComponent.hpp>

#include <cereal/cereal.hpp>

// 名前空間 : PetaPetaPenguin::ECS
namespace PetaPetaPenguin::ECS {

    //--------------------------------------------------------------
    //! @brief  ClickCounterComponentのcerealシリアライズ定義
    //--------------------------------------------------------------
    template <class Archive>
    void save(Archive& archive, const ClickCounterComponent& counter) {
        archive(cereal::make_nvp("textEntity", counter.textEntity));
    }

    //--------------------------------------------------------------
    //! @brief  ClickCounterComponentのcerealデシリアライズ定義
    //--------------------------------------------------------------
    template <class Archive>
    void load(Archive& archive, ClickCounterComponent& counter) {
        archive(counter.textEntity);
    }

}    // namespace PetaPetaPenguin::ECS
