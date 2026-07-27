#!/bin/bash -e
set +x

# Builds the CouchDoom standalone app for the host platform and packages it into
# ci/bin/<App>_<OS>.zip. On macOS it also code-signs and notarizes when the
# signing secrets are present (skipped locally). Called by the GitHub workflows.

APP="CouchDoom"

# ---- macOS signing setup (CI only, and only when secrets are provided) ----
if [ "$(uname)" == "Darwin" ]; then
  if [ -n "$GITHUB_ACTIONS" ] && [ -n "$APPLICATION" ]; then
    echo "Create a keychain"
    security create-keychain -p nr4aGPyz Keys.keychain

    echo "$APPLICATION" | base64 -D -o /tmp/Application.p12
    echo "$INSTALLER"   | base64 -D -o /tmp/Installer.p12

    security import /tmp/Application.p12 -t agg -k Keys.keychain -P aym9PKWB -A -T /usr/bin/codesign
    security import /tmp/Installer.p12   -t agg -k Keys.keychain -P aym9PKWB -A -T /usr/bin/codesign

    security list-keychains -s Keys.keychain
    security default-keychain -s Keys.keychain
    security unlock-keychain -p nr4aGPyz Keys.keychain
    security set-keychain-settings -l -u -t 13600 Keys.keychain
    security set-key-partition-list -S apple-tool:,apple: -s -k nr4aGPyz Keys.keychain
  fi
  DEV_APP_ID="Developer ID Application: Roland Rabien (3FS7DJDG38)"
fi

ROOT=$(cd "$(dirname "$0")/.."; pwd)
cd "$ROOT"
echo "$ROOT"

rm -Rf "$ROOT/ci/bin"
mkdir -p "$ROOT/ci/bin"

# ---- macOS ----
if [ "$(uname)" == "Darwin" ]; then
  cmake --preset xcode
  cmake --build --preset xcode --config Release --target "$APP"

  cp -R "$ROOT/Builds/xcode/${APP}_artefacts/Release/$APP.app" "$ROOT/ci/bin/"

  cd "$ROOT/ci/bin"
  if [ -n "$APPLICATION" ]; then
    codesign -s "$DEV_APP_ID" --deep -v "$APP.app" --options=runtime --timestamp --force
  else
    echo "Not signing"
  fi

  if [ -n "$APPLE_USER" ]; then
    ditto -c -k --keepParent "$APP.app" "${APP}_Mac.zip"
    xcrun notarytool submit --verbose --apple-id "$APPLE_USER" --password "$APPLE_PASS" \
          --team-id "3FS7DJDG38" --wait --timeout 30m "${APP}_Mac.zip"
    rm "${APP}_Mac.zip"
    xcrun stapler staple "$APP.app"
  else
    echo "Not notarizing"
  fi

  ditto -c -k --keepParent "$APP.app" "${APP}_Mac.zip"

# ---- Linux ----
elif [ "$(expr substr "$(uname -s)" 1 5)" == "Linux" ]; then
  cmake --preset ninja-gcc
  cmake --build --preset ninja-gcc --config Release --target "$APP"

  cp "$ROOT/Builds/ninja-gcc/${APP}_artefacts/Release/$APP" "$ROOT/ci/bin/"

  cd "$ROOT/ci/bin"
  strip "$APP" || true
  zip -r "${APP}_Linux.zip" "$APP"

# ---- Windows (MSYS/MinGW bash) ----
elif [ "$(expr substr "$(uname -s)" 1 10)" == "MINGW64_NT" ]; then
  cmake --preset vs
  cmake --build --preset vs --config Release --target "$APP"

  cp "$ROOT/Builds/vs/${APP}_artefacts/Release/$APP.exe" "$ROOT/ci/bin/"

  cd "$ROOT/ci/bin"
  7z a "${APP}_Win.zip" "$APP.exe"
fi
