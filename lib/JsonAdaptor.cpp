#include "pch.hpp"
#include "JsonAdaptor.hpp"
#include "Logger.hpp"

namespace {
    inline noq::Value fromJson(const noq::JsonAdaptor::JsonValue& val){
        switch(val.type()){
        case nlohmann::json::value_t::boolean:
            return noq::Value(val.get<bool>());
        case nlohmann::json::value_t::number_float:
            return noq::Value(val.get<double>());
        case nlohmann::json::value_t::number_integer:
            return noq::Value(val.get<int>());
        case nlohmann::json::value_t::number_unsigned:
            return noq::Value(val.get<unsigned int>());
        case nlohmann::json::value_t::string:
            return noq::Value(val.get<std::string>());
        case nlohmann::json::value_t::null:
            return noq::Value(noq::Null());
        case nlohmann::json::value_t::array:{
            auto rv = noq::Array();
            for(auto& j : val){
                rv.push_back(fromJson(j));
            }
            return noq::Value(rv);
        }
        default:
            assert(false);
            return noq::Value();
        }
    }

    // add jobj to resl, if jval == val
    inline void checkValAndAddObject(const std::string& idName, const noq::Value& val, const noq::Value& jval, const noq::JsonAdaptor::JsonValue& jobj, std::vector<noq::Value>& resl) {
        if(jval.isArray()) {
            for(auto& j : jval.getArray()){
                if(j != val){
                    noq::Logger().trace("JsonAdaptor::queryObjectByKeyValue:[{0}] key-value does not match", jobj);
                    return;
                }
            }
        }else{
            if(jval != val){
                noq::Logger().trace("JsonAdaptor::queryObjectByKeyValue:[{0}] key-value does not match", jobj);
                return;
            }
        }

        auto oit = jobj.find(idName);
        if(oit == jobj.end()){
            noq::Logger().trace("JsonAdaptor::queryObjectByKeyValue:[{0}] does not have {1}", jobj, idName);
            return;
        }

        auto& jobjv = *oit;
        noq::Logger().trace("JsonAdaptor::queryObjectByKeyValue:[{0}] matches", jobj);
        resl.push_back(fromJson(jobjv));
    }
}

std::unique_ptr<noq::Adaptor> noq::JsonAdaptor::JsonFactory::create(const noq::Url& url) const {
    std::unique_ptr<JsonAdaptor> p = std::make_unique<JsonAdaptor>();
    p->idName_ = "id";
    auto iit = url.params.find("id");
    if(iit != url.params.end()){
        p->idName_ = iit->second;
    }
    if(url.host == "file"){
        std::ifstream is(url.path);
        p->loadStream(is);
    }else if(url.host == "string"){
        auto dit = url.params.find("data");
        if(dit == url.params.end()){
            throw noq::Exception("URL should contain data parameter");
        }

        std::istringstream is(dit->second);
        p->loadStream(is);
    }else if(url.host == "memory"){
        // do nothing
    }else{
        throw noq::Exception("unrecognized json type '" + url.host + "' in '" + url.src + "'");
    }
    return p;
}

void noq::JsonAdaptor::loadStream(std::istream& is) {
    is >> root_;
}

void noq::JsonAdaptor::saveStream(std::ostream& os) {

    os << root_;
}

void noq::JsonAdaptor::insert(const std::string& obj) const {
    std::istringstream is(obj);
    JsonValue jobj;
    is >> jobj;
    root_.push_back(jobj);
    noq::Logger().trace("JsonAdaptor::insert:{0}", jobj);
}

std::vector<noq::Value> noq::JsonAdaptor::queryValueByKey(const noq::Value& key) const {
    noq::Logger().trace("JsonAdaptor::queryValueByKey:key:[{1}]", 0, key);
    std::vector<noq::Value> resl;

    for(auto it = root_.begin(), ite = root_.end(); it != ite; ++it) {
        // get the next json object in the list
        auto& jobj = *it;
        if(!jobj.is_object()) {
            noq::Logger().trace("JsonAdaptor::queryValueByKey:[{0}] is not a JSON object", jobj);
            continue;
        }

        // check if it has 'key' in it
        auto kit = jobj.find(key.getString());
        if(kit == jobj.end()){
            noq::Logger().trace("JsonAdaptor::queryValueByKey:[{0}] does not have key {1}", jobj, key);
            continue;
        }

        // if yes, add the value to result
        auto& jval = *kit;
        noq::Logger().trace("JsonAdaptor::queryValueByKey:[{0}] matches {1}", jobj, key);
        resl.push_back(fromJson(jval));
    }
    return resl;
}

std::vector<noq::Value> noq::JsonAdaptor::queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const {
    noq::Logger().trace("JsonAdaptor::queryValueByObjectKey:obj:[{1}], key:[{2}]", 0, obj, key);
    std::vector<noq::Value> resl;

    for(auto it = root_.begin(), ite = root_.end(); it != ite; ++it) {
        // get next object in the list
        auto& jobj = *it;
        if(!jobj.is_object()) {
            noq::Logger().trace("JsonAdaptor::queryValueByObjectKey:[{0}] is not a JSON object", jobj);
            continue;
        }

        // check if it has the 'id' attribute
        auto jit = jobj.find(idName_);
        if(jit == jobj.end()){
            noq::Logger().trace("JsonAdaptor::queryValueByObjectKey:[{0}] does not have {1}", jobj, idName_);
            continue;
        }

        // check if its id matches the id of the object we are looking at
        auto& jobjv = *jit;
        if(fromJson(jobjv) != obj){
            noq::Logger().trace("JsonAdaptor::queryValueByObjectKey:[{0}] does not match {1}", jobj, obj);
            continue;
        }

        // check if this object has the 'key' we are looking for
        auto kit = jobj.find(key.getString());
        if(kit == jobj.end()){
            noq::Logger().trace("JsonAdaptor::queryValueByObjectKey:[{0}] does not have key {1}", jobj, key);
            continue;
        }

        // if yes, add the value to the result
        auto& jval = *kit;
        auto vali = fromJson(jval);
        if(vali.isArray()){
            for(auto& v : vali.getArray()){
                noq::Logger().trace("JsonAdaptor::queryValueByObjectKey:[{0}] matches {1}::{2}", jobj, obj, key);
                resl.push_back(fromJson(jval));
            }
        }else{
            noq::Logger().trace("JsonAdaptor::queryValueByObjectKey:[{0}] matches {1}::{2}", jobj, obj, key);
            resl.push_back(fromJson(jval));
        }
    }
    return resl;
}

std::vector<noq::Value> noq::JsonAdaptor::queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const {
    noq::Logger().trace("JsonAdaptor::queryObjectByKeyValue:key:[{1}], val:[{2}]", 0, key, val);
    std::vector<noq::Value> resl;
    for(auto it = root_.begin(), ite = root_.end(); it != ite; ++it) {
        auto& jobj = *it;
        if(!jobj.is_object()) {
            noq::Logger().trace("JsonAdaptor::queryObjectByKeyValue:[{0}] is not a JSON object", jobj);
            continue;
        }

        auto jit = jobj.find(key.getString());
        if(jit == jobj.end()){
            noq::Logger().trace("JsonAdaptor::queryObjectByKeyValue:[{0}] does not have key {1}", jobj, key);
            continue;
        }

        auto& jval = *jit;
        auto vali = fromJson(jval);

        if(vali.isArray()){
            for(auto& v : vali.getArray()){
                checkValAndAddObject(idName_, v, val, jobj, resl);
            }
        }else{
            checkValAndAddObject(idName_, vali, val, jobj, resl);
        }
    }
    return resl;
}
