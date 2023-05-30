import random
from faker import Faker

import basemod


fake = Faker()

def generate_group_chat_list(n=1, *, max_users):
    def generate_group_chat(chat_id):
        return {
            "chat_id": chat_id,
            "user_id": random.randint(1, max_users),
        }
    return [generate_group_chat(i) for i in range(n)]

def generate_message_list(n=1, *, max_chats, max_users):
    chat_id_list = random.sample(range(1, max_chats), min(max_chats // 2 + 1, n))
    values = []
    for id in chat_id_list:
        if len(values) >= n:
            break
        mu = random.randint(1, max_users // 2 + 1)
        users = random.sample(range(1, max_users+1), mu)
        for u in users:
            values.append((id, u))
    values = values[:n]

    return [{"chat_id": ci, "user_id": ui, "message": fake.paragraph(nb_sentences=2)} for ci, ui in values]

def main():
    user_count = int(input())
    chat_count = int(input())
    msg_count = int(input())

    print(f"Max groups: {chat_count}")
    print(f"Max chat messages: {msg_count}")

    for i in range(basemod.SHARD_COUNT):
        connection = basemod.MySQLConnection()

        connection.get(f"""CREATE TABLE IF NOT EXISTS `GroupChats` (
            `id` INT NOT NULL AUTO_INCREMENT,
            `chat_id` INT NOT NULL,
            `user_id` INT NOT NULL,
            PRIMARY KEY (`id`), KEY `uid` (`chat_id`)
        ); {basemod.get_hint(i)}""")

    for i in range(basemod.SHARD_COUNT):
        connection = basemod.MySQLConnection()

        connection.get(f"""CREATE TABLE IF NOT EXISTS `GCMessages` (
            `id` INT NOT NULL AUTO_INCREMENT,
            `chat_id` INT NOT NULL,
            `user_id` INT NOT NULL,
            `message` VARCHAR(4096) NOT NULL,
            PRIMARY KEY (`id`), KEY `cid` (`chat_id`)
        ); {basemod.get_hint(i)}""")

    connection = basemod.MySQLConnection()

    chats = generate_group_chat_list(chat_count, max_users=user_count)
    msgs = generate_message_list(msg_count, max_users=user_count, max_chats=chat_count)
    connection.insert_values(
            ("INSERT INTO `GroupChats` "
             "(`chat_id`, `user_id`) "
             "VALUES (%(chat_id)s, %(user_id)s)"), 
            chats, lambda x : basemod.get_hash(str(x["chat_id"]) + str(x["user_id"])))
    connection.insert_values(
            ("INSERT INTO `GCMessages` "
             "(`chat_id`, `user_id`, `message`) "
             "VALUES (%(chat_id)s, %(user_id)s, %(message)s)"), 
            msgs, lambda x : basemod.get_hash(str(x["chat_id"]) + str(x["user_id"])))

if __name__ == "__main__":
    main()
