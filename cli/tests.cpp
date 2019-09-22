#include "noql/noql.hpp"
#include "tests.hpp"
#include <fstream>
#include <sstream>

namespace {
    struct TestEntry;
    TestEntry* headEntry = nullptr;
    TestEntry* tailEntry = nullptr;

    struct TestEntry {
        typedef void (*fnT)(noq::unittest::Result&, const char*, const std::string&);
        const char* name = nullptr;
        fnT fn;
        TestEntry* next = nullptr;
        inline TestEntry(const char* n, fnT tfn) : name(n), fn(tfn) {
            if(headEntry == nullptr){
                headEntry = this;
            }
            if(tailEntry != nullptr){
                tailEntry->next = this;
            }
            tailEntry = this;
        }
    };

#define UNITTEST(tname) \
    void tname(noq::unittest::Result& result, const char* name, const std::string& p0); \
    static TestEntry _te_##tname(#tname, tname); \
    void tname(noq::unittest::Result& result, const char* name, const std::string& p0)

////////////////////////////////////////////
    template <typename T>
    inline std::string str(const std::vector<T>& val) {
        std::ostringstream os;
        std::string sep;
        os << "[";
        for(auto& s : val){
            os << sep << s;
            sep = ",";
        }
        os << "]";
        return os.str();
    }

    inline std::string str(const noq::Function& val) {
        std::ostringstream os;
        val.str(os);
        return os.str();
    }

    inline bool verify(noq::unittest::Result& result, const char* id, const bool& cond, const std::string& msg) {
        std::ostringstream os;
        os << "UNITTEST:";
        if(cond){
            os << "PASSED  :";
        }else{
            os << "FAILED  :";
            ++result.failed;
        }
        os << id << ", " << msg;

        ++result.total;
        result.fn(os.str());
        return cond;
    }

    void presult(noq::unittest::Result& result, const std::string& id, const bool& passed, const std::string& src, const std::string& msg, const std::vector<noq::Function>& expected, const std::vector<noq::Function>& received) {
        std::ostringstream os;
        os << "UNITTEST:";
        if(passed){
            os << "PASSED  :";
        }else{
            os << "FAILED  :";
            ++result.failed;
        }
        os << id;
        if(src.length() > 0){
            os << ":" << src;
        }
        os << ". ";
        if(msg.length() > 0){
            os << msg << ". ";
        }
        os << "Expected:" << str(expected) << ", Received:" << str(received);
        ++result.total;
        result.fn(os.str());
    }

    UNITTEST(string_test000) {
        static const char* test_module =
            "INSERT DOCUMENT {"
                "\"id\":\"amitabh\","
                "\"age\":66,"
                "\"height\":190.34,"
                "\"male\":true,"
                "\"single\":false,"
                "\"nri\":null,"
                "\"children\":[\"abc\", \"xyz\"],"
            "};\n"

            "INSERT DOCUMENT {"
                "\"id\":\"abhishek\","
                "\"parent\":\"amitabh\","
            "};\n"

            "RULE father(@x,@y) := parent(@x,@y) && male(@y,true);"
            "VERIFY father(\"abhishek\", @Father) := [father(\"abhishek\",\"amitabh\")];"
            ;

        noq::Connection conn;
        conn.open(result.tsrc);
        conn.testCB_ = [&result, &name](const bool& passed, const std::string& msg, const std::vector<noq::Function>& expected, const std::vector<noq::Function>& received){
            presult(result, name, passed, "", msg, expected, received);
        };
        conn.execString(test_module);
    }

    UNITTEST(file_test000) {
        std::vector<std::string> pathList;
        if(p0.length() > 0){
            pathList.push_back(p0);
        }else{
            for(auto& f : std::filesystem::directory_iterator(result.tdir)){
                pathList.push_back(f.path());
            }
        }
        for(auto& f : pathList){
            noq::Connection conn;
            conn.open(result.tsrc);
            conn.testCB_ = [&result, &name](const bool& passed, const std::string& msg, const std::vector<noq::Function>& expected, const std::vector<noq::Function>& received){
                presult(result, name, passed, "", msg, expected, received);
            };
            conn.execFile(f);
        }

    }

    UNITTEST(compile_test001) {
        static const char* test_module =
            "RULE expr(@x,@y, @z) := (@x < @y) && (@z = 1);"
            ;

        noq::Connection conn;
        conn.testCB_ = [&result, &name](const bool& passed, const std::string& msg, const std::vector<noq::Function>& expected, const std::vector<noq::Function>& received){
            presult(result, name, passed, "", msg, expected, received);
        };
        conn.execString(test_module);

        noq::Query qry;
        qry.compile("expr(@x, @y, @z)");
        qry.set("x", noq::Value(100));
        qry.set("y", noq::Value(234));
        qry.exec(conn);

        verify(result, name, (qry.resl().size() == 1), "qry.resl().size() == 1");
        for(auto& r : qry){
            verify(result, name, (qry.get("z", r).value() == 1), "qry.get(\"z\", r).value() == noq::Value(1)");
        }
    }

    UNITTEST(url_test001) {
        auto url = noq::Url::parse("mongodb://localhost:27017/xx/zz/?k1=v1");
        verify(result, name, (url.protocol == "mongodb"), "url.protocol == \"mongodb\", " + url.protocol);
        verify(result, name, (url.host == "localhost"), "url.host == \"localhost\", " + url.host);
        verify(result, name, (url.port == "27017"), "url.port == \"27017\", " + url.port);
        verify(result, name, (url.path == "/xx/zz/"), "url.path == \"/xx/zz/\", " + url.path);
        verify(result, name, (url.params.size() == 1), "url.params.size() == 1");
        verify(result, name, ((url.params.size() == 1) && (url.params.at("k1") == "v1")), "(url.params.size() == 1) && (url.params.at(\"k1\") == \"v1\")");
    }

    UNITTEST(url_test002) {
        auto url = noq::Url::parse("mongodb://localhost/xx/zz/?k1=v1");
        verify(result, name, (url.protocol == "mongodb"), "url.protocol == \"mongodb\", " + url.protocol);
        verify(result, name, (url.host == "localhost"), "url.host == \"localhost\", " + url.host);
        verify(result, name, (url.port == ""), "url.port == \"\", " + url.port);
        verify(result, name, (url.path == "/xx/zz/"), "url.path == \"/xx/zz/\", " + url.path);
        verify(result, name, (url.params.size() == 1), "url.params.size() == 1");
        verify(result, name, ((url.params.size() == 1) && (url.params.at("k1") == "v1")), "(url.params.size() == 1) && (url.params.at(\"k1\") == \"v1\")");
    }
}

void noq::unittest::runTest(const std::string& toTestx, Result& result) {
    auto toTest = toTestx;
    std::string toTestP0;
    auto cpos = toTestx.find(":");
    if(cpos != std::string::npos){
        toTest = toTestx.substr(0,cpos);
        toTestP0 = toTestx.substr(cpos+1);
    }
    if(toTest == "none"){
        return;
    }

    auto entry = headEntry;
    while(entry != nullptr){
        if((toTest == "all") || (toTest == std::string(entry->name))){
            entry->fn(result, entry->name, toTestP0);
        }
        entry = entry->next;
    }
}
