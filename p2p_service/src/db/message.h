#ifndef GC_MESSAGE_DB_H
#define GC_MESSAGE_DB_H

#include <vector>
#include <optional>
#include <string>
#include "Poco/JSON/Object.h"

namespace database {
class P2PMessage {
 private:
  std::string get_sharding_hint();

  long _from_id;
  long _to_id;
  std::string _message;

 public:
  static void init();
  static P2PMessage fromJSON(const std::string& str);
  static std::vector<P2PMessage> find_by_user_id(long user_id);

  void save_to_mysql();
  Poco::JSON::Object::Ptr toJSON() const;

  long get_from_id() const;
  long get_to_id() const;
  const std::string& get_message() const;

  long& from_id();
  long& to_id();
  std::string& message();

};
}

#endif // !GC_MESSAGE_DB_H
