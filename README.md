# UnityFPSUnlocker
## インストール条件
- [Magisk](https://github.com/topjohnwu/Magisk/releases)
- `Zygisk`の有効化
- `Shamiko`の有効化
<br>ShamikoはSafetyNetによる対策が入ったゲームに適用する場合に必要になる事があります。(ウマ娘など)

## 使用方法
UnityFPSUnlockerとZygiskの有効化を行ない、`TargetList.json`を`/data/local/tmp/`に配置をしてください。

- `フレームレート`は`fps`の項目で設定をします。
- `ディレイ値は [5秒を推奨します]` `delay`で設定をした時間の経過後にFPSのアンロックが実行されます。
- `normal_list` はゲームのパッケージ名を設定します(通常モード)
- `mod_list` はオペコードの改変モードになります。
この項目にFPS値の変更が再ロックされてしまうゲームのパッケージ名を記述する事で完全にアンロックがされます。
しかし、ゲームによってはメモリ改造を行なっていると認識される場合があります。

`TargetList.json`に`JSONのフォーマットのチェック`を行なうようにしました。(>=1.8)  
`logcat -s UnityFPSUnlocker:V`で動作のログを確認する事ができます。

`2.0` ゲームごとに設定を変更を可能にするために形式を変更しました。

`3.0` `/sdcard/Android/data/{package_name}/files/il2cpp`にファイルが存在する場合はそちらを自動的にロードするようにしました。
