#!/bin/bash

curl -4 --header "Content-Type: application/json" \
  --request POST \
  --data '{"username": "bob", "password": "asdf"}' \
  http://localhost:8888/auth

curl -4 --header "Content-Type: application/json" \
  --request POST \
  --data '{"price": 10.99}' \
  http://localhost:8888/item/lmdb

curl -4 http://localhost:8888/item/lmdb

curl -4 http://localhost:8888/items

#curl -v -4 --header "Content-Type: application/json" \
#  --request POST \
#  --data '{"name": "table", "price": 5.10}' \
#  http://localhost:8888/store/lmdb/item

