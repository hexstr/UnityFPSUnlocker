# FGOFPSUnlocker 
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
(~~我也不知道会不会导致~~大概是不会封号  
[点击下载](https://github.com/nishuoshenme/FGOFPSUnlocker/releases)

- 国服没有检测`xposed`所以直接做成`xposed模块`了
- 日服是一个`magisk-riru模块`，需要安装[magisk](https://github.com/topjohnwu/Magisk/releases/)
- 修改`il2cpp.so`
	- 下载[il2cppdumper](https://github.com/Perfare/Il2CppDumper/releases)
	- 从安装包提取`lib/arm64-v8a/il2cpp.so`和`assets/bin/Data/Managed/Metadata/global-metadata.dat`
	- 打开`Il2CppDumper.exe`并分别选择`il2cpp.so`和`global-metadata.dat`
	- 输入`2018.4`按下回车和`3`
	- 在`dump.cs`中搜索`set_targetFrameRate`并记下地址
	- 使用十六进制编辑器跳转到地址并修改`FF C3 00 D1 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 84 00 B0 81 6A 40 F9 [F3 03 00 2A]`为`FF C3 00 D1 F4 4F 01 A9 FD 7B 02 A9 FD 83 00 91 F4 84 00 B0 81 6A 40 F9 [93 07 80 52]`
	- 替换修改后的文件，完成~

# 最后
欢迎打赏~  
![alipay](https://github.com/nishuoshenme/fgoUnlockFPS/raw/master/alipay.png)

***
以上ᕕ( ᐛ )ᕗ  
收到台服玩家们的￥100打赏，感谢~  
收到￥100打赏，感谢~