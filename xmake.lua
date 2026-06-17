add_rules("mode.debug", "mode.release")
set_languages("c++26")

target("compiler")
    set_kind("binary")
    set_optimize("none");
    set_optimize("smallest");
    add_files("src/*.cpp", "src/*.cppm")
