#!/bin/bash
# setup.sh - 一键设置 uv 和 pre-commit 环境 (Shell脚本)

echo "开始设置 uv 和 pre-commit 环境..."

# 检查 uv 是否已安装
if ! command -v uv &> /dev/null; then
    echo "安装 uv..."
    if curl -LsSf https://astral.sh/uv/install.sh | sh; then
        echo "uv 安装完成。"
    else
        echo "uv 安装失败"
        echo "运行失败，考虑配置网络代理"
        exit 1
    fi
else
    echo "uv 已安装。"
fi

# 刷新环境变量
export PATH="$HOME/.local/bin:$PATH"

# 使用 uv 安装 pre-commit
echo "使用 uv 安装 pre-commit..."
if uv add -r requirements.txt;uv sync; then
    echo "pre-commit 安装完成。"
else
    echo "pre-commit 安装失败"
    echo "运行失败，考虑配置网络代理"
    exit 1
fi

# 激活虚拟环境
echo "激活虚拟环境..."
if source .venv/bin/activate; then
    echo "虚拟环境激活完成。"
else
    echo "虚拟环境激活失败"
    echo "运行失败，考虑配置网络代理"
    exit 1
fi

# 安装 pre-commit 钩子
echo "安装 pre-commit 钩子..."
if pre-commit install; then
    echo "pre-commit 钩子安装完成。"
else
    echo "pre-commit 钩子安装失败"
    echo "运行失败，考虑配置网络代理"
    exit 1
fi

echo "安装完成"