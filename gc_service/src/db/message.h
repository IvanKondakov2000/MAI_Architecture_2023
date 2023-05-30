#ifndef GC_MESSAGE_DB_H
#define GC_MESSAGE_DB_H

#include <optional>
#include <string>
#include "Poco/JSON/Object.h"

namespace database {
class GroupChatMessage {
 private:
  std::string get_sharding_hint();

  long _chat_id;
  long _user_id;
  std::string _message;

 public:
  static void init();
  static GroupChatMessage fromJSON(const std::string& str);
  static std::vector<GroupChatMessage> find_by_chat_id(long chat_id);

  void save_to_mysql();
  Poco::JSON::Object::Ptr toJSON() const;

  long get_chat_id() const;
  long get_user_id() const;
  const std::string& get_message() const;

  long& chat_id();
  long& user_id();
  std::string& message();

};
}

#endif // !GC_MESSAGE_DB_H
