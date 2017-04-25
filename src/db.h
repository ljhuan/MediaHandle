#ifndef DB_H_
#define DB_H_

#include <string>
#include "odb/database.hxx"
#include "odb/transaction.hxx"
#include "odb/sqlite/database.hxx"
#include "odb/schema-catalog.hxx"
#include "singleton.h"
#include "log.h"

class MPDB {
public:
    MPDB() : sqliteConnectionFactory_(new odb::sqlite::connection_pool_factory(16)) {
        std::shared_ptr<odb::database> db(new odb::sqlite::database("mp.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, true, "", sqliteConnectionFactory_));
        odb::transaction t(db->begin());
        odb::schema_catalog::create_schema(*db);
        t.commit();
    }

    std::shared_ptr<odb::database> sqliteDB() {
        try {
            std::shared_ptr<odb::database> db(new odb::sqlite::database("mp.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, true, "", sqliteConnectionFactory_));
            return db;
        }
        catch (const std::exception& ex) {
            LOG(ERROR) << "ODB ERROR:" << ex.what();
            return nullptr;
        }
    }
    ~MPDB() {}
private:
    std::auto_ptr<odb::sqlite::connection_factory> sqliteConnectionFactory_;
    SINGLETON_DECL(MPDB);
};

#endif  // DB_H_
