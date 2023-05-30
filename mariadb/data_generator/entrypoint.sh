#!/bin/sh

USER_COUNT=2000
GCHAT_COUNT=50
GMSG_COUNT=150
P2PMSG_COUNT=400

DATABASE_HOST='mai-db'
DATABASE_NAME='archdb'
DATABASE_USER='stud'
DATABASE_PASSWORD='stud'

echo "$USER_COUNT" | python3 user.py
echo "$USER_COUNT\n$GCHAT_COUNT\n$GMSG_COUNT" | python3 group_chats.py
echo "$USER_COUNT\n$P2PMSG_COUNT" | python3 p2p_chats.py
