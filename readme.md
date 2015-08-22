#サクラエディタ 私用版
　このサクラエディタは、各種パッチを適用して新機能の開発・テストのため本家版からforkしたものです。
また、個人のGitのテスト運用を兼ねています。  
　tags/branchs/trunk等trunk2以外はミラーしていません。

## 本家
　http://sakura-editor.sourceforge.net/

##バイナリの入手
x86版 release tagのページからどうぞ  
https://github.com/mocaskr/sakura/releases/tag/v020.20150823-r4027  

chm版ヘルプは release tagのページにあります  
https://github.com/mocaskr/sakura_help/releases/tag/v012  
v003からヘルプの参照先を変更しました  
http://mocaskr.web.fc2.com/sakurahelp2/HLP000001.html  
ヘルプのソース・変更履歴は以下にあります  
https://github.com/mocaskr/sakura_help  

##注意
比較的枯れてないパッチも混ざっているため、バグに注意してください。  
本家でも再現するバグは本家のほうへお願いします。  
私用版固有の問題は、issuesを活用してください。fork/pull requestも歓迎します。
基本的には、一度pushした物はrebaseしません。本家trunk2のHEADを可能な限り追いかける予定です。
マージ困難な場合などでtrunk2側を優先し適用済みパッチを削除する可能性もあります。

## 変更内容
sakura 2.2.0.1 mocaskr_ver 020.20150823-r4027  
baseバージョンからの変更点一覧  
New/Imp/Chg
* プロポーショナルフォント
* ウィンドウ一覧ダイアログ
* 共通設定のツリー表示
* マウスジェスチャー
* タイプ別設定の追加と強調キーワードの外部化
* フォントスタイルに斜体(仮)と取り消し線を追加
* Grepファイル名で正規表現フィルター
* ダイアログのフォント変更設定追加
* 行番号の10行毎の強調表示
* 強調キーワードを20個に拡張
* ダブルクリック(URL選択)にコマンド割り当て
* プラグインをマクロ記録できるようにする等マクロ変更
* マクロの保存パラメータの取得マクロ追加
* コメントのネスト
* XMLアウトラインとHTMLの修正
* DIFF差分の相手ファイルの文字コード指定
* ExpandParameter("$\<patchinfo\>")の追加
* カスタムメニュー・ツールバー・キーバインドの初期化ボタン
* 全設定リセット機能追加
* ダイレクトタグジャンプでの起点フォルダ変更機能
* Grep(置換)でファイル部分もファイル選択ダイアログから設定できるように
* ini読み取り専用オプション
* iniの履歴を分離して記憶するオプション
* 大きいツールバーアイコン対応
* カーソル左右リピート設定(iniのみ)
* カーソル位置下のマウススクロール
* 上下スクロールでEOFを一番上に表示オプション
* ファイル読み込みでスレッドを使う
* マクロの文字列コピーを減らす
* CProfileの読み込みの高速化
* CIfObj::GetIDsOfNamesの比較をmapに使って高速化
* カラーの文字列にC++11を追加,C/C++はC/C++03に変更しRawを処理しない
* タイプ別のエクスポートで入力補完プラグインのId名を使うように
* 入力補完プラグインを複数定義可能に
* Webヘルプでcgiをつかなわない/URL変更/URL設定追加
* マクロ保存でS_を追加しない
* マクロフォルダのデフォルトをiniフォルダから相対パスに変更
* 色つきHTMLコピーの出力HTMLの一部省略
* 背景色印刷
* 選択文字数に改行コード分を含めるオプション

Fix
* C/C++用強調キーワードにてfinalとfloatが動作しない
* 印刷プレビュー中の各種不具合修正(ミニマップ、タイプ別変更、印刷番号変更)
* UTF-7自動認識を厳しくする
* メニュー自動追加でアクセスキーが追加されない
* プロファイルマネージャを画面中央に表示
* アウトラインを上下にドッキングした場合の初期表示の高さが不正
* ユーザ別設定のときのマルチプロファイルの場所がおかしい
* ミニマップのDIFFマークを非表示に修正
* 設定画面のファイル選択ダイアログで相対パスにできるときはする

ベースバージョンからベースリビジョンの間のtrunk2側での修正点
* C/C++インデントの改良
* TSC、CSVモード
Fix/Keep
* 正規表現のGrep置換で行内の1つめしか置換されない
* Grepの検索対象ファイルを厳密にする
* コンパイルオプションに/NXCOMPAT /SAFESEH /DYNAMICBASEを追加
* 正規表現OFF置換で異常終了する
* USE_CRASHDUMPをデフォルトで有効化
* メッセージボックスのコピーがおかしい
* DLL,EXEの検索パスのセキュリティ設定変更
* TCITEMのcchTextMaxメンバが初期化されていなかった。
* (選択)前の変更行へが追加されるときにもセパレータが余分に追加される
