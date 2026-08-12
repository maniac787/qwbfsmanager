#!/usr/bin/env bash
# Bump PACKAGE_VERSION from Conventional Commits and update CHANGELOG.md.
# Exits 0 with SKIP=1 when no bump is needed.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

PRO_FILE="qwbfs/qwbfs.pro"
CHANGELOG_FILE="CHANGELOG.md"
REPO="${GITHUB_REPOSITORY:-maniac787/qwbfsmanager}"
DATE="$(date -u +%Y-%m-%d)"

write_skip() {
  local reason="$1"
  echo "Skipping: $reason"
  if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
    echo "skip=1" >>"$GITHUB_OUTPUT"
  fi
  if [[ -n "${GITHUB_ENV:-}" ]]; then
    echo "SKIP=1" >>"$GITHUB_ENV"
  fi
  exit 0
}

if [[ "${GITHUB_EVENT_NAME:-}" == "push" ]]; then
  head_msg="$(git log -1 --pretty=%B)"
  if [[ "$head_msg" == chore\(release\):* ]] || [[ "$head_msg" == *"[skip ci]"* ]]; then
    write_skip "HEAD is a release/skip-ci commit"
  fi
fi

current="$(grep -E '^PACKAGE_VERSION\s*=' "$PRO_FILE" | head -n1 | sed -E 's/^PACKAGE_VERSION[[:space:]]*=[[:space:]]*//;s/[[:space:]]*$//')"
if [[ ! "$current" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Invalid PACKAGE_VERSION: $current" >&2
  exit 1
fi

IFS=. read -r major minor patch <<<"$current"

last_tag="$(git tag -l 'v*' --sort=-v:refname | head -n1 || true)"
if [[ -n "$last_tag" ]]; then
  range="${last_tag}..HEAD"
else
  range="HEAD"
fi

commit_subjects="$(git log --format='%s' "$range" 2>/dev/null || true)"
if [[ -z "$commit_subjects" ]]; then
  write_skip "no commits since ${last_tag:-start}"
fi

commit_log="$(git log --format='%s%n%b' "$range")"
# Filter out release commits for bump decision
filtered_subjects="$(git log --format='%s' "$range" | grep -v -E '^chore\(release\):' || true)"
if [[ -z "$filtered_subjects" ]]; then
  write_skip "only release commits since ${last_tag:-start}"
fi

bump="patch"
if echo "$commit_log" | grep -qE '^BREAKING CHANGE:|[[:space:]]BREAKING CHANGE:'; then
  bump="major"
elif echo "$filtered_subjects" | grep -qE '^[a-zA-Z]+(\([^)]+\))?!:'; then
  bump="major"
elif echo "$filtered_subjects" | grep -qE '^feat(\([^)]+\))?:'; then
  bump="minor"
fi

case "$bump" in
  major)
    major=$((major + 1))
    minor=0
    patch=0
    ;;
  minor)
    minor=$((minor + 1))
    patch=0
    ;;
  patch)
    patch=$((patch + 1))
    ;;
esac

new_version="${major}.${minor}.${patch}"
echo "Bumping $current -> $new_version ($bump)"

# Update qwbfs.pro
sed -i -E "s/^PACKAGE_VERSION[[:space:]]*=[[:space:]]*.*/PACKAGE_VERSION = ${new_version}/" "$PRO_FILE"

# Extract Unreleased body (between ## [Unreleased] and next ## [)
unreleased_body="$(awk '
  /^## \[Unreleased\]/ {capture=1; next}
  /^## \[/ && capture {exit}
  capture {print}
' "$CHANGELOG_FILE")"

# Detect if Unreleased has real bullet entries
has_entries=0
if echo "$unreleased_body" | grep -qE '^[[:space:]]*-[[:space:]]+'; then
  has_entries=1
fi

if [[ "$has_entries" -eq 0 ]]; then
  release_notes="### Changed"$'\n'
  while IFS= read -r subject; do
    [[ -z "$subject" ]] && continue
    [[ "$subject" == chore\(release\):* ]] && continue
    release_notes+="- ${subject}"$'\n'
  done <<<"$filtered_subjects"
else
  # Trim leading/trailing blank lines
  release_notes="$(printf '%s\n' "$unreleased_body" | sed -e '/./,$!d' | sed -e :a -e '/^\n*$/{$d;N;ba' -e '}')"
  # Ensure trailing newline
  [[ "$release_notes" == *$'\n' ]] || release_notes+=$'\n'
fi

new_unreleased=$'## [Unreleased]\n\n### Added\n\n### Changed\n\n### Fixed\n'

tmp="$(mktemp)"
awk -v new_ver="$new_version" -v date="$DATE" -v notes="$release_notes" -v unreleased_block="$new_unreleased" -v repo="$REPO" '
  BEGIN {
    date_line = "## [" new_ver "] - " date
    released = 0
    skip_unreleased = 0
    in_footer = 0
  }
  {
    if ($0 ~ /^Application version in tree:/ || $0 ~ /\(archived\)\. Application version in tree:/) {
      sub(/`[0-9]+\.[0-9]+\.[0-9]+`/, "`" new_ver "`")
      print
      next
    }

    if ($0 ~ /^## \[Unreleased\]/) {
      print unreleased_block
      print date_line
      printf "%s", notes
      print ""
      skip_unreleased = 1
      released = 1
      next
    }

    if (skip_unreleased) {
      if ($0 ~ /^## \[/) {
        skip_unreleased = 0
      } else {
        next
      }
    }

    if ($0 ~ /^\[Unreleased\]:/) {
      print "[Unreleased]: https://github.com/" repo "/compare/v" new_ver "...HEAD"
      print "[" new_ver "]: https://github.com/" repo "/releases/tag/v" new_ver
      next
    }

    print
  }
' "$CHANGELOG_FILE" >"$tmp"

mv "$tmp" "$CHANGELOG_FILE"

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
  {
    echo "skip=0"
    echo "version=${new_version}"
    echo "tag=v${new_version}"
    echo "bump=${bump}"
  } >>"$GITHUB_OUTPUT"
fi

if [[ -n "${GITHUB_ENV:-}" ]]; then
  {
    echo "SKIP=0"
    echo "NEW_VERSION=${new_version}"
    echo "NEW_TAG=v${new_version}"
  } >>"$GITHUB_ENV"
fi

echo "Updated $PRO_FILE and $CHANGELOG_FILE to $new_version"
