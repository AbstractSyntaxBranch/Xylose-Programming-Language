import lexer;
import parser;
import files;
import std;

int main() {
    try{
    std::system("clear");
    std::string path = "/home/robin/Projects/compiler/src/example.txt";
    Parser parser = Parser::getDefault(path);
    parser.parseToGlobals();
    parser.foldDuplicateEntries();
    // parser.printFunctions();
    // parser.printTypedefs();
    // const auto& a = parser.getLines(parser.getTokenViews(**parser.functions.lower_bound("a")->second.implementation));
    const auto& b = parser.parseLinesFromTokenList(**parser.functions.lower_bound("a")->second.implementation, parser.functions.lower_bound("a")->second.generic_fields);

    // const auto& index = parser.getIndexTopOfAST(a.front());
    // std::println("Top: {}", !index ? "NONE" : a.front().at(*index).front().value);
    // const auto& b = parser.getLineAST(a.front());
    parser.printFunctions();
    for(const auto& h : b){
        std::println("---LINE:---");
        parser.printLineAST(h);
    }
    }catch(const std::exception& e){
        std::cout << "ERROR: " << e.what() << std::endl;
    }

    // parser.printTokenProcessingRange(a);
    return 0;
}
