# Changelog

## 0.1.0, 2026-09-05

First version. Runs on Apple Silicon and Intel, one zip per architecture: `mmb4m-0.1.0-silicon.zip` and `mmb4m-0.1.0-intel.zip`, each holding the single file `mmbasic`.

Built from MMBasic for Linux, branch `develop-v0.8-8`, commit `529fded`, 2026-07-30, an alpha of MMB4L 0.8. The port's version is its own and moves independently: the interpreter reports `MMBasic for macOS arm64 v0.1.0`, and this line is where the MMB4L state behind it is recorded.

What is different from MMBasic for Linux at that commit:

- builds and runs on macOS, Apple Silicon and Intel
- `PATH_MAX` comes from the macOS header, so paths stop at 1024 characters
- serial baud rates above 230400 are gone; macOS does not have them
- `EDIT` opens MMBasic's own editor by default. `OPTION EDITOR` still selects nano, VS Code or any other
- the port's copyright line in the startup banner, under the original three
- the banner, `--version` and `MM.INFO(VERSION)` report the port's own version and `macOS arm64` or `macOS x86_64`, not MMB4L's version and `Darwin`

Known issue: `OPTION SIMULATE` inside a running program faults at the next graphics command. Start with `-s` instead. See [known problems](docs/known-issues.md).
