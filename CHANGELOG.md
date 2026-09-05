# Changelog

## 0.1.0, 2026-09-05

First version. Runs on Apple Silicon and Intel, one zip per architecture: `mmb4m-0.1.0-silicon.zip` and `mmb4m-0.1.0-intel.zip`, each holding the single file `mmbasic`.

Built from MMBasic for Linux, branch `develop-v0.8-8`, commit `529fded`, 2026-07-30. That is an alpha of MMB4L 0.8; the interpreter reports itself as `v0.8-alpha.1`.

What is different from MMBasic for Linux at that commit:

- builds and runs on macOS, Apple Silicon and Intel
- `PATH_MAX` comes from the macOS header, so paths stop at 1024 characters
- serial baud rates above 230400 are gone; macOS does not have them
- `EDIT` opens MMBasic's own editor by default. `OPTION EDITOR` still selects nano, VS Code or any other
- the port's copyright line in the startup banner, under the original three

Known issue: `OPTION SIMULATE` inside a running program faults at the next graphics command. Start with `-s` instead. See [known problems](docs/known-issues.md).
