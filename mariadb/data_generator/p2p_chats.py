import random
import string

from faker import Faker

import basemod


fake = Faker()

def generate_message_list(n=1, *, max_users):
    from_list = random.sample(range(1, max_users+1), min(max_users // 2 + 1, n))
    values = []
    for id in from_list:
        if len(values) >= n:
            break
        mu = random.randint(1, max_users // 2 + 1)
        users = random.sample(range(1, max_users+1), mu)
        for u in users:
            values.append((id, u))
    values = values[:n]

    return [{"from_id": ci, "to_id": ui, "message": fake.paragraph(nb_sentences=2)} for ci, ui in values]

def main():
    user_count = int(input())
    msg_count = int(input())

    print(f"Max peer chat messages: {msg_count}")

    for i in range(basemod.SHARD_COUNT):
        connection = basemod.MySQLConnection()

        connection.get(f"""CREATE TABLE IF NOT EXISTS `PToPMessages` (
            `id` INT NOT NULL AUTO_INCREMENT,
            `from_id` INT NOT NULL,
            `to_id` INT NOT NULL,
            `message` VARCHAR(4096) NOT NULL,
            PRIMARY KEY (`id`)
        ); {basemod.get_hint(i)}""")

    connection = basemod.MySQLConnection()

    msgs = generate_message_list(msg_count, max_users=user_count)
    connection.insert_values(
            ("INSERT INTO `PToPMessages` "
             "(`from_id`, `to_id`, `message`) "
             "VALUES (%(from_id)s, %(to_id)s, %(message)s)"),
            msgs, lambda x : basemod.get_hash(str(x["from_id"]) + str(x["to_id"])))

if __name__ == "__main__":
    main()
