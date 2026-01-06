#!/usr/bin/env bash
set -e
URL=${URL:-http://localhost:8080/api/search_v2?q=phone&limit=10&offset=0}
CONCURRENCY=${CONCURRENCY:-5}
REQUESTS=${REQUESTS:-50}
seq 1 $REQUESTS | xargs -n1 -P $CONCURRENCY -I{} curl -s "$URL" >/dev/null
