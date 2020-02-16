#!/bin/bash

curl --header "Content-Type: application/json" \
  --request POST \
  --data '{"name":"lmdb"}' \
  http://localhost:8888/store

curl --header "Content-Type: application/json" \
  --request POST \
  --data '{"name": "table", "price": 5.10}' \
  http://localhost:8888/store/lmdb/item

