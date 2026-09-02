//-------------------------------------------------------------
//! @file   PenguinSkinDefinition.hpp
//! @brief  PenguinSkinDefinitionクラスの宣言
//! @author 山﨑愛
//-------------------------------------------------------------
#pragma once
#include <array>
#include <string>
#include <Tsukino/Engine/Asset/AssetRef.hpp>
// 名前空間 : PetaPetaPenguin::Data
namespace PetaPetaPenguin::Data{
    struct PenguinSkinDefinition {
        // スキン名
        std::string name;

        // 各部位のテクスチャセット
        std::array<Tsukino::Asset::AssetRef, 2> centerTexs;
        std::array<Tsukino::Asset::AssetRef, 2> leftTexs;
        std::array<Tsukino::Asset::AssetRef, 2> rightTexs;
    };
}    // namespace PetaPetaPenguin::Data
