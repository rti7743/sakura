#サクラエディタ 私用版
　このサクラエディタは、各種パッチを適用して新機能の開発・テストのため本家版からforkしたものです。
また、個人のGitのテスト運用を兼ねています。  
　tags/branchs/trunk等trunk2以外はミラーしていません。

## 本家
　http://sakura-editor.sourceforge.net/

##バイナリの入手
x86版 release tagのページからどうぞ  
https://github.com/mocaskr/sakura/releases/tag/v010.20150213-r3977  

chm版ヘルプは release tagのページにあります  
https://github.com/mocaskr/sakura_help/releases/tag/v007  
v003からヘルプの参照先を変更しました  
http://mocaskr.web.fc2.com/sakurahelp2/HLP000001.html  
ヘルプのソース・変更履歴は以下にあります  
https://github.com/mocaskr/sakura_help  

## 変更内容
sakura 2.1.1.4 mocaskr_ver 011.20150213-r3977  
baseリビジョンからの変更点一覧  
 713	プロポーショナルフォント  
 717	C/C++インデントの改良  
 735	ファイル読み込みでスレッドを使う  
 805	DLL,EXEの検索パスのセキュリティ設定変更  
 826	コンパイルオプションに/NXCOMPAT /SAFESEH /DYNAMICBASEを追加  
 837	フォントスタイルに斜体(仮)と取り消し線を追加  
 ---	Webヘルプでcgiをつかなわない/URL変更/URL設定追加(2d4a13ce)  
 ---	ExpandParameter("$\<patchinfo\>")の追加(13e8a713)  
 883	Grepに否定行検索を追加  
 882	置換に行削除モード追加  
 830	マクロの文字列コピーを減らす  
 865	マクロ保存処理の変更  
 850	コメントのネスト  
 884	カスタムメニュー・ツールバー・キーバインドの初期化ボタン  
 885	全設定リセット機能追加  
 889	Grepファイル名で正規表現フィルター  
 908	CProfileの読み込みの高速化  
 914	CIfObj::GetIDsOfNamesの比較をmapに使って高速化  
 920	ダイアログのフォント変更設定追加  
 937	ini読み取り専用オプション  
 936	iniの履歴を分離して記憶するオプション  
 938	大きいツールバーアイコン対応  
 944	XMLアウトラインとHTMLの修正  
番号と詳細は http://sourceforge.net/p/sakura-editor/patchunicode/ 参照  
