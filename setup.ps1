# setup.ps1 - 一键设置 uv 和 pre-commit 环境

Write-Host "开始设置 uv 和 pre-commit 环境..."

# 检查 uv 是否已安装
if (!(Get-Command uv -ErrorAction SilentlyContinue)) {
    Write-Host "安装 uv..."
    try {
        # 下载并安装 uv
        Invoke-WebRequest -Uri "https://github.com/astral-sh/uv/releases/latest/download/uv-installer.ps1" -OutFile "uv-installer.ps1"
        & ".\uv-installer.ps1"
        Remove-Item "uv-installer.ps1"
        Write-Host "uv 安装完成。"
    } catch {
        Write-Error "uv 安装失败: $_"
        exit 1
    }
} else {
    Write-Host "uv 已安装。"
}

# 刷新环境变量
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

# 使用 uv 安装 pre-commit
Write-Host "使用 uv 安装 pre-commit..."
try {
    & uv add pre-commit
    & uv sync
    Write-Host "pre-commit 安装完成。"
} catch {
    Write-Error "pre-commit 安装失败: $_"
    exit 1
}

# 激活虚拟环境
Write-Host "激活虚拟环境..."
try {
    & .\.venv\Scripts\Activate.ps1
    Write-Host "虚拟环境激活完成。"
} catch {
    Write-Error "虚拟环境激活失败: $_"
    exit 1
}

# 安装 pre-commit 钩子
Write-Host "安装 pre-commit 钩子..."
try {
    & pre-commit install
    Write-Host "pre-commit 钩子安装完成。"
} catch {
    Write-Error "pre-commit 钩子安装失败: $_"
    exit 1
}

Write-Host "安装完成"