@echo off
cd /d "D:\university\HKI_25_26\trr\project_cuoi_ki"

:: Xóa thư mục build cũ
rmdir /s /q release
rmdir /s /q debug

:: Tạo Makefile mới
C:\Qt\6.9.2\mingw_64\bin\qmake.exe -o Makefile project.pro -spec win32-g++

:: Build lại dự án
mingw32-make clean
mingw32-make -j8

:: Chạy chương trình nếu có file exe
if exist release\project.exe (
    cd release
    project.exe
    cd ..
) else (
    echo Build failed, project.exe not found.
)
