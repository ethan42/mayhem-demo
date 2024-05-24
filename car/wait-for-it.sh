#!/usr/bin/env bash
set -e

host="$1"
shift
cmd="$@"

until nc -z "$host" 8000; do
  echo "Waiting for $host:8000..."
  sleep 1
done

echo "$host:8000 is up - executing command"
exec $cmd
