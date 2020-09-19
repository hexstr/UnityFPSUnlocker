# 修改二进制文件
使用`ida pro`打开`libunity.so`，等待分析结束后按下`shift + F12`打开字符串列表，`ctrl + F`搜索
```
set_targetFrame
```
双击，跳转到字符串，双击上方的函数`sub_2FE430`
![step1](https://github.com/nishuoshenme/FGOFPSUnlocker/raw/patchfile/imgs/1.jpg)
双击`B指令`右侧函数继续跳转
![step2](https://github.com/nishuoshenme/FGOFPSUnlocker/raw/patchfile/imgs/2.jpg)
跳转2到3次后应该能够看到代码
![step3](https://github.com/nishuoshenme/FGOFPSUnlocker/raw/patchfile/imgs/3.jpg)
光标移至`00406F2C`行，依次点击`Edit->Patch program->Change byte`
![step4](https://github.com/nishuoshenme/FGOFPSUnlocker/raw/patchfile/imgs/4.jpg)
把
```
04 10 9F E5 01 00 8F E7 1E FF 2F E1 F4 B0 9C 00
```
修改为`NOP`
```
00 F0 20 E3 00 F0 20 E3 1E FF 2F E1 F4 B0 9C 00
```
之后双击上方`unk_DD202C`点击`Edit->Patch program->Change byte`
![step5](https://github.com/nishuoshenme/FGOFPSUnlocker/raw/patchfile/imgs/5.jpg)
把
```
FF FF FF FF 00 00 00 00 00 00 00 00 80 43 CE 00
```
修改为你想设置的帧率，比如`0x3C`
```
3C 00 00 00 00 00 00 00 00 00 00 00 80 43 CE 00
```
最后，点击`Edit->Patch program->Apply patches to input file...`并确定，即可。
***
以上ᕕ( ᐛ )ᕗ