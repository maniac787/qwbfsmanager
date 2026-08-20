#!/usr/bin/env bash
# Build a portable Windows x64 zip of qwbfsmanager (Qt6 / MSYS2 MinGW64).
# Usage: build-windows.sh [version]
# Outputs: dist/qwbfsmanager-<version>-win64.zip
# Sets GITHUB_OUTPUT: zip_path, zip_name, version
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

PRO_FILE="qwbfs/qwbfs.pro"
DIST_DIR="${DIST_DIR:-$ROOT/dist}"

if [[ $# -ge 1 && -n "${1:-}" ]]; then
  VERSION="$1"
else
  VERSION="$(grep -E '^PACKAGE_VERSION\s*=' "$PRO_FILE" | head -n1 | sed -E 's/^PACKAGE_VERSION[[:space:]]*=[[:space:]]*//;s/[[:space:]]*$//')"
fi

# Strip leading v from tags if passed as v1.2.3
VERSION="${VERSION#v}"

if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+([.-].+)?$ ]]; then
  echo "Invalid version: ${VERSION}" >&2
  exit 1
fi

ZIP_NAME="qwbfsmanager-${VERSION}-win64.zip"
ZIP_PATH="${DIST_DIR}/${ZIP_NAME}"
DEPLOY_NAME="qwbfsmanager-${VERSION}-win64"
DEPLOY_DIR="${DIST_DIR}/${DEPLOY_NAME}"

export PATH="/mingw64/bin:/usr/bin:${PATH}"

echo "Building ${ZIP_NAME} ..."

command -v qmake6 >/dev/null
command -v mingw32-make >/dev/null
command -v windeployqt6 >/dev/null || command -v windeployqt >/dev/null

WINDEPLOYQT="$(command -v windeployqt6 || command -v windeployqt)"

make distclean >/dev/null 2>&1 || true
find . -name Makefile -delete 2>/dev/null || true

# Compile translations (.ts -> .qm); *.qm are gitignored
LRELEASE=""
for candidate in lrelease-qt6 lrelease6 lrelease; do
  if command -v "$candidate" >/dev/null 2>&1; then
    LRELEASE="$candidate"
    break
  fi
done
if [[ -z "$LRELEASE" ]]; then
  echo "lrelease not found; install mingw-w64-x86_64-qt6-tools" >&2
  exit 1
fi
find translations -name '*.ts' -print0 | xargs -0 -r "$LRELEASE"
if [[ -d fresh.git/translations ]]; then
  find fresh.git/translations -name '*.ts' -print0 | xargs -0 -r "$LRELEASE" || true
fi

qmake6 -r qwbfs.pro
mingw32-make -j"$(nproc)"

EXE="${ROOT}/bin/qwbfsmanager.exe"
if [[ ! -f "$EXE" ]]; then
  echo "Expected binary not found at ${EXE}" >&2
  find bin -type f >&2 || true
  exit 1
fi

rm -rf "$DEPLOY_DIR"
mkdir -p "$DIST_DIR" "$DEPLOY_DIR/translations"

cp "$EXE" "$DEPLOY_DIR/qwbfsmanager.exe"
cp -f translations/*.qm "$DEPLOY_DIR/translations/" 2>/dev/null || true
if [[ -d fresh.git/translations ]]; then
  cp -f fresh.git/translations/*.qm "$DEPLOY_DIR/translations/" 2>/dev/null || true
fi
if [[ -f LICENSE ]]; then
  cp LICENSE "$DEPLOY_DIR/"
elif [[ -f GPL-2 ]]; then
  cp GPL-2 "$DEPLOY_DIR/LICENSE"
fi

# Bundle Qt plugins/DLLs next to the exe
"$WINDEPLOYQT" --release --compiler-runtime "$DEPLOY_DIR/qwbfsmanager.exe"

# OpenSSL and MinGW runtimes (windeployqt does not always copy them)
for pattern in \
  libcrypto-*.dll \
  libssl-*.dll \
  libgcc_s_seh-1.dll \
  libstdc++-6.dll \
  libwinpthread-1.dll
do
  # shellcheck disable=SC2086
  cp -n /mingw64/bin/${pattern} "$DEPLOY_DIR/" 2>/dev/null || true
done

# Prefer Compress-Archive on native Windows CI when zip is missing
mkdir -p "$DIST_DIR"
rm -f "$ZIP_PATH"
(
  cd "$DIST_DIR"
  if command -v zip >/dev/null 2>&1; then
    zip -r -q "$ZIP_NAME" "$DEPLOY_NAME"
  elif command -v powershell.exe >/dev/null 2>&1; then
    powershell.exe -NoProfile -Command \
      "Compress-Archive -Path '$DEPLOY_NAME' -DestinationPath '$ZIP_NAME' -Force"
  else
    echo "Neither zip nor powershell.exe available to create the archive" >&2
    exit 1
  fi
)

echo "Built ${ZIP_PATH}"
ls -lh "$ZIP_PATH"
ls -la "$DEPLOY_DIR" | head -40

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  {
    echo "zip_path=${ZIP_PATH}"
    echo "zip_name=${ZIP_NAME}"
    echo "version=${VERSION}"
  } >>"$GITHUB_OUTPUT"
fi

if [[ -n "${GITHUB_ENV:-}" ]]; then
  {
    echo "WIN_ZIP_PATH=${ZIP_PATH}"
    echo "WIN_ZIP_NAME=${ZIP_NAME}"
  } >>"$GITHUB_ENV"
fi
