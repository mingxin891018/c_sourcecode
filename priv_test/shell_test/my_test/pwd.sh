#!/bin/bash
echo ${BASH_SOURCE[0]}
export DIRNAME="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $DIRNAME
echo "==================="


echo $(dirname $0)

