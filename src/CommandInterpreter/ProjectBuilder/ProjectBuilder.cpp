#include "ProjectBuilder.hpp"
#include "../../Data/Diagnostics.hpp"
#include "../CommandInterpreter.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>

void createNewProject(int argc, char **argv){
    std::string name;
    if(argc <= 2) {
        fmt::println(diag::ERROR, "Error: Expected a project name.");
        std::exit(-1);
    }
    if(argc > 3){
        unknownFlag(argc-1, argv+1);
        std::exit(-1);
    }
    name = std::string{argv[2]};
    if(std::ranges::any_of("#%&{}\\<>*?/$!'\":@+`|=", [&](char c){return name.contains(c);})
    ) {
        fmt::println(diag::ERROR, "Error: Project name is not valid.");
        std::exit(-1);
    }
    if(std::filesystem::exists(name)){
        fmt::println(diag::ERROR, "Error: Folder already exists.");
    }
    std::filesystem::create_directory(name);
    std::filesystem::create_directory(name + "/src");
    std::ofstream main_file;
    std::ofstream gitignore_file;
    main_file.open(name + "/src/main.xylo");
    gitignore_file.open(name + "/.gitignore");
    main_file <<
    std::string{
        #embed "main.xylo.template"
        ,0
    };
    gitignore_file <<
    std::string{
        #embed ".gitignore.template"
        ,0
    };
}
