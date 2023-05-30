import os
import mysql.connector
from mysql.connector import errorcode


SHARD_COUNT = 3

def get_hint(shard_id):
    shard_id = shard_id % SHARD_COUNT
    return f"-- sharding:{shard_id}"

def get_hash(astr):
    p = 53
    m = 61566613

    roll = 0
    for ch in astr[::-1]:
        roll = (roll * p) % m + ord(ch)
    return roll

class MySQLConnection:
    def __init__(self):
        self.cnx = mysql.connector.connect(
            host=os.environ["DB_HOST"],
            port=os.environ["DB_PORT"],
            database=os.environ["DB_DATABASE"],
            user=os.environ["DB_LOGIN"],
            password=os.environ["DB_PASSWORD"])
        self.cursor = self.cnx.cursor()
        self.cnx.autocommit = True

    def __del__(self):
        self.cursor.close()
        self.cnx.close()

    def get(self, command):
        try:
            self.cursor.execute(command)
        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_TABLE_EXISTS_ERROR:
                print("already exists.")
            else:
                print(err.msg)
        else:
            print("get: OK")

        return(list(self.cursor))

    def execute(self, command):
        try:
            self.cursor.execute(command)
        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_TABLE_EXISTS_ERROR:
                print("already exists.")
            else:
                print(err.msg)
        else:
            print("execute: OK")

        self.cnx.commit()

    def insert_values(self, insert_command, value_list, hash_func):
        for value in value_list:
            hint = get_hint(hash_func(value))
            self.cursor.execute(insert_command + hint, value)
        self.cnx.commit()
