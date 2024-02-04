# nkodicebot

某チンチロっぽい遊びを IRC 上で提供します。

## 依存

適当なバージョンの Node.js が必要です。

## ビルド

```console
$ make
```

## 起動

*`server`* は接続する IRC サーバの名前、*`'#chan'`* は join するチャンネルの名前です。
ポート番号は 6667 に固定されています。

```console
$ ./nkobot server '#chan'
```

## コマンド

<dl>
<dt>$START$</dt>
<dd>スコアの集計をリセットします。新しいゲームの始まりです。</dd>
<dt>$DICE$</dt>
<dd>ダイスを振ります。</dd>
<dt>$$$DICE50$$$</dt>
<dd>$DICE$ を 50 回実行するのと同じです。出力は早送りされます。</dd>
<dt>$END$</dt>
<dd>スコアの集計結果を表示します。</dd>
</dl>

## バグ

よく Segmentation Fault して異常終了します。
