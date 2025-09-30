@echo off
REM setup.bat - 一键设置 uv 和 pre-commit 环境 (Windows批处理)
chcp 65001 >nul

echo 开始设置 uv 和 pre-commit 环境...

REM 检查 uv 是否已安装
uv --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 安装 uv...
    powershell -Command "try { Invoke-WebRequest -Uri 'https://github.com/astral-sh/uv/releases/latest/download/uv-installer.ps1' -OutFile 'uv-installer.ps1'; & '.\uv-installer.ps1'; Remove-Item 'uv-installer.ps1'; echo 'uv 安装完成。' } catch { Write-Error \"uv 安装失败: $_\"; exit 1 }"
    if %errorlevel% neq 0 exit /b 1
) else (
    echo uv 已安装。
)

REM 刷新环境变量
set PATH=%PATH%;%USERPROFILE%\.local\bin

REM 使用 uv 安装 pre-commit
echo 使用 uv 安装 pre-commit...
uv add -r requirements.txt
uv sync
if %errorlevel% neq 0 (
    echo pre-commit 安装失败
    exit /b 1
)
echo pre-commit 安装完成。

REM 激活虚拟环境
echo 激活虚拟环境...
call .venv\Scripts\activate.bat
if %errorlevel% neq 0 (
    echo 虚拟环境激活失败
    exit /b 1
)
echo 虚拟环境激活完成。

REM 安装 pre-commit 钩子
echo 安装 pre-commit 钩子...
pre-commit install
if %errorlevel% neq 0 (
    echo pre-commit 钩子安装失败
    exit /b 1
)
echo pre-commit 钩子安装完成。

echo 安装完成
pause