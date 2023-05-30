#ifndef DATABASE_H
#define DATABASE_H

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/SessionPool.h>
#include <memory>
#include <string>
#include <vector>

namespace database {
class Database {
 private:
  std::string _connection_string;
  std::unique_ptr<Poco::Data::SessionPool> _pool;
  Database();

 public:
  static Database& get();
  Poco::Data::Session create_session();
  static size_t get_max_shard();
  static std::string get_sharding_hint(long hash);
  static std::vector<std::string> get_all_sharding_hints();
};
}  // namespace database
#endif
