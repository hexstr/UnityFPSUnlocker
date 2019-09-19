# fgoUnlockFPS  
为fgo、bgo和bgo(tw)提供在手机上解锁60FPS的模块。
# 原理
原理参考这里：[Application-targetFrameRate](https://docs.unity3d.com/ScriptReference/Application-targetFrameRate.html)  
就一行代码
```Csharp
UnityEngine.Application.targetFrameRate = 60;
//把`60`换成`-1`就是不限制帧率
```
然后注入dll调用一下上面这个函数完事儿...  
更新2.0之后换成了`il2cpp`这样就更简单了，利用自带api`il2cpp_resolve_icall`直接调用

# 使用
(我也不知道会不会导致封号  
[点击下载](https://github.com/nishuoshenme/bgoUnlockFPS/releases)

- 国服没有检测`xposed`所以直接做成`xposed模块`了
- 日服暂时没有检测，直接替换`libil2cpp.so`即可，位于`/data/app/com.aniplex.fategrandorder-1/lib/`下载对应架构的文件替换即可
- ~~日服是一个`magisk-riru模块`，需要安装[magisk](https://github.com/topjohnwu/Magisk/releases/)并且在`Magisk Manager`中使用随机包名和`Magisk Hide`隐藏，然后安装[riru-core](https://github.com/RikkaApps/Riru/releases)，最后安装模块即可~~

  - ~~说起来还有一种手动替换的方法，不需要安装这些东西，只需要有`twrp`并且能对`system`分区读写就行~~

    > ~~首先是`Riru`,下载最新的[releases](https://github.com/RikkaApps/Riru/releases)并解压`/system/lib`文件夹  
    > 重命名手机中的`/system/lib/libmemtrack.so`为`libmemtrack_real.so`(最好先备份一下`/system/lib/libmemtrack.so`  
    > 从解压的`Riru`文件夹中找到`libmemtrack.so`复制粘贴到手机的`/system/lib`文件夹  
    > 接着解压模块，复制`data`文件夹粘贴到手机对应的文件夹合并  
    > 然后是模块中`/system/lib/libriru_fgounlockfps.so`不用说，复制到手机的对应位置  
    > 最后重启，之后`adb logcat -s Riru:V`一下`Riru`会出现日志  
    > 总之就是把对应的文件替换正确就行，还不清楚的话去看一下`Riru`的`install.sh`~~

# 最后
欢迎打赏~  
![alipay](https://github.com/nishuoshenme/fgoUnlockFPS/raw/master/alipay.png)

***
以上ᕕ( ᐛ )ᕗ  
收到台服玩家们的￥100打赏，感谢~