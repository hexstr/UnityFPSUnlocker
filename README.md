# UnityFPSUnlocker
[README_EN](https://github.com/hexstr/UnityFPSUnlocker/blob/zygisk_module/README_en.md)
[README_JP](https://github.com/hexstr/UnityFPSUnlocker/blob/zygisk_module/README_jp.md)

## 安装需求
- 设备已安装 [Magisk](https://github.com/topjohnwu/Magisk/releases) / [KernelSU](https://github.com/tiann/KernelSU/releases) / [APatch](https://github.com/bmax121/APatch/releases)
- 启用`Zygisk` (对于KernelSU / APatch用户 需要安装`Zygisk Next`)
- `Xposed`版本需要`LSPosed`
- `Xposed`模块和`Zygisk`模块二选一

## Xposed模块
一般`Xposed模块`不会随`Magisk模块`一起更新，所以请在[releases](https://github.com/hexstr/UnityFPSUnlocker/releases)中往前翻，寻找`app-debug.apk`。  
你可以提取最新的`zygisk_module.zip/zygisk/{对应架构}.so`替换掉`app-debug.apk/lib/{对应架构}/libUnityFPSUnlocker.so`，然后用mt管理器重新签名。

## 附加需求
- 对存在检测的游戏，比如`libtprt.so`，可以试试保持SELinux严格模式`setenforce 1`

## 其它
- 对存在检测的游戏，如果发生闪退，请不要提交issue

## 使用
若你的手机没有开启`Zygisk`，请先开启`Zygisk`后重启。当上述需求准备完毕后即可刷入`UnityFPSUnlocker`模块。在重启之前，先下载`TargetList.json`放入`/data/local/tmp/TargetList.json`，并修改你的配置。  
模块会通过判断游戏数据目录是否存在`/sdcard/Android/data/{包名}/files/il2cpp`来自动加载。

```
{
  "global": {
    "delay": 10,
    "mod_opcode": true,
    "fps": 90,
    "scale": 1.0
  },
  "custom": {
    "com.random.package.name.a": {
      "fps": 60
    },
    "com.random.package.name.b": {
      "mod_opcode": false
    },
    "com.random.package.name.c": {
      "delay": 5
    }
  }
}
```

其中，`global`节点中的配置为:

- `fps` 需要设置的`fps`，设置为`0`以禁用
- `delay` 游戏载入后等待`delay`秒执行
- `mod_opcode` 是否修改`opcode`，如果你发现游戏会重新锁定fps，可以把这项改为`true`，但由于修改内存，可能会被反作弊检测到
- `scale` 设置分辨率的倍数，一般保持`1.0`即可，必须为小数。`当前屏幕宽度 * scale x 当前屏幕高度 * scale`

然后，`custom`节点中的配置会覆盖`global`中的配置单独生效:

- `key` 包名，比如`com.random.package.name.a`
- `fps` 同上
- `mod_opcode` 同上
- `delay` 同上
- `scale` 同上

`TargetList.json`修改后可以搜索`json 格式校验`校验是否完整。修改后立即生效(`模块版本>=1.8`)。  
可以在终端模拟器输入`logcat -s UnityFPSUnlocker`查看输入日志。

## 黑名单
- 对于`Magisk模块`，设置对应的`fps`为`0`即可，插件将不会生效
- 对于`Xposed模块`，在`LSPosed`中取消勾选对应的应用即可，如果没有显示应用，可以点击右上角选择`隐藏->游戏`

## 分辨率
最近发现`BlueArchive`最高分辨率只有`1080P`，在模拟器上有肉眼可见的锯齿，而且在`16:10`的比例下甚至像素点比`16:9`更少

`2560x1600->1822x1138`

对比

`2560x1440->1920x1080`

所以加上调整分辨率的功能，对比图如下

https://imgsli.com/MjI3NDQ2/0/1

https://imgsli.com/MjI3NDQ2/2/3

如果你不需要超分辨率，可以修改`/sdcard/Android/data/com.nexon.bluearchive/files/DeviceOption`，把`Resolution`修改为大于`3`即可，这样会走`default`分支(在函数`GraphicsManager.CoSetScreenResolution()`中)

- `0`: 1080P
- `1`: 720P
- `2`: 540P
- `3`: 480P
- `default` default

