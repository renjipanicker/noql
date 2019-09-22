#pragma once

#include "Exception.hpp"

namespace noq {
    struct Url {
        std::string src;
        std::string protocol;
        std::string host;
        std::string port;
        std::string path;
        std::map<std::string, std::string> params;

        static inline const Url parse(const std::string& str) {
            Url url;
            url.src = str;
            enum class State {
                Protocol0,
                Protocol1,
                Protocol2,
                Host0,
                Port0,
                Path0,
                ParamKey0,
                ParamVal0,
                ParamVal1,
                ParamVal2,
            };
            State state = State::Protocol0;
            std::string key;
            std::string val;
            for(auto& ch : str) {
                switch(state){
                case State::Protocol0:
                    if(ch == ':'){
                        state = State::Protocol1;
                        break;
                    }
                    url.protocol += ch;
                    break;
                case State::Protocol1:
                    if(ch != '/'){
                        throw noq::Exception("unable to parse URI:[" + str + "]");
                    }
                    state = State::Protocol2;
                    break;
                case State::Protocol2:
                    if(ch != '/'){
                        throw noq::Exception("unable to parse URI:[" + str + "]");
                    }
                    state = State::Host0;
                    break;
                case State::Host0:
                    if(ch == ':'){
                        state = State::Port0;
                        break;
                    }
                    if(ch == '/'){
                        url.path += ch;
                        state = State::Path0;
                        break;
                    }
                    url.host += ch;
                    break;
                case State::Port0:
                    if(ch == '/'){
                        url.path += ch;
                        state = State::Path0;
                        break;
                    }
                    url.port += ch;
                    break;
                case State::Path0:
                    if(ch == '?'){
                        state = State::ParamKey0;
                        break;
                    }
                    url.path += ch;
                    break;
                case State::ParamKey0:
                    if(ch == '='){
                        state = State::ParamVal0;
                        break;
                    }
                    key += ch;
                    break;
                case State::ParamVal0:
                    if(ch == '\''){
                        state = State::ParamVal1;
                        break;
                    }
                    if(ch == '&'){
                        url.params[key] = val;
                        key = "";
                        val = "";
                        state = State::ParamKey0;
                        break;
                    }
                    val += ch;
                    break;
                case State::ParamVal1:
                    if(ch == '\''){
                        url.params[key] = val;
                        key = "";
                        val = "";
                        state = State::ParamVal2;
                        break;
                    }
                    val += ch;
                    break;
                case State::ParamVal2:
                    if(ch == '&'){
                        state = State::ParamKey0;
                        break;
                    }
                    break;
                }
            }

            switch(state){
            case State::ParamVal0:
            case State::ParamVal1:
                url.params[key] = val;
                break;
            default:
                break;
            }

            return url;
        }
    };
    

}
