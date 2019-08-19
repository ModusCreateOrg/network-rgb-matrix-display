#!/usr/bin/env bash
if [[ ! -f build/matrix-server ]]; then
	./mkbuild.sh
fi

build/matrix-server