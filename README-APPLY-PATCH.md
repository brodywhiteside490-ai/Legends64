# Custom Shop Maker — Android build patch

This patch is for the existing `brodywhiteside490-ai/Legends64` repository.

## Included
- `CMakeLists.txt`: fixes source paths because `main.cpp`, `ShopData.cpp`, and their header currently live in the repository root.
- `.github/workflows/build-android64.yml`: adds an Android arm64 Geode build and uploads the produced `.geode` file as a GitHub Actions artifact.

## Apply it
1. Download and extract this ZIP.
2. In your GitHub repository, replace `CMakeLists.txt` with the included one.
3. Upload `.github/workflows/build-android64.yml` at that exact path.
4. Open the repository's **Actions** tab and run **Build Custom Shop Maker (Android 64-bit)**, or push a commit to `main`.
5. If the build succeeds, open the completed workflow run and download the `Custom-Shop-Maker-Android64` artifact.

## Important
This is a build-setup patch, not a compiled `.geode` file. The existing source still has known feature gaps: More Icons unlock integration and Scratch-room injection are not implemented, keeper part indices are not rendered as a composite character, and the code has not yet been compiled or runtime-tested against your exact Geometry Dash/Geode installation. If the build fails, use the workflow log to identify the first compiler error before treating the mod as installable.
