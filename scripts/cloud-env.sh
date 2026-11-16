# Source this file in the managed Linux workspace before CMake/build/test.
export PATH="/workspace/.tools/python/cmake/data/bin:/workspace/.tools/python/bin:$PATH"
export PKG_CONFIG_PATH="/workspace/.tools/sysroot/usr/lib/x86_64-linux-gnu/pkgconfig:/workspace/.tools/sysroot/usr/share/pkgconfig"
export PKG_CONFIG_SYSROOT_DIR=/workspace/.tools/sysroot
export XDG_CACHE_HOME=/workspace/.tools/cache
export CPLUS_INCLUDE_PATH="/workspace/.tools/sysroot/usr/include:/workspace/.tools/sysroot/usr/include/freetype2${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
export C_INCLUDE_PATH="/workspace/.tools/sysroot/usr/include:/workspace/.tools/sysroot/usr/include/freetype2${C_INCLUDE_PATH:+:$C_INCLUDE_PATH}"
export LIBRARY_PATH="/workspace/.tools/sysroot/usr/lib/x86_64-linux-gnu${LIBRARY_PATH:+:$LIBRARY_PATH}"
export LD_LIBRARY_PATH="/workspace/.tools/sysroot/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
