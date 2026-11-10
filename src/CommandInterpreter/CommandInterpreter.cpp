#include "CommandInterpreter.hpp"
#include "Build/Build.hpp"
#include "Help/Help.hpp"
#include "Version/Version.hpp"
#include "ProjectBuilder/ProjectBuilder.hpp"
#include "../Data/Diagnostics.hpp"
#include "fmt/base.h"
#include <cstdlib>
#include <cstring>
#include <span>

void unknownFlag(int argc, char **argv){
    for(const char *flag : std::span{argv + 2, argv + argc})
        fmt::println(diag::WARNING, "Unknown flag: {}", flag);
}

void handleRequest(int argc, char **argv){
    bool no_input = argc < 2;
    bool has_flags = argc > 2;
    if(no_input){
        showVersion();
        showHelp();
        std::exit(0);
    }
    if(!std::strcmp(argv[1], "version") || !std::strcmp(argv[1], "-v")){
        if(has_flags)
            unknownFlag(argc, argv);
        showVersion();
        std::exit(0);
    }
    if(!std::strcmp(argv[1], "help") || !std::strcmp(argv[1], "-h")){
        if(has_flags)
            unknownFlag(argc, argv);
        showHelp();
        std::exit(0);
    }
    else if(!std::strcmp(argv[1], "new")){
        createNewProject(argc, argv);
        std::exit(0);
    }
    else if(!std::strcmp(argv[1], "build")){
        build(argc, argv);
        std::exit(0);
    }
    fmt::println(diag::ERROR, "Error: Unknown command: {}", argv[1]);
}
