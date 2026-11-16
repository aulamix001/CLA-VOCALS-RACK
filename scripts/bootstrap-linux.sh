#!/usr/bin/env bash
# Local, unprivileged setup for the managed Debian cloud image.
set -euo pipefail
repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
tool_dir=/workspace/.tools
mkdir -p "$tool_dir/apt/lists/partial" "$tool_dir/apt/archives/partial" "$tool_dir/sysroot"
UV_CACHE_DIR="$tool_dir/uv-cache" uv pip install --target "$tool_dir/python" cmake==3.31.6 ninja==1.11.1.4
if [[ ! -d "$tool_dir/JUCE/.git" ]]; then
    git clone --depth 1 --branch 8.0.6 https://github.com/juce-framework/JUCE.git "$tool_dir/JUCE"
fi
[[ $(git -C "$tool_dir/JUCE" rev-parse HEAD) == 51a8a6d7aeae7326956d747737ccf1575e61e209 ]]
cat > "$tool_dir/apt/sources.list" <<'SOURCES'
deb https://deb.debian.org/debian trixie main
deb https://deb.debian.org/debian-security trixie-security main
SOURCES
apt_args=(-o "Dir::Etc::sourcelist=$tool_dir/apt/sources.list" -o Dir::Etc::sourceparts=- -o "Dir::State::lists=$tool_dir/apt/lists" -o "Dir::Cache::archives=$tool_dir/apt/archives" -o APT::Sandbox::User=agent -o Debug::NoLocking=true)
apt-get "${apt_args[@]}" update
apt-get "${apt_args[@]}" --download-only --no-install-recommends -y install libasound2-dev libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev
(cd "$tool_dir/apt/archives" && apt-get "${apt_args[@]}" download libfreetype6 libfontconfig1 libasound2t64 libx11-6 libxcb1 libxau6 libxdmcp6 libxext6 libxrender1 libxcursor1 libxfixes3 libxinerama1 libxrandr2 libpng16-16t64 libgl1 libglx0 libopengl0 libegl1)
for package in "$tool_dir"/apt/archives/*.deb; do dpkg-deb -x "$package" "$tool_dir/sysroot"; done
source "$repo_dir/scripts/cloud-env.sh"
cd "$repo_dir"
python scripts/verify_sources.py
cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM="$tool_dir/python/bin/ninja" -DFETCHCONTENT_SOURCE_DIR_JUCE="$tool_dir/JUCE" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$tool_dir/sysroot/usr"
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
