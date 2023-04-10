# UnityFPSUnlocker
## 安装需求
- [Magisk](https://github.com/topjohnwu/Magisk/releases)
- 启用`zygisk`

## 使用
下载并刷入模块。在重启之前，先下载`TargetList.json`放入`/data/local/tmp/TargetList.json`，并修改你的配置。

- `framerate` 需要设置的`fps`
- `delay [second, > 5 recommended]` 游戏载入后等待`delay`秒执行解锁
- `normal_list` 游戏包名（普通）
- `mod_list` 游戏包名（修改opcode，如果游戏会重新锁定FPS，可以把包名填在这里，但由于修改内存，可能会被检测到）

`TargetList.json`修改后可以搜索`json 格式校验`校验是否完整。修改后立即生效（>=1.8）。  
可以在终端模拟器输入`logcat -s UnityFPSUnlocker:V`查看输入日志。

`2.0`为了支持单独游戏单独配置，格式有变化。

`3.0`通过判断是否存在`/sdcard/Android/data/{package_name}/files/il2cpp`来自动加载。
