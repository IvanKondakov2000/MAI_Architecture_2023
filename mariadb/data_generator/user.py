import random
import string

from faker import Faker

import basemod


fake = Faker()

def generate_password(length, alphabet=[
            *string.ascii_lowercase, *string.ascii_uppercase,
            '-', '_', '.', '@', '&']
        ):
    return ''.join([random.choice(alphabet) for _ in range(length)])

def generate_profile_list(n=1):
    genders = {'M': 'male', 'F': 'female'}
    def generate_profile(i):
        faker_profile = fake.simple_profile()
        first_name, last_name = faker_profile['name'].split()[-2:]
        return {
            'first_name': first_name,
            'last_name': last_name,
            'email': faker_profile['mail'],
            'login': f'user{i}',
            'password': f'passwd{i}',
            'gender': genders[faker_profile['sex']]
        }

    return [generate_profile(i) for i in range(n)]

def main():
    connection = None

    for i in range(basemod.SHARD_COUNT):
        connection = basemod.MySQLConnection()

        connection.get(f"""CREATE TABLE IF NOT EXISTS `User` (
            `id`         INT NOT NULL AUTO_INCREMENT,
            `first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
            `last_name`  VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
            `login`      VARCHAR(256) NOT NULL,
            `password`   VARCHAR(256) NOT NULL,
            `email`      VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
            `gender`     VARCHAR(16) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
            PRIMARY KEY (`id`), KEY `fn` (`first_name`), KEY `ln` (`last_name`)
        ); {basemod.get_hint(i)}""")

    connection = basemod.MySQLConnection()

    values = generate_profile_list(200)
    connection.insert_values(("INSERT INTO `User` "
        "(`first_name`, `last_name`, `login`, `password`, `email`, `gender`) "
        "VALUES (%(first_name)s, %(last_name)s, %(login)s, %(password)s, %(email)s, %(gender)s)"), values, lambda x : basemod.get_hash(x['login'] + x['password']))

if __name__ == "__main__":
    main()

