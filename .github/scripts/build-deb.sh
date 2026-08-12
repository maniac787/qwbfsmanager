#!/usr/bin/env bash
# Build an amd64 .deb of qwbfsmanager (Qt6 / qmake6).
# Usage: build-deb.sh [version]
# Outputs: dist/qwbfsmanager_<version>_amd64.deb
# Sets GITHUB_OUTPUT / GITHUB_ENV: deb_path, deb_name, version
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

PRO_FILE="qwbfs/qwbfs.pro"
DIST_DIR="${DIST_DIR:-$ROOT/dist}"
STAGING="${STAGING:-$ROOT/.deb-staging}"
BUILD_DIR="${BUILD_DIR:-$ROOT/.deb-build}"

if [[ $# -ge 1 && -n "${1:-}" ]]; then
  VERSION="$1"
else
  VERSION="$(grep -E '^PACKAGE_VERSION\s*=' "$PRO_FILE" | head -n1 | sed -E 's/^PACKAGE_VERSION[[:space:]]*=[[:space:]]*//;s/[[:space:]]*$//')"
fi

if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+([.-].+)?$ ]]; then
  echo "Invalid version: ${VERSION}" >&2
  exit 1
fi

# Strip leading v from tags if passed as v1.2.3
VERSION="${VERSION#v}"

ARCH="$(dpkg --print-architecture)"
PKG_NAME="qwbfsmanager"
DEB_NAME="${PKG_NAME}_${VERSION}_${ARCH}.deb"
DEB_PATH="${DIST_DIR}/${DEB_NAME}"

echo "Building ${DEB_NAME} ..."

rm -rf "$STAGING" "$BUILD_DIR"
mkdir -p "$STAGING" "$DIST_DIR" "$BUILD_DIR"

# Out-of-source-ish clean configure with PREFIX=/usr
make distclean >/dev/null 2>&1 || true
find . -name Makefile -delete 2>/dev/null || true

# Compile translations (.ts -> .qm); *.qm are gitignored
LRELEASE=""
for candidate in lrelease6 lrelease /usr/lib/qt6/bin/lrelease /usr/lib/qt5/bin/lrelease; do
  if command -v "$candidate" >/dev/null 2>&1 || [[ -x "$candidate" ]]; then
    LRELEASE="$candidate"
    break
  fi
done
if [[ -n "$LRELEASE" ]]; then
  find translations -name '*.ts' -print0 | xargs -0 -r "$LRELEASE"
elif compgen -G "translations/*.qm" >/dev/null; then
  echo "Warning: lrelease not found; reusing existing .qm files"
else
  echo "lrelease not found and no .qm files present; install qt6-l10n-tools" >&2
  exit 1
fi

qmake6 -r qwbfs.pro PREFIX=/usr
make -j"$(nproc)"
make install INSTALL_ROOT="$STAGING"

BIN_PATH="${STAGING}/usr/bin/qwbfsmanager"
if [[ ! -x "$BIN_PATH" ]]; then
  echo "Expected binary not found at ${BIN_PATH}" >&2
  find "$STAGING" -type f >&2 || true
  exit 1
fi

strip --strip-unneeded "$BIN_PATH" || true

mkdir -p "${STAGING}/DEBIAN" "${STAGING}/usr/share/doc/${PKG_NAME}"

# Runtime Depends via dpkg-shlibdeps when available
DEPENDS="libc6, libudev1"
if command -v dpkg-shlibdeps >/dev/null 2>&1; then
  mkdir -p "${STAGING}/debian"
  cat >"${STAGING}/debian/control" <<EOF
Source: ${PKG_NAME}
Package: ${PKG_NAME}
Depends: \${shlibs:Depends}
EOF
  (
    cd "$STAGING"
    dpkg-shlibdeps -Tdebian/substvars ./usr/bin/qwbfsmanager
  )
  if [[ -f "${STAGING}/debian/substvars" ]]; then
    SHLIBS="$(sed -n 's/^shlibs:Depends=//p' "${STAGING}/debian/substvars" | head -n1)"
    if [[ -n "$SHLIBS" ]]; then
      DEPENDS="$SHLIBS"
    fi
  fi
  rm -rf "${STAGING}/debian"
else
  # Fallback for environments without dpkg-dev helpers
  DEPENDS="libc6, libqt6core6 | libqt6core6t64, libqt6gui6 | libqt6gui6t64, libqt6widgets6 | libqt6widgets6t64, libqt6network6 | libqt6network6t64, libqt6xml6 | libqt6xml6t64, libqt6dbus6 | libqt6dbus6t64, libssl3 | libssl3t64, libudev1"
fi

INSTALLED_SIZE="$(du -sk "${STAGING}/usr" | awk '{print $1}')"

cat >"${STAGING}/DEBIAN/control" <<EOF
Package: ${PKG_NAME}
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: QWBFS Manager contributors <https://github.com/${GITHUB_REPOSITORY:-maniac787/qwbfsmanager}>
Installed-Size: ${INSTALLED_SIZE}
Depends: ${DEPENDS}
Homepage: https://github.com/${GITHUB_REPOSITORY:-maniac787/qwbfsmanager}
Description: Cross-platform Wii Backup File System manager
 QWBFS Manager is a Qt GUI for working with hard disk drives
 formatted with the WBFS file system used by Nintendo Wii backups.
EOF

cp LICENSE "${STAGING}/usr/share/doc/${PKG_NAME}/copyright" 2>/dev/null \
  || cp GPL-2 "${STAGING}/usr/share/doc/${PKG_NAME}/copyright"

# Compress changelog if present
if [[ -f CHANGELOG.md ]]; then
  cp CHANGELOG.md "${STAGING}/usr/share/doc/${PKG_NAME}/changelog"
  gzip -9 -f "${STAGING}/usr/share/doc/${PKG_NAME}/changelog"
fi

# Permissions expected by dpkg
chmod 0755 "${STAGING}/DEBIAN"
chmod 0644 "${STAGING}/DEBIAN/control"
find "${STAGING}/usr" -type d -exec chmod 0755 {} +
find "${STAGING}/usr" -type f -exec chmod 0644 {} +
chmod 0755 "$BIN_PATH"

dpkg-deb --root-owner-group --build "$STAGING" "$DEB_PATH"

echo "Built ${DEB_PATH}"
dpkg-deb --info "$DEB_PATH" || true
dpkg-deb --contents "$DEB_PATH" | head -40 || true

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  {
    echo "deb_path=${DEB_PATH}"
    echo "deb_name=${DEB_NAME}"
    echo "version=${VERSION}"
  } >>"$GITHUB_OUTPUT"
fi

if [[ -n "${GITHUB_ENV:-}" ]]; then
  {
    echo "DEB_PATH=${DEB_PATH}"
    echo "DEB_NAME=${DEB_NAME}"
  } >>"$GITHUB_ENV"
fi
