#ifndef GC_DB_H
#define GC_DB_H

#include <optional>
#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database {
class GroupChatUsers {
 private:
  std::string get_sharding_hint(long chat_id, long user_id);

  long _chat_id;
  std::vector<long> _user_ids;

 public:
  static GroupChatUsers fromJSON(const std::string& str);

  long get_chat_id() const;
  const std::vector<long>& get_user_ids() const;

  long& chat_id();
  std::vector<long>& user_ids();

  static void init();
  static GroupChatUsers read_by_chat_id(long id);
  static std::vector<long> get_all_chat_ids();
  void save_to_mysql();

  Poco::JSON::Object::Ptr toJSON() const;
};
}  // namespace database

#endif
