#!/usr/bin/env bash
# Install a user-local .desktop entry + icon so GNOME/Wayland shows the
# correct dock icon when running ./bin/qwbfsmanager.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${ROOT}/bin/qwbfsmanager"
ICON_SRC="${ROOT}/qwbfs/resources/qwbfsmanager.png"
DESKTOP_SRC="${ROOT}/packages/qwbfsmanager.desktop"

APP_DIR="${HOME}/.local/share/applications"
ICON_DIR_512="${HOME}/.local/share/icons/hicolor/512x512/apps"
ICON_DIR_256="${HOME}/.local/share/icons/hicolor/256x256/apps"
DESKTOP_DST="${APP_DIR}/qwbfsmanager.desktop"

if [[ ! -x "${BIN}" ]]; then
  echo "Binary not found: ${BIN}" >&2
  echo "Build first: qmake6 qwbfs.pro && make -j\"\$(nproc)\"" >&2
  exit 1
fi

if [[ ! -f "${ICON_SRC}" ]]; then
  echo "Icon not found: ${ICON_SRC}" >&2
  exit 1
fi

mkdir -p "${APP_DIR}" "${ICON_DIR_512}" "${ICON_DIR_256}"

install -m 644 "${ICON_SRC}" "${ICON_DIR_512}/qwbfsmanager.png"
install -m 644 "${ICON_SRC}" "${ICON_DIR_256}/qwbfsmanager.png"

# Point Exec to this build so launching from the menu and dock matches ./bin/qwbfsmanager
sed -e "s|^Exec=.*|Exec=${BIN}|" \
    -e "s|^Icon=.*|Icon=qwbfsmanager|" \
    "${DESKTOP_SRC}" > "${DESKTOP_DST}"
chmod 644 "${DESKTOP_DST}"

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${APP_DIR}" >/dev/null 2>&1 || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -f -t "${HOME}/.local/share/icons/hicolor" >/dev/null 2>&1 || true
fi

echo "Installed:"
echo "  ${DESKTOP_DST}"
echo "  ${ICON_DIR_512}/qwbfsmanager.png"
echo "Restart the app (./bin/qwbfsmanager) to refresh the dock icon."
