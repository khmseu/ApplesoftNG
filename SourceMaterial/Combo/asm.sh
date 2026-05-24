#!/bin/bash

cd "$(dirname "$0")" || exit

/usr/bin/xa -C -XMASM -o asrom.bin -P asrom.lst -e asrom.err -l asrom.sym -r asrom.s
