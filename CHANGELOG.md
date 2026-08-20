# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
where practical.

This fork continues [pasnox/qwbfsmanager](https://github.com/pasnox/qwbfsmanager)
(archived). Application version in tree: `1.5.1` (`qwbfs/qwbfs.pro`).

## [Unreleased]

### Added

### Changed

### Fixed

## [1.5.1] - 2026-08-20
### Changed
- fix: solve win release (#7)

## [1.5.0] - 2026-08-20
### Changed
- feat: win deploy (#6)

## [1.4.2] - 2026-08-20
### Changed
- Feature/windows changes (#5)

## [1.4.1] - 2026-08-20
### Added
- Automated amd64 `.deb` package build attached to GitHub Releases (`.github/scripts/build-deb.sh`)
- Menu bar (File / Help) on its own row above the toolbar; About/Version dialogs include maintainer info

### Changed

### Fixed

## [1.4.0] - 2026-08-12
### Added
- Automated amd64 `.deb` package build attached to GitHub Releases (`.github/scripts/build-deb.sh`)
- Menu bar (File / Help) on its own row above the toolbar; About/Version dialogs include maintainer info

### Changed

### Fixed

## [1.3.0] - 2026-08-12
### Added
- `LICENSE` file (GPL-2.0) so GitHub detects the project license; kept existing `GPL-2`
- Pull request template for change control (`.github/pull_request_template.md`)
- GitHub Action that creates a Release when a `v*` tag is pushed (`.github/workflows/release.yml`)
- Source archive attached to automated releases
- This changelog (`CHANGELOG.md`)
- Automatic version bump on merge to `main` (Conventional Commits → `qwbfs.pro` + changelog + tag + release)

### Changed
- README: downloads link, releases workflow docs, and license section for this fork

## [1.2.6] - fork baseline

### Notes
- Baseline of this repository based on the original QWBFS Manager sources
- Includes project packaging, Docker helpers, and dependency notes for local builds

## [1.2.5] - original

### Added
- Build with Qt4 and/or Qt5
- Italian translation update (kimotori)

## [1.2.0] - original

### Added
- Partitions combobox (name, filesystem, size)
- Game sorting by id, title, size, and region
- Enhanced ListView (list and icon modes)
- CoverFlow view
- Batch ISO ↔ WBFS conversion
- Batch renaming of ISO/WBFS files with customizable mask
- Drag-and-drop support for WBFS files
- New application icon and splash screen
- Spanish (`es_ES`) and Catalan (`ca_ES`) translations
- Italian (`it_IT`) translation

## [1.1.0] - original

### Added
- French translation (`fr_FR`)
- Update checker
- Passive error handling
- Translations manager
- Network cache with max retry per query

## [1.0.0] - original

### Added
- Initial release: WBFS drive management via libwbfs
- Game listing (titles, sizes, codes)
- Drag-and-drop ISO import
- Batch processing, rename, extract, and delete
- Direct and indirect drive-to-drive transfer
- Worker thread for IO operations

[Unreleased]: https://github.com/maniac787/qwbfsmanager/compare/v1.5.1...HEAD
[1.5.1]: https://github.com/maniac787/qwbfsmanager/releases/tag/v1.5.1
[1.5.0]: https://github.com/maniac787/qwbfsmanager/releases/tag/v1.5.0
[1.4.2]: https://github.com/maniac787/qwbfsmanager/releases/tag/v1.4.2
[1.4.1]: https://github.com/maniac787/qwbfsmanager/releases/tag/v1.4.1
[1.4.0]: https://github.com/maniac787/qwbfsmanager/releases/tag/v1.4.0
[1.3.0]: https://github.com/maniac787/qwbfsmanager/releases/tag/v1.3.0
[1.2.6]: https://github.com/maniac787/qwbfsmanager/releases/tag/v1.2.6
[1.2.5]: https://github.com/pasnox/qwbfsmanager
[1.2.0]: https://github.com/pasnox/qwbfsmanager
[1.1.0]: https://github.com/pasnox/qwbfsmanager
[1.0.0]: https://github.com/pasnox/qwbfsmanager
