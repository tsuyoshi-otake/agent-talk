<p align="center">
  <h1 align="center">AgentTalk</h1>
  <p align="center">
    Microsoft Office 2000のオフィスアシスタント風のGUIを持つAIチャットインターフェイス。クリッピーやカイル（イルカ）といった懐かしいキャラクターとの会話を楽しめます。
  </p>
</p>

---

## 動作要件

- OpenAI APIキー
- Windows11対応
- Visual C++ 再頒布可能パッケージ（最新版）
- .NET Framework 3.5

## インストール方法

1. Visual C++ 再頒布可能パッケージをインストール
   - [Microsoft公式サイト](https://learn.microsoft.com/ja-jp/cpp/windows/latest-supported-vc-redist?view=msvc-140)からダウンロードしてインストール

2. .NET Framework 3.5を有効化
   - Windowsキーを押して「Windows Features」または「Windowsの機能」を検索
   - 「Windows の機能の有効化または無効化」を開く
   - 「.NET Framework 3.5 (.NET 2.0 および 3.0 を含む)」にチェックを入れる
   - 「OK」をクリックして変更を適用

3. アプリケーションをダウンロードしてインストール
4. 設定画面でOpenAI APIキーを設定
5. 希望の言語とキャラクターを選択
6. チャットを開始！

## 使い方

![kairu](images/kairu.gif)

1. テキストボックスに質問や指示を入力
2. 「送信」ボタンをクリックまたはCtrl+Enterを押す
3. AIからの応答を確認
4. 会話を続ける場合は「続ける」をクリック（またはCtrl+Enter）
5. 会話をリセットする場合は「リセット」をクリック（またはCtrl+Backspace）

クイックチャット：
- ダブルクリックでチャットパネルの表示/非表示を切り替え可能。作業中の素早い質問に最適です。

## 機能

AgentTalkは、懐かしのMicrosoft Officeアシスタントの体験を最新のAI機能と組み合わせて提供します：

### 主な機能

- **AIとのインタラクティブなチャット**:
  - 会話の文脈を保持した対話が可能
  - キャラクター設定：クリッピー（英語）またはカイル（日本語）
  - AIの応答を自動でクリップボードにコピー可能

- **クラシックなUI体験**:
  - Microsoft Office 2000スタイルのインターフェイス
  - デスクトップ上で自由に移動可能
  - 常に最前面表示のオプション
  - 入力フォームの拡大表示機能

- **ショートカットキー**:
  - **Ctrl+Enter**: 送信/続ける
  - **Ctrl+Backspace**: 会話をリセット
  - **Ctrl+A**: テキスト全選択（編集モード時）
  - **ダブルクリック**: 入力フォームの表示/非表示切り替え

### 高度な機能

- **キャラクター設定**:
  - 英語モード：Microsoft Office 2000のクリッピー
  - 日本語モード：Microsoft Office 2000のカイル（イルカ）
  - 選択したキャラクターに応じたシステムプロンプトの自動調整

- **カスタマイズオプション**:
  - 言語切り替え（英語/日本語）
  - 入力フォームサイズの拡大
  - キャラクター設定の有効/無効
  - 自動クリップボードコピー
  - APIキーの設定
  - AIモデルの選択
  - システムプロンプトのカスタマイズ

## 設定オプション

AgentTalkは以下の設定オプションを提供します：

- **言語**: 英語/日本語の切り替え（キャラクター選択に影響）
- **入力フォーム**: 拡大表示の有効/無効
- **アシスタント人格**: キャラクター設定の有効/無効
- **自動クリップボード**: 応答の自動コピー機能
- **API設定**:
  - APIキーの設定
  - 使用モデルの選択
- **システムプロンプト**: AI動作のカスタマイズ（キャラクター設定無効時のみ）

## コマンド

- ダブルクリック：チャットパネルの表示/非表示
- Ctrl+Enter：メッセージ送信/会話を続ける
- Ctrl+Backspace：会話をリセット
- Ctrl+A：テキスト全選択（編集モード時）

## 注意事項

- OpenAI APIキーが必要です
- APIリクエストには料金が発生することがあります（OpenAIの料金体系に従います）
- デフォルトのホストとエンドポイントは変更できません
- システムプロンプトのカスタマイズは、キャラクター設定が無効な場合のみ可能です

## ライセンス

このプロジェクトはMITライセンスの下で提供されています - 詳細は[LICENSE](LICENSE)ファイルをご覧ください。
