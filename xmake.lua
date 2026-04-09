-- include subprojects
includes("lib/commonlibsse-ng")

-- set project constants
set_project("OpenAnimationReplacer-Math")
set_version("1.0.3")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- require packages
add_requires("exprtk", "imgui v1.89.6", "rapidjson")

-- define targets
target("OpenAnimationReplacer-Math")
    add_rules("commonlibsse-ng.plugin", {
        name = "OpenAnimationReplacer-Math",
        author = "Ersh",
        description = "SKSE64 plugin adding math statement conditions for Open Animation Replacer"
    })

    -- add packages to target
    add_packages("exprtk", "imgui", "rapidjson")

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
