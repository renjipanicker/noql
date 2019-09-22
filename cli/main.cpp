#include "noql/noql.hpp"
#include "tests.hpp"
#include <fstream>

namespace {
    bool done = false;

    inline void processLine(noq::Connection& conn, const std::string& line) {
        if((line == "help") || (line == "?") || (line == ".h") || (line == ".help")){
            std::cout << ".q                        Quit" << std::endl;
            std::cout << ".h                        Help" << std::endl;
            std::cout << ".r <filename>             Read <filename>" << std::endl;
            std::cout << "INSERT DOCUMENT <json>;   Insert <json> document" << std::endl;
            std::cout << "RULE <rule>;              Set <rule>" << std::endl;
            std::cout << "?<query>;                 Execute <query>" << std::endl;
            std::cout << std::endl;
            std::cout << "e.g:" << std::endl;
            std::cout << "INSERT DOCUMENT {\"id\":\"anakin\",\"child\":\"leia\"};" << std::endl;
            std::cout << "INSERT DOCUMENT {\"id\":\"leia\",\"gender\":\"female\"};" << std::endl;
            std::cout << "RULE daughter(@x,@y) := child(@x,@y) && gender(@y,\"female\");" << std::endl;
            std::cout << "?daughter(\"anakin\",@y);" << std::endl;
            return;
        }
        if(line.at(0) == '.'){
            auto cmd = line;
            std::string p0;
            auto spos = line.find(" ");
            if(spos != std::string::npos){
                cmd = line.substr(0,spos);
                p0 = line.substr(spos+1);
            }

            if((cmd == ".q") || (cmd == ".quit")){
                done = true;
                return;
            }

            if((cmd == ".r") || (cmd == ".read")){
                if(p0 == "") {
                    p0 = "test.noql";
                }
                std::cout << "reading " << p0 << std::endl;
                std::fstream ss(p0);
                conn.exec(ss);
                return;
            }
            std::cout << "unknown command" << std::endl;
            return;
        }
        if(line.at(0) == '?'){
            auto qstr = line.substr(1);
            noq::Query query;
            query.compile(qstr);
            query.exec(conn);
            std::cout << "Found " << query.resl().size() << " result" << ((query.resl().size()==1)?"":"s") << "." << std::endl;
            for(auto& r : query){
                std::cout << r << std::endl;
            }
            return;
        }
        conn.execString(line);
    }
}

int main(int argc, const char** argv){
    std::string toTest = "none";
    std::string tdir = std::filesystem::path(__FILE__).parent_path().string() + "/tests";
    std::string tsrc = "json://memory/?id=id";
    bool repl = true;

    const char** argp = argv;
    ++argp;
    while(*argp){
        std::string arg(*argp);
        if(arg == "-h"){
            std::cout << "Usage:vmrun <options>" << std::endl;
            std::cout << "Options" << std::endl;
#ifndef NDEBUG
            std::cout << " -t <'all'|'none'|test_name>" << std::endl;
#endif
            return 1;
        }else if(arg == "-i"){
            repl = true;
#ifndef NDEBUG
        }else if(arg == "-t"){
            ++argp;
            const std::string val(*argp);
            toTest = val;
            repl = false;
        }else if(arg == "-td"){
            ++argp;
            const std::string val(*argp);
            tdir = val;
#endif
        }else if(arg == "-ts"){
            ++argp;
            const std::string val(*argp);
            tsrc = val;
        }else{
            std::cout << "unknown option:" << arg << std::endl;
        }
        ++argp;
    }

    noq::init();

    if(toTest != "none"){
        noq::unittest::Result result;
        result.tdir = tdir;
        result.tsrc = tsrc;
        result.fn = [](const std::string& msg){
            std::cout << msg << std::endl;
        };

        noq::unittest::runTest(toTest, result);
        std::cout << "UNITTEST(PASSED/TOTAL):" << (result.total - result.failed) << "/" << result.total << std::endl;
    }

    if(repl){
        noq::Connection conn;
        conn.open(tsrc);
        std::cout << "Connected to " << tsrc << std::endl;
        std::cout << "Type ? for help." << std::endl;
        while(!done) {
            std::string line;
            std::cout << ">";
            if(!std::getline(std::cin, line)){
                break;
            }
            if(line.length() == 0){
                continue;
            }
            try{
                processLine(conn, line);
                std::cout << "OK" << std::endl;
            }catch(const std::exception& e){
                std::cout << "Error:" << e.what() << std::endl;
            }
        }
    }
    return 0;
}
