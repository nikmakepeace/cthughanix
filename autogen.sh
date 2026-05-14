#!/bin/sh
# Check for a working autoreconf
if ! type autoreconf >/dev/null 2>&1; then
  echo "Error: autoreconf not found. Please install autoconf and automake."
  exit 1
fi

# Run the bootstrap
autoreconf -vfi