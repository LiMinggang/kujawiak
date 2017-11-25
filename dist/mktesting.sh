#!/bin/bash

export TARGET="kuView"
if [ -d $TARGET ]; then
  rm -frd $TARGET
fi
mkdir $TARGET

cp ../doc/*.txt $TARGET/
cp ../build/linux/kuview.bin $TARGET/
cp ../lang/*.mo $TARGET/
touch $TARGET/_testing_version_

export STAMP=`date +%Y%m%d`
export FILENAME=$TARGET-$STAMP.tgz
if [ -f $FILENAME ]; then
  rm $FILENAME
fi
tar zcvf  $FILENAME $TARGET
rm -frd $TARGET

