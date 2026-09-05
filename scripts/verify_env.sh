#!/bin/sh
# scripts/verify_env.sh — sanity checks for the CI build environment.
# Verifies the toolchain works, reports platform info, exit 0 on success.

set -e

echo "--- toolchain ---"
cc --version | head -n1
make --version | head -n1

echo "--- platform ---"
uname -m
nproc

echo "--- thread sanity (pthread smoke) ---"
make -s corebench
./corebench --threads 2 --iterations 2000 --quiet
echo "environment OK"
