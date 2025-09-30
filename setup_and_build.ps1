# Script để thiết lập môi trường Qt và build dự án
# Chạy script này với: .\setup_and_build.ps1

Write-Host "=== Thiết lập môi trường Qt 6.9.2 ===" -ForegroundColor Green

# Các đường dẫn có thể của Qt (thay đổi theo nơi bạn cài đặt)
$possibleQtPaths = @(
    "C:\Qt\6.9.2\msvc2022_64\bin",
    "C:\Qt\6.9.2\mingw_64\bin", 
    "C:\Qt\6.9.2\msvc2019_64\bin",
    "C:\Program Files\Qt\6.9.2\msvc2022_64\bin"
)

$possibleCMakePaths = @(
    "C:\Qt\Tools\CMake_64\bin",
    "C:\Program Files\CMake\bin",
    "C:\Program Files (x86)\CMake\bin"
)

# Tìm Qt
$qtFound = $false
foreach ($path in $possibleQtPaths) {
    if (Test-Path $path) {
        Write-Host "Tìm thấy Qt tại: $path" -ForegroundColor Yellow
        $env:PATH += ";$path"
        $qtFound = $true
        break
    }
}

# Tìm CMake
$cmakeFound = $false
foreach ($path in $possibleCMakePaths) {
    if (Test-Path $path) {
        Write-Host "Tìm thấy CMake tại: $path" -ForegroundColor Yellow
        $env:PATH += ";$path"
        $cmakeFound = $true
        break
    }
}

if (-not $qtFound) {
    Write-Host "Không tìm thấy Qt 6.9.2. Vui lòng cài đặt Qt từ https://www.qt.io/download-qt-installer" -ForegroundColor Red
    exit 1
}

if (-not $cmakeFound) {
    Write-Host "Không tìm thấy CMake. Vui lòng cài đặt CMake hoặc Qt Creator" -ForegroundColor Red
    exit 1
}

Write-Host "=== Build dự án ===" -ForegroundColor Green

# Tạo thư mục build nếu chưa có
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Name "build" | Out-Null
}

# Chuyển vào thư mục build
Set-Location "build"

# Cấu hình với CMake
Write-Host "Đang cấu hình với CMake..." -ForegroundColor Cyan
cmake .. -G "Visual Studio 17 2022" -A x64

if ($LASTEXITCODE -eq 0) {
    Write-Host "Cấu hình thành công!" -ForegroundColor Green
    
    # Build dự án
    Write-Host "Đang build dự án..." -ForegroundColor Cyan
    cmake --build . --config Release
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Build thành công!" -ForegroundColor Green
        Write-Host "=== Chạy ứng dụng ===" -ForegroundColor Green
        
        # Tìm file executable
        $exePath = Get-ChildItem -Path "." -Filter "*.exe" -Recurse | Where-Object { $_.Name -like "*TrafficPatrolEulerCPP*" } | Select-Object -First 1
        
        if ($exePath) {
            Write-Host "Đang chạy: $($exePath.FullName)" -ForegroundColor Cyan
            & $exePath.FullName
        } else {
            Write-Host "Không tìm thấy file executable. Kiểm tra thư mục Release hoặc Debug." -ForegroundColor Yellow
        }
    } else {
        Write-Host "Build thất bại!" -ForegroundColor Red
    }
} else {
    Write-Host "Cấu hình thất bại!" -ForegroundColor Red
}

# Quay lại thư mục gốc
Set-Location ".."
