#!/bin/sh

set -e

ver="$1"

sed -i s/"\(PLHW_VERSION \"\)\(.*\)\(\".*\)$"/"\1$ver\3"/ libplhw.h
git add libplhw.h

exit 0
