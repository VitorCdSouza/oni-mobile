#!/usr/bin/env bash
# Install build/oni-mobile.apk on the phone through its own installer (adb install
# fails with INSTALL_FAILED_USER_RESTRICTED on HyperOS): push it to Download, index it
# in the media store and open it with the package installer. Then tap Update on the
# phone within ~10 s.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
APK=${1:-$ROOT/build/oni-mobile.apk}
DEST=/sdcard/Download/oni-mobile.apk

adb push "$APK" "$DEST"
adb shell content call --uri content://media/external/file --method scan_file --arg "$DEST" > /dev/null
id=$(adb shell content query --uri content://media/external/file --projection _id:_data \
    | grep "/Download/oni-mobile.apk" | sed -E 's/.*_id=([0-9]+),.*/\1/' | head -1)
if [ -z "$id" ]; then
    echo "the APK is not in the media store yet; run again" >&2
    exit 1
fi
adb shell am start -a android.intent.action.VIEW -t application/vnd.android.package-archive \
    -d "content://media/external/file/$id" --grant-read-uri-permission > /dev/null
echo "installer open on the phone: tap Update"
