#!/bin/sh

VERSION=$1

if [ -z "$VERSION" ]; then
  echo "usage: ./archive-win32.sh <version>"
  exit
fi

OUTPUT="__dist/autom8_win32_$VERSION"

rm -rf "$OUTPUT"

#mkdir -p "$OUTPUT/plugins"
mkdir -p "$OUTPUT/themes"
mkdir -p "$OUTPUT/locales"
cp __output32/Release/autom8-curses.exe "$OUTPUT"
cp __output32/Release/*.dll "$OUTPUT"
#cp __output32/Release/plugins/*.dll "$OUTPUT/plugins"
cp __output32/Release/themes/*.json "$OUTPUT/themes"
cp __output32/Release/locales/*.json "$OUTPUT/locales"
pushd $OUTPUT
7z a -tzip "autom8_win32_$VERSION.zip" ./* -mx=9
mv "autom8_win32_$VERSION.zip" ..
popd

rm -rf "$OUTPUT"
