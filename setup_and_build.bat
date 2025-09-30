@echo off
echo === Thiết lập môi trường Qt 6.9.2 ===

REM Thêm các đường dẫn có thể của Qt vào PATH
set "PATH=%PATH%;C:\Qt\6.9.2\msvc2022_64\bin"
set "PATH=%PATH%;C:\Qt\6.9.2\mingw_64\bin"
set "PATH=%PATH%;C:\Qt\6.9.2\msvc2019_64\bin"
set "PATH=%PATH%;C:\Program Files\Qt\6.9.2\msvc2022_64\bin"
set "PATH=%PATH%;C:\Qt\Tools\CMake_64\bin"
set "PATH=%PATH%;C:\Program Files\CMake\bin"
set "PATH=%PATH%;C:\Program Files (x86)\CMake\bin"

echo === Build dự án ===

REM Tạo thư mục build nếu chưa có
if not exist "build" mkdir build

REM Chuyển vào thư mục build
cd build

REM Cấu hình với CMake
echo Đang cấu hình với CMake...
cmake .. -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% EQU 0 (
    echo Cấu hình thành công!
    
    REM Build dự án
    echo Đang build dự án...
    cmake --build . --config Release
    
    if %ERRORLEVEL% EQU 0 (
        echo Build thành công!
        echo === Chạy ứng dụng ===
        
        REM Tìm và chạy file executable
        for /r . %%i in (*TrafficPatrolEulerCPP*.exe) do (
            echo Đang chạy: %%i
            start "" "%%i"
            goto :end
        )
        
        echo Không tìm thấy file executable. Kiểm tra thư mục Release hoặc Debug.
    ) else (
        echo Build thất bại!
    )
) else (
    echo Cấu hình thất bại!
)

:end
REM Quay lại thư mục gốc
cd ..
pause
