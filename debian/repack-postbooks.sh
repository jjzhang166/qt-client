#!/bin/bash

set -e

ORIG_TARBALL=$(readlink -f ${1})

if [ ! -e "${ORIG_TARBALL}" ];
then
  echo "${ORIG_TARBALL} does not exist"
  exit 1
fi

VERSION=`echo ${1} | cut -f2 -d-`

TMP_WS=`mktemp -d`

cd ${TMP_WS}

COMPRESSION_SCHEME=`echo "${1}" | tr '.' '\n' | tail -1`
case ${COMPRESSION_SCHEME} in
  gz)
    SYM=z
    ;;
  bz2)
    SYM=j
    ;;
  *)
    echo "Unable to recognise orig tarball compression scheme"
    exit 1
    ;;
esac

tar x${SYM}f "${ORIG_TARBALL}"

mv `ls` postbooks

find . -name '*DS_Store' -exec rm {} \;
find . -name '*.o' -exec rm {} \;
find . -name '*.a' -exec rm {} \;
find . -exec file {} \; | \
    grep -v script | grep executable | cut -f1 -d: | xargs -r rm


tar cjf /tmp/postbooks_${VERSION}.orig.tar.bz2 postbooks

