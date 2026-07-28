import subprocess
import os

# 1. SysConfig 工具路径（GUI 版本）
SYSCFG_PATH = r"D:\ti\sysconfig_1.27.1\sysconfig_gui.bat"

# 2. 项目根目录（脚本位于 eide/ 下，向上一级即为项目根目录）
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# 3. SDK 产品信息（位于项目根目录的 .metadata 中）
PRODUCT_JSON = os.path.join(PROJECT_ROOT, ".metadata", "product.json")

# 4. syscfg 配置文件位于 user/ 目录下
CONFIG_FILE = os.path.join(PROJECT_ROOT, "user", "empty.syscfg")

# 5. 执行命令（打开 GUI）
if os.path.exists(CONFIG_FILE):
    if os.path.exists(PRODUCT_JSON):
        print(f"正在打开 SysConfig GUI: {CONFIG_FILE}")
        cmd = [SYSCFG_PATH, "-s", PRODUCT_JSON, CONFIG_FILE]
        subprocess.Popen(cmd)
    else:
        print(f"警告: 找不到产品配置文件 {PRODUCT_JSON}")
        print("将直接打开 SysConfig GUI（可能缺少 SDK 模块支持）")
        cmd = [SYSCFG_PATH, CONFIG_FILE]
        subprocess.Popen(cmd)
else:
    print(f"错误: 找不到配置文件 {CONFIG_FILE}")