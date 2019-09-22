#include "pch.hpp"
#include "MongoAdaptor.hpp"
#include "Logger.hpp"

namespace MongoDb {
    struct Document {
        bson_t* ptr;

        inline void set(const std::string& pkey, const bson_oid_t& pval) {
            BSON_APPEND_OID(ptr, pkey.c_str(), &pval);
        }

        inline void set(const std::string& pkey, const bson_value_t& pval) {
            BSON_APPEND_VALUE(ptr, pkey.c_str(), &pval);
        }

        inline void set(const std::string& pkey, const std::string& pval) {
            BSON_APPEND_UTF8(ptr, pkey.c_str(), pval.c_str());
        }

        inline void set(const std::string& pkey, const bool& pval) {
            BSON_APPEND_BOOL(ptr, pkey.c_str(), pval);
        }

        inline void set(const std::string& pkey, const Document& pval) {
            BSON_APPEND_DOCUMENT(ptr, pkey.c_str(), pval.ptr);
        }

        inline void setId(const std::string& pkey, const std::string& pval) {
            set(pkey, pval);
        }

        inline std::string str() const {
            auto str = bson_as_canonical_extended_json (ptr, NULL);
            std::string rv(str);
            bson_free (str);
            return rv;
        }

        inline Document() {
            ptr = bson_new();
            if(!ptr){
                throw noq::Exception("unable to get bson");
            }
        }

        inline ~Document() {
            bson_destroy(ptr);
        }

        inline Document(const std::string& v) {
            bson_error_t error;
            ptr = bson_new_from_json ((const uint8_t *)v.c_str(), -1, &error);
            if(!ptr){
                throw noq::Exception("unable to create document from json:" + v);
            }
        }

        inline Document(bson_t* v) : ptr(v) {}
        inline Document(const Document& src) = delete;
        inline Document(Document&& src) = delete;
    };

    struct Client {
        mongoc_client_t* ptr;
        inline void open(const std::string& uri) {
            ptr = mongoc_client_new(uri.c_str());
            if(!ptr){
                throw noq::Exception("unable to create client:" + uri);
            }
        }

        inline ~Client() {
            mongoc_client_destroy(ptr);
        }
    };

    struct Cursor {
        mongoc_cursor_t* ptr;
        inline void open(mongoc_collection_t* coll, const Document& qry) {
            ptr = mongoc_collection_find_with_opts(coll, qry.ptr, NULL, NULL);
            if(!ptr){
                throw noq::Exception("unable to create cursor:" + qry.str());
            }
        }

        inline bool next(const bson_t** doc) {
            return (mongoc_cursor_next (ptr, doc));
        }

        inline ~Cursor() {
            mongoc_cursor_destroy(ptr);
        }
    };

    struct Collection {
        mongoc_collection_t* ptr;
        inline void open(const Client& client, const std::string& dbname, const std::string& cname) {
            ptr = mongoc_client_get_collection(client.ptr, dbname.c_str(), cname.c_str());
            if(!ptr){
                throw noq::Exception("unable to get collection:" + cname);
            }
        }

        inline ~Collection() {
            mongoc_collection_destroy(ptr);
        }

        inline std::vector<noq::Value> select(const Document& qry, const std::string& key) const {
            std::vector<noq::Value> rv;
            Cursor cursor;
            cursor.open(ptr, qry);
            const bson_t* doc;
            while (cursor.next(&doc)) {
                bson_iter_t it;
                if (!bson_iter_init_find(&it, doc, key.c_str()) ) {
                    continue;
                }
                switch(bson_iter_type(&it)){
                    case BSON_TYPE_UTF8:{
                        uint32_t len = 0;
                        const auto str = bson_iter_utf8(&it, &len);
                        std::string val(str, len);
                        rv.emplace_back(val);
                        break;
                    }
                    case BSON_TYPE_OID:{
                        assert(false);
                        break;
                    }
                    case BSON_TYPE_DOUBLE:{
                        const auto val = bson_iter_double(&it);
                        rv.emplace_back(val);
                        break;
                    }
                    case BSON_TYPE_DOCUMENT:{
                        assert(false);
                        break;
                    }
                    case BSON_TYPE_BOOL:{
                        const auto val = bson_iter_bool(&it);
                        rv.emplace_back(val);
                        break;
                    }
                    case BSON_TYPE_NULL:{
                        rv.emplace_back(noq::Null());
                        break;
                    }
                    case BSON_TYPE_INT32:{
                        const auto val = bson_iter_int32(&it);
                        rv.emplace_back(val);
                        break;
                    }
                    case BSON_TYPE_INT64:{
                        const unsigned int val = bson_iter_int64(&it);
                        rv.emplace_back(val);
                        break;
                    }
                    default:
                        assert(false);
                        break;
                }
            }
            return rv;
        }

        inline void insert(const Document& update) const {
            bson_error_t error;
            if (!mongoc_collection_insert_one(ptr, update.ptr, NULL, NULL, &error)) {
                throw noq::Exception(std::string() + "unable to insert data:" + error.message);
            }
        }

        inline void upsert(const Document& select, const Document& update) const {
            MongoDb::Document opts;
            opts.set("upsert", true);
            bson_error_t error;
            if (!mongoc_collection_update_one(ptr, select.ptr, update.ptr, opts.ptr, NULL, &error)) {
                throw noq::Exception(std::string() + "unable to upsert data:" + error.message);
            }
        }
    };
}

struct noq::MongoAdaptor::Impl {
    MongoDb::Client client;
    MongoDb::Collection collection;
    std::string idName = "id";

    inline void open(const noq::Url& url) {
        std::string str = "mongodb://" + url.host;
        if(url.port.length() > 0){
            str += (":" + url.port);
        }
        if(url.path.at(0) != '/'){
            str += "/";
        }
        str += url.path;
        std::string databaseName;
        std::string collectionName;
        std::string sep = "?";
        for(auto& qp : url.params) {
            if(qp.first == "database"){
                databaseName = qp.second;
            }else if(qp.first == "collection"){
                collectionName = qp.second;
            }else if(qp.first == "id"){
                idName = qp.second;
            }else{
                str += sep;
                sep = "&";
                str += (qp.first + "=" + qp.second);
            }
        }
        if(databaseName.length() == 0){
            throw noq::Exception("'database' not specified");
        }
        if(collectionName.length() == 0){
            throw noq::Exception("'collection' not specified");
        }
        if(idName == "_id"){
            throw noq::Exception("id-key cannot be '_id', please select another one (default is 'id')");
        }
        client.open(str);
        collection.open(client, databaseName, collectionName);
    }

    inline void close() {
    }

    inline void insert(const noq::Value& obj) const {
        MongoDb::Document root(obj.getString());
        collection.insert(root);
    }

    inline auto queryValueByKey(const noq::Value& key) const {
        noq::Logger().trace("queryValueByKey:key:[{1}]", 0, key);
        std::vector<noq::Value> resl;
        return resl;
    }

    inline auto queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const {
        noq::Logger().trace("queryValueByObjectKey:obj:[{1}], key:[{2}]", 0, obj, key);
        MongoDb::Document qry;
        qry.setId(idName, obj.getString());
        auto resl = collection.select(qry, key.getString());
        return resl;
    }

    inline auto queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const {
        noq::Logger().trace("queryValueByObjectKey:key:[{1}], val:[{2}]", 0, key, val);
        MongoDb::Document qry;
        qry.set(key.getString(), val.getString());
        auto resl = collection.select(qry, idName);
        return resl;
    }
};

noq::MongoAdaptor::MongoFactory::MongoFactory() {
    mongoc_init();
}

noq::MongoAdaptor::MongoFactory::~MongoFactory() {
    mongoc_cleanup();
}

std::unique_ptr<noq::Adaptor> noq::MongoAdaptor::MongoFactory::create(const noq::Url& url) const {
    std::cout << "MDB::create:" << url.host << std::endl;
    auto p = std::make_unique<MongoAdaptor>();
    p->open(url);
    return p;
}

noq::MongoAdaptor::MongoAdaptor() {
    impl_ = std::make_unique<Impl>();
}

noq::MongoAdaptor::MongoAdaptor(const noq::Url& url) {
    impl_ = std::make_unique<Impl>();
    impl_->open(url);
}

noq::MongoAdaptor::~MongoAdaptor() {
}

void noq::MongoAdaptor::open(const noq::Url& url) {
    impl_->open(url);
}

void noq::MongoAdaptor::close() {
    return impl_->close();
}

void noq::MongoAdaptor::insert(const std::string& obj) const {
    return impl_->insert(obj);
}

std::vector<noq::Value> noq::MongoAdaptor::queryValueByKey(const noq::Value& key) const {
    return impl_->queryValueByKey(key);
}

std::vector<noq::Value> noq::MongoAdaptor::queryValueByObjectKey(const noq::Value& obj, const noq::Value& key) const {
    return impl_->queryValueByObjectKey(obj, key);
}

std::vector<noq::Value> noq::MongoAdaptor::queryObjectByKeyValue(const noq::Value& key, const noq::Value& val) const {
    return impl_->queryObjectByKeyValue(key, val);
}
