#!/bin/bash

g++ -I. -fpermissive MiningPool/*.cpp -lcurl -lsqlite3 -lpthread -o dynapool

exit