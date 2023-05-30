#include "message.h"

#include <exception>
#include <sstream>
#include <vector>

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>

#include "../../../common/config.h"
#include "../../../common/database.h"
#include "../../../common/helper.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database {
std::string GroupChatMessage::get_sharding_hint() {
  long hash = get_hash(std::to_string(_chat_id) + std::to_string(_user_id));
  return database::Database::get_sharding_hint(hash);
}

void GroupChatMessage::init() {
  try {
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      Poco::Data::Session session = database::Database::get().create_session();
      Statement create_stmt(session);
      create_stmt << "CREATE TABLE IF NOT EXISTS `GCMessages` ("
                  << "`id` INT NOT NULL AUTO_INCREMENT,"
                  << "`chat_id` INT NOT NULL,"
                  << "`user_id` INT NOT NULL,"
                  << "`message` VARCHAR(4096) NOT NULL,"
                  << "PRIMARY KEY (`id`), KEY `cid` (`chat_id`)"
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

GroupChatMessage GroupChatMessage::fromJSON(const std::string& str) {
  GroupChatMessage msg;
  Poco::JSON::Parser parser;
  Poco::Dynamic::Var result = parser.parse(str);
  Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  msg.chat_id() = object->getValue<long>("chat_id");
  msg.user_id() = object->getValue<long>("user_id");
  msg.message() = object->getValue<std::string>("message");

  return msg;
}

std::vector<GroupChatMessage> GroupChatMessage::find_by_chat_id(long chat_id) {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement select(session);
    std::vector<GroupChatMessage> msgs;
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      select << "SELECT chat_id, user_id, message "
                "FROM GCMessages where chat_id="+ std::to_string(chat_id) + hint;
      select.execute();

      Poco::Data::RecordSet rs(select);

      for (auto& row : rs) {
        GroupChatMessage msg;
        msg.chat_id() = row["chat_id"].convert<long>();
        msg.user_id() = row["user_id"].convert<long>();
        msg.message() = row["message"].convert<std::string>();
        msgs.push_back(msg);
      }
    }

    return msgs;
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
  }

  return {};
}

void GroupChatMessage::save_to_mysql() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement insert(session);

    struct {
      long chat_id;
      long user_id;
      std::string message;
    } entry;

    insert
        << "INSERT INTO GCMessages (chat_id, user_id, message) "
           "VALUES(?, ?, ?)" + get_sharding_hint(),
        use(entry.chat_id), use(entry.user_id), use(entry.message);

    entry.chat_id = _chat_id;
    entry.user_id = _user_id;
    entry.message = _message;
    insert.execute();

    std::cout << "inserted group msg:" << _chat_id << ' ' << _chat_id << std::endl;

  } catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
    throw;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
    throw;
  }
}

Poco::JSON::Object::Ptr GroupChatMessage::toJSON() const {
  Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

  root->set("chat_id", _chat_id);
  root->set("user_id", _user_id);
  root->set("message", _message);

  return root;
}

long GroupChatMessage::get_chat_id() const { return _chat_id; }
long GroupChatMessage::get_user_id() const { return _user_id; }
const std::string& GroupChatMessage::get_message() const { return _message; }

long& GroupChatMessage::chat_id() { return _chat_id; }
long& GroupChatMessage::user_id() { return _user_id; }
std::string& GroupChatMessage::message() { return _message; }

}
