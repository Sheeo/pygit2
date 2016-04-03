git clone --depth=1 -b maint/v0.24 https://github.com/libgit2/libgit2.git
cd libgit2/

mkdir build
cp "c:/projects/pygit2/appveyor/stdint.h" "c:/projects/pygit2/libgit2/include"
SET LIBGIT2=c:/projects/pygit2/libgit2
cmake . -DSTDCALL=OFF -DBUILD_CLAR=OFF -DCMAKE_INSTALL_PREFIX=%LIBGIT2% -G "Visual Studio 9 2008"
cmake --build . --target install
ls -la %LIBGIT2%/git2.dir
ls -la %LIBGIT2%/git2.dir/Debug
cat %LIBGIT2%/git2.dir/Debug/BuildLog.htm
pwd
ls -la %LIBGIT2%
ls -la %LIBGIT2%/lib
ls -la %LIBGIT2%/bin
cd ..
