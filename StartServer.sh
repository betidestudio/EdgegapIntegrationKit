#!/bin/sh

GAME_PORT=$(echo $ARBITRIUM_PORTS_MAPPING | jq '.ports.gameport.internal')

printenv
env

$(dirname "$0")/<PROJECT_NAME>Server.sh -log -PORT=$GAME_PORT 