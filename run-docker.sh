#!/usr/bin/env sh

set -e

docker build \
  --tag gf/majestic \
  .

docker run \
  -it \
  --rm \
  -v ${PWD}/src/runtime/haskell:/tmp/gf-core/src/runtime/haskell \
  -v ${PWD}/src/runtime/python:/tmp/gf-core/src/runtime/python \
  -v ${PWD}/src/runtime/javascript:/tmp/gf-core/src/runtime/javascript \
  -v                               /tmp/gf-core/src/runtime/javascript/node_modules \
  gf/majestic \
  bash
