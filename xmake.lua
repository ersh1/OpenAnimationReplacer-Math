-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes ("lib/commonlibsse-ng")

-- set project
set_project("OpenAnimationReplacer-Math")
set_version("1.0.2")
set_license("GPL-3.0")

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- set policies
set_policy("package.requires_lock", true)

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- require packages
add_requires("exprtk", "imgui v1.89.6", "rapidjson")

-- targets
target("OpenAnimationReplacer-Math")
    -- add dependencies to target
    add_deps("commonlibsse-ng")

    -- add packages to target
    add_packages("exprtk", "imgui", "rapidjson")

    -- add commonlibsse-ng plugin
    add_rules("commonlibsse-ng.plugin", {
        name = "OpenAnimationReplacer-Math",
        author = "Ersh",
        description = "SKSE64 plugin adding math statement conditions for Open Animation Replacer"
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
