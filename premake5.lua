workspace "PetaPetaPenguin"              -- ソリューション名
    architecture "x64"                   -- アーキテクチャ
    configurations { "Debug", "Release" } -- ビルド構成

    startproject "PetaPetaPenguin"       -- スタートアッププロジェクト
    location ".build"                    -- ビルドファイルの出力先
    multiprocessorcompile "On"           -- マルチプロセッサコンパイルを有効化
    exceptionhandling "On"               -- 例外処理を有効化

    filter "configurations:*"
        defines { "JPH_DEBUG_RENDERER" } -- 値は1でなくても定義されていることが重要
    filter {}

    filter "configurations:Debug"
        optimize "Off"
        symbols "On"

    filter "action:vs*"
        buildoptions { "/utf-8" }
    filter {}

    filter "configurations:Release"
        optimize "Full"
        symbols "On"
        -- NDEBUG は premake が自動では付けないため明示的に定義する。
        defines { "NDEBUG" }
    filter {}

    filter "configurations:*"
        linkoptions { "/IGNORE:4006" }
    filter {}

----------------------------------------
-- TsukinoEngine を取り込む（サブモジュール）
-- ここでの _MAIN_SCRIPT_DIR はこのファイル（リポジトリルート）のため、
-- External/TsukinoEngine/premake5.lua 内の ROOT_IS_SAME_AS_ROOT 判定が
-- false になり、そちら側の workspace 宣言と Tsukino.Sandbox プロジェクトは
-- 自動的にスキップされる。エンジンの各静的ライブラリだけが取り込まれる。
----------------------------------------
include "External/TsukinoEngine"

----------------------------------------
-- PetaPetaPenguin（実行ファイル）
----------------------------------------
project "PetaPetaPenguin"
    location ".build/PetaPetaPenguin"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"
    forceincludes { "pch.h" }            -- 強制インクルード

    filter "action:vs*"
        buildoptions { "/permissive-" }
    filter {}

    pchheader "pch.h"
    pchsource "PetaPetaPenguin/pch.cpp"

    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}")

    local GAME_ROOT = path.getdirectory(_SCRIPT)   -- リポジトリルート

    -- デバッグ時：ワークスペースの親（＝リポジトリルート）で実行し、
    -- ソースツリー上の Assets を直接参照する
    filter "configurations:Debug"
        debugdir "%{wks.location}/.."
    filter {}

    -- リリース時：exeの隣を実行ディレクトリとし、Assetsをそこへコピーする
    filter "configurations:Release"
        debugdir "%{cfg.targetdir}"
        postbuildcommands {
            "{COPYDIR} " .. GAME_ROOT .. "/PetaPetaPenguin/Assets %{cfg.targetdir}/PetaPetaPenguin/Assets"
        }
    filter {}

    files {
        "PetaPetaPenguin/src/**.cpp",
        "PetaPetaPenguin/include/**.hpp",
        "PetaPetaPenguin/pch.cpp",
    }

    includedirs {
        "PetaPetaPenguin/include",
        "External/TsukinoEngine/Tsukino.Audio/include",
        "External/TsukinoEngine/Tsukino.GraphicsCommon/include",
        "External/TsukinoEngine/Tsukino.Engine/include",
        "External/TsukinoEngine/Tsukino.Renderer/include",
        "External/TsukinoEngine/Tsukino.BuiltIn/include",
        "External/TsukinoEngine/Tsukino.EngineIntegration/include",
        "External/TsukinoEngine/Tsukino.Core/include",
        "External/TsukinoEngine/External/cereal/include",
        "External/TsukinoEngine/External/hlslpp/include",
        "External/TsukinoEngine/External/entt/single_include",
        "External/TsukinoEngine/External/JoltPhysics",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp/Effekseer",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp/EffekseerRendererDX11",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp/EffekseerRendererCommon",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp/3rdParty",
    }

    links {
        "Tsukino.Engine",
        "Tsukino.Renderer",
        "Tsukino.GraphicsCommon",
        "Tsukino.Audio",
        "Tsukino.BuiltIn",
        "Tsukino.EngineIntegration",
        "JoltPhysics",
        "Tsukino.Core",
        "EffekseerRendererDX11",
        "EffekseerRendererCommon",
        "Effekseer",
        "d3d11",
        "dxgi",
        "d3dcompiler",
        "dwrite",
    }

    nuget { "directxtk_desktop_win10:2026.4.1.1",
            "AssimpCpp:5.0.1.6",
    }
