//-------------------------------------------------------------
//! @file   ClickCounterSaveData.cpp
//! @brief  ClickCounterSaveData構造体の実装
//! @author 山﨑愛
//-------------------------------------------------------------
#include <PetaPetaPenguin/Data/ClickCounterSaveData.hpp>

#include <Tsukino/Core/IO/FileSystem.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <cereal/archives/json.hpp>

#include <fstream>
// 名前空間 : PetaPetaPenguin::Data
namespace PetaPetaPenguin::Data {
    namespace {
        //-------------------------------------------------------------
        //! @brief  セーブファイルのパスを取得する
        //-------------------------------------------------------------
        Tsukino::Core::Path GetSaveFilePath() {
            return Tsukino::IO::FileSystem::GetAssetRootPath() / "PetaPetaPenguin" / "SaveData" / "ClickCount.json";
        }
    }    // namespace

    //-------------------------------------------------------------
    //! @brief  保存済みJSONを読み込む（存在しない場合はデフォルト値）
    //-------------------------------------------------------------
    ClickCounterSaveData ClickCounterSaveData::Load() {
        ClickCounterSaveData data;

        const Tsukino::Core::Path path = GetSaveFilePath();
        if(!Tsukino::IO::FileSystem::Exists(path))
            return data;

        std::ifstream is(path.string());
        if(!is.is_open()) {
            Tsukino::Core::Log::Warn("ClickCounter save file not found: " + path.string());
            return data;
        }

        try {
            cereal::JSONInputArchive archive(is);
            archive(cereal::make_nvp("ClickCounter", data));
        } catch(const std::exception& e) {
            Tsukino::Core::Log::Error("Failed to parse ClickCounter save file: " + std::string(e.what()));
            return ClickCounterSaveData{};
        }
        return data;
    }

    //-------------------------------------------------------------
    //! @brief  現在の内容をJSONへ保存する
    //-------------------------------------------------------------
    void ClickCounterSaveData::Save() const {
        const Tsukino::Core::Path path = GetSaveFilePath();
        (void)Tsukino::IO::FileSystem::CreateDirectories(path.parent_path());

        std::ofstream os(path.string());
        if(!os.is_open()) {
            Tsukino::Core::Log::Error("Failed to open ClickCounter save file: " + path.string());
            return;
        }

        try {
            cereal::JSONOutputArchive archive(os);
            archive(cereal::make_nvp("ClickCounter", *this));
        } catch(const std::exception& e) {
            Tsukino::Core::Log::Error("Failed to write ClickCounter save file: " + std::string(e.what()));
        }
    }
}    // namespace PetaPetaPenguin::Data
