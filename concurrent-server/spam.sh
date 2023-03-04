#!/usr/bin/env bash

if [[ ($@ == "--help") || $@ == "-h" ]]; then
  echo "Usage: $0 STRING HOST PORT"
  echo "Send STRING to HOST on PORT every 0.5s, establish a new TCP connection for every transmission"
  exit 0
fi

while sleep 0.5; do
  echo $1 | nc -W 1 $2 $3
done
