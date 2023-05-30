#include "message.h"

#include <exception>
#include <sstream>
#include <string>
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
std::string P2PMessage::get_sharding_hint() {
  long hash = get_hash(std::to_string(_from_id) + std::to_string(_to_id));
  return database::Database::get_sharding_hint(hash);
}

void P2PMessage::init() {
  try {
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      Poco::Data::Session session = database::Database::get().create_session();
      Statement create_stmt(session);
      create_stmt << "CREATE TABLE IF NOT EXISTS `PToPMessages` ("
                  << "`id` INT NOT NULL AUTO_INCREMENT,"
                  << "`from_id` INT NOT NULL,"
                  << "`to_id` INT NOT NULL,"
                  << "`message` VARCHAR(4096) NOT NULL,"
                  << "PRIMARY KEY (`id`)"
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

P2PMessage P2PMessage::fromJSON(const std::string& str) {
  P2PMessage msg;
  Poco::JSON::Parser parser;
  Poco::Dynamic::Var result = parser.parse(str);
  Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  msg.from_id() = object->getValue<long>("from_id");
  msg.to_id() = object->getValue<long>("to_id");
  msg.message() = object->getValue<std::string>("message");

  return msg;
}

std::vector<P2PMessage> P2PMessage::find_by_user_id(long user_id) {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement select(session);
    std::vector<P2PMessage> msgs;
    for (const auto& hint : database::Database::get_all_sharding_hints()) {
      select << "SELECT from_id, to_id, message "
                "FROM PToPMessages WHERE from_id=" + std::to_string(user_id) + " OR to_id=" + std::to_string(user_id) + " " + hint;
      select.execute();

      Poco::Data::RecordSet rs(select);

      for (auto& row : rs) {
        P2PMessage msg;
        msg.from_id() = row["from_id"].convert<long>();
        msg.to_id() = row["to_id"].convert<long>();
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

void P2PMessage::save_to_mysql() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement insert(session);

    struct {
      long from_id;
      long to_id;
      std::string message;
    } entry;

    insert
        << "INSERT INTO PToPMessages (from_id, to_id, message) "
           "VALUES(?, ?, ?)" + get_sharding_hint(),
        use(entry.from_id), use(entry.to_id), use(entry.message);

    entry.from_id = _from_id;
    entry.to_id = _to_id;
    entry.message = _message;
    insert.execute();

    std::cout << "inserted p2p msg:" << _from_id << ' ' << _from_id << std::endl;

  } catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
    throw;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
    throw;
  }
}

Poco::JSON::Object::Ptr P2PMessage::toJSON() const {
  Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

  root->set("from_id", _from_id);
  root->set("to_id", _to_id);
  root->set("message", _message);

  return root;
}

long P2PMessage::get_from_id() const { return _from_id; }
long P2PMessage::get_to_id() const { return _to_id; }
const std::string& P2PMessage::get_message() const { return _message; }

long& P2PMessage::from_id() { return _from_id; }
long& P2PMessage::to_id() { return _to_id; }
std::string& P2PMessage::message() { return _message; }

}
