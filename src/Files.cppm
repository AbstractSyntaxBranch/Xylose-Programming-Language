export module files;
import std;

export std::string readFileByPath(std::string path){
    std::ifstream file;
    file.open(path);
    if(!file.is_open())
        throw std::runtime_error(std::format("Failed to open file: {}", path.c_str()));
    std::string contents;
    while(file.good()) {
        std::string line;
        std::getline(file, line);
        contents.append(line + '\n');
    }
    return contents;
}
