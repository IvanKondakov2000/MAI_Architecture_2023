#include "gcm.h"

#include <exception>
#include <sstream>
#include <set>

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>
#include <string>

#include "../../../common/config.h"
#include "../../../common/database.h"
#include "../../../common/helper.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database {
std::string GroupChatUsers::get_sharding_hint(long chat_id, long user_id) {
  long hash = get_hash(std::to_string(chat_id) + std::to_string(user_id));
  return database::Database::get_sharding_hint(hash);
}

void GroupChatUsers::init() {
  try {
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      Poco::Data::Session session = database::Database::get().create_session();
      Statement create_stmt(session);
      create_stmt << "CREATE TABLE IF NOT EXISTS `GroupChats` ("
                  << "`id` INT NOT NULL AUTO_INCREMENT,"
                  << "`chat_id` INT NOT NULL,"
                  << "`user_id` INT NOT NULL,"
                  << "PRIMARY KEY (`id`), KEY `uid` (`chat_id`)"
                  << ");"
                  << hint,
          now;
    }
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
    throw;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
    throw;
  }
}

Poco::JSON::Object::Ptr GroupChatUsers::toJSON() const {
  Poco::JSON::Array user_ids;
  for (const auto& e : get_user_ids()) {
    user_ids.add(e);
  }

  Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

  root->set("chat_id", _chat_id);
  root->set("user_ids", user_ids);

  return root;
}

GroupChatUsers GroupChatUsers::fromJSON(const std::string& str) {
  GroupChatUsers chat;
  Poco::JSON::Parser parser;
  Poco::Dynamic::Var result = parser.parse(str);
  Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  chat.chat_id() = object->getValue<long>("chat_id");

  auto user_array = object->getArray("user_ids");
  for (size_t i = 0; i < user_array->size(); ++i) {
    chat.user_ids().push_back(user_array->get(i));
  }

  return chat;
}

std::vector<long> GroupChatUsers::get_all_chat_ids() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement select(session);
    std::set<long> chat_ids_set;
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      select << "SELECT DISTINCT chat_id "
                "FROM GroupChats " + hint;
      select.execute();

      Poco::Data::RecordSet rs(select);

      for (auto& row : rs) {
        chat_ids_set.insert(row["chat_id"].convert<long>());
      }
    }

    std::vector<long> chat_ids(chat_ids_set.begin(), chat_ids_set.end());
    return chat_ids;
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
  }

  return {};
}

GroupChatUsers GroupChatUsers::read_by_chat_id(long id) {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement select(session);
    GroupChatUsers c;
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      select << "SELECT id, chat_id, user_id "
                "FROM GroupChats where chat_id=" + std::to_string(id) + hint;
      select.execute();

      Poco::Data::RecordSet rs(select);

      c.chat_id() = id;
      for (auto& row : rs) {
        c.user_ids().push_back(row["user_id"].convert<long>());
      }
    }

    return c;
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
  }

  return {};
}

void GroupChatUsers::save_to_mysql() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement insert(session);

    struct {
      long chat_id;
      long user_id;
    } entry;

    insert
        << "INSERT INTO GroupChats (chat_id, user_id) "
           "VALUES(?, ?)" + get_sharding_hint(entry.chat_id, entry.user_id),
        use(entry.chat_id), use(entry.user_id);

    entry.chat_id = _chat_id;
    for (const auto& e : _user_ids) {
      entry.user_id = e;
      insert.execute();
    }

    std::cout << "inserted for chat:" << _chat_id << std::endl;

  } catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
    throw;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
    throw;
  }
}

long GroupChatUsers::get_chat_id() const { return _chat_id; }
const std::vector<long>& GroupChatUsers::get_user_ids() const { return _user_ids; }

long& GroupChatUsers::chat_id() { return _chat_id; }
std::vector<long>& GroupChatUsers::user_ids() { return _user_ids; }

}
