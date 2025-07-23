v4: prophesee + 海康
v2: celex + 淘宝相机 

v1: celex + 淘宝相机      


连 网线   相机          （镜头）；


cd build 

./TwoCamera -c detect.json

# 双相机检测系统使用说明

## 基本使用方法

### 命令行参数

程序支持以下命令行参数：

```bash
# 使用默认配置文件 (dv_dvs.json)
./TwoCamera

# 指定自定义配置文件
./TwoCamera -c /path/to/your/config.json

# 启用调试模式
./TwoCamera -d

# 同时使用自定义配置文件和调试模式
./TwoCamera -c custom_config.json -d

# 查看帮助信息
./TwoCamera --help

# 查看版本信息
./TwoCamera --version
```

### 配置文件说明

- **默认配置文件**: `dv_dvs.json`
- **配置文件格式**: JSON格式
- **配置文件位置**: 可以是相对路径或绝对路径
- **配置文件内容**: 包含相机参数、录制设置、检测配置等

### 配置文件统一管理

现在所有配置文件读取都通过 `-c` 参数统一指定，包括：

1. **主配置**: 相机启动、检测功能等
2. **录制配置**: 录制路径、格式设置等  
3. **DVS配置**: DVS相机参数、累积时间等
4. **自动录制配置**: 批量录制参数等

### 示例用法

```bash
# 生产环境使用
./TwoCamera -c production_config.json

# 测试环境使用
./TwoCamera -c test_config.json

# 开发调试使用
./TwoCamera -c debug_config.json -d
```

## 配置文件迁移

如果您之前使用的是 `detect.json`，请将其内容合并到 `dv_dvs.json` 中，或者使用 `-c detect.json` 参数继续使用原配置文件。

## 注意事项

- 确保配置文件路径正确且文件存在
- 配置文件必须是有效的JSON格式
- 如果配置文件不存在，程序将使用默认配置并显示警告

## 编译

使用cmake编译

## 使用方法

TwoCamera 支持以下命令行参数：

- `-c <配置文件路径>`: 指定配置文件路径，默认为 "detect.json"
- `-d`: 开启调试模式

### 示例

```bash
# 使用默认配置文件
./TwoCamera

# 指定配置文件
./TwoCamera -c my_config.json

# 开启调试模式
./TwoCamera -d

# 同时指定配置文件并开启调试模式
./TwoCamera -c my_config.json -d
```

## 配置文件格式

配置文件采用 JSON 格式，包含所有应用程序设置，其中 DV 显示相关的配置示例如下：

```json
{
  "controls": {
    "dv": true,
    "dvs_view": true,
    "dv_view": true
  },
  "fontPointSize": 24,
  "fpn": "FPN_2.txt",
  "dvDisplay": {
    "mode": "original",  // 显示模式: "original" 或 "cropped"
    "crop": {
      "enabled": true,   // 是否启用裁剪
      "x": 100,          // 裁剪起始x坐标
      "y": 50,           // 裁剪起始y坐标
      "width": 640,      // 裁剪宽度
      "height": 480      // 裁剪高度
    },
    "save": {
      "enabled": true,   // 是否保存裁剪后的图片
      "path": "saved_crops"  // 保存裁剪图片的路径
    }
  },
  "saveOptions": {}
}
```
