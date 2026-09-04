#!/usr/bin/env bash

set -euo pipefail

unset CDPATH
repo_root="$(cd -- "$(dirname -- "$0")/.." && pwd)"
object="${OBJECT:-$repo_root/plugins/Witchboard-v2.o}"
release_dir="${RELEASE_DIR:-$repo_root/release}"

case "$object" in
    /*) ;;
    *) object="$repo_root/$object" ;;
esac

case "$release_dir" in
    /*) ;;
    *) release_dir="$repo_root/$release_dir" ;;
esac

staging_dir="$release_dir/staging/witchboard"

test -f "$object"

rm -rf -- "$release_dir"
mkdir -p "$staging_dir/programs/plug-ins"

cp "$object" "$staging_dir/programs/plug-ins/Witchboard-v2.o"
printf '%s\n' \
    "Copy programs/plug-ins/Witchboard-v2.o to the same path on the disting NT MicroSD card." \
    "The plug-in appears as 'Witchboard v2' and uses GUID WtC2." \
    "Requires a disting NT firmware version compatible with C++ plugin API v13." \
    >"$staging_dir/INSTALL.txt"

(
    cd "$staging_dir"
    zip -q -r "$release_dir/Witchboard-v2.zip" programs INSTALL.txt
)

cp "$object" "$release_dir/Witchboard-v2.o"
rm -rf -- "$release_dir/staging"

unzip -l "$release_dir/Witchboard-v2.zip"
