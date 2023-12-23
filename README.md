# UnityFPSUnlocker
[README_JP](https://github.com/hexstr/UnityFPSUnlocker/blob/zygisk_module/README_jp.md)

## 安装需求
- [Magisk](https://github.com/topjohnwu/Magisk/releases)
- 启用`zygisk`

## 使用
下载并刷入模块。在重启之前，先下载`TargetList.json`放入`/data/local/tmp/TargetList.json`，并修改你的配置。  
插件会通过判断是否存在`/sdcard/Android/data/{package_name}/files/il2cpp`来自动加载。

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

- `fps` 需要设置的`fps`
- `delay` 游戏载入后等待`delay`秒执行
- `mod_opcode` 是否修改`opcode`，如果你发现游戏会重新锁定fps，可以把这项改为`true`，但由于修改内存，可能会被反作弊检测到
- `scale` 设置分辨率的倍数，一般保持`1.0`即可，必须为小数。`当前屏幕宽度 * scale x 当前屏幕高度 * scale`

然后，`custom`节点中的配置会覆盖`global`中的配置单独生效:

- `key` 包名，比如`com.random.package.name.a`
- `fps` 同上
- `mod_opcode` 同上
- `delay` 同上
- `scale` 同上

`TargetList.json`修改后可以搜索`json 格式校验`校验是否完整。修改后立即生效（`>=1.8`）。  
可以在终端模拟器输入`logcat -s UnityFPSUnlocker`查看输入日志。

## 分辨率
最近发现`BlueArchive`最高分辨率只有`1080P`，在模拟器上有肉眼可见的锯齿，而且在`16:10`的比例下甚至像素点比`16:9`更少

`2560x1600->1822x1138`

对比

`2560x1440->1920x1080`

所以加上调整分辨率的功能，对比图如下

https://imgsli.com/MjI3NDQ2/0/1

https://imgsli.com/MjI3NDQ2/2/3

但是设置的时机需要尽可能早，也就是`delay`尽可能短，否则需要更改`绘图`选项中的任意一项后生效（比如开关一次`抗锯齿`）
