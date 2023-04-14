# UnityFPSUnlocker
## インストール条件
- [Magisk](https://github.com/topjohnwu/Magisk/releases)
- `Zygisk`の有効化
- `Shamiko`の有効化
<br>ShamikoはSafetyNetによる対策が入ったゲームに適用する場合に必要になる事があります。(ウマ娘など)

## 使用方法
UnityFPSUnlockerとZygiskの有効化を行ない、`TargetList.json`を`/data/local/tmp/`に配置をしてください。<br>
プラグインは`/sdcard/Android/data/{パッケージ名}/files/il2cpp`にファイルが入っていると自動的にロードされます。

```
{
  "global": {
    "delay": 10,
    "mod_opcode": true,
    "fps": 90
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

以下は`global`ノードの設定方法です:
- `fps`はアンロックを行なう`FPS値`を入力します。
- `delay`はアンロックを行なうまでの`ディレイ値`の設定です。
- `mod_opcode`は`オペコード`の変更モードです。<br>
FPSを再ロックしてしまうゲームでこのオプションを`ture`にする事で、完全なアンロックが可能になりますがメモリの変更を行なうため「アンチチート機能」が反応してしまう事があります。

`custom`ノードの設定は`global`ノードの設定を上書きし、別々で有効化されます:
- `キー`となるパッケージ名、例:`com.random.package.name.a`
- `fps` `global`ノード設定方法と同上
- `mod_opcode` `global`ノード設定方法と同上
- `delay` `global`ノード設定方法と同上

`TargetList.json`に`JSONのフォーマットのチェック`を行なうようにしました。(>=1.8)  
動作のログはターミナルエミュレーターで`logcat -s UnityFPSUnlocker`を入力する事でログを確認できます。
