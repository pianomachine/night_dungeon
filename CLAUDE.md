# Claude Code Rules

## Unreal Engine C++ Class Creation

**重要: C++クラスは必ずUE5エディタから作成すること**

Claude CodeやVS Codeなどのコードエディタで直接 `.h` / `.cpp` ファイルを作成してはいけない。

### 理由

- UE5はクラス作成時に `.uproject` や `.generated.h` などのメタデータを自動生成する
- 手動で作成したファイルはビルドシステム（UnrealBuildTool）に認識されない
- リフレクションシステム（UCLASS, UPROPERTY等）が正しく動作しない
- ホットリロードやライブコーディングで問題が発生する

### 正しい手順

1. UE5エディタを開く
2. `Tools` > `New C++ Class...` または コンテンツブラウザで右クリック > `New C++ Class`
3. 親クラスを選択（Actor, Character, Component等）
4. クラス名と保存場所を指定
5. エディタがファイル生成後、自動的にIDEが開く

### Claude Codeでできること

- **既存の** C++ファイルの編集・修正
- ヘッダーへの新しいメンバー変数・関数の追加
- 実装の変更
- **新規クラスの作成はできない**（エディタで行うよう案内すること）

---

## Git Branch Strategy

**必須: Git Flow ブランチ戦略を使用すること**

このプロジェクトでは Git Flow ブランチ戦略を厳守する。

### ブランチ構成

- `main` - 本番リリース用。直接コミット禁止
- `develop` - 開発用メインブランチ。機能統合用
- `feature/*` - 新機能開発用 (例: `feature/add-jump-animation`)
- `release/*` - リリース準備用 (例: `release/1.0.0`)
- `hotfix/*` - 緊急バグ修正用 (例: `hotfix/fix-crash`)

### ワークフロー

1. 新機能は `develop` から `feature/*` ブランチを作成
2. 完成したら `develop` にマージ
3. リリース準備時は `develop` から `release/*` を作成
4. リリース後は `main` と `develop` 両方にマージ
5. 本番の緊急修正は `main` から `hotfix/*` を作成し、修正後 `main` と `develop` にマージ

### コマンド例

```bash
# 新機能開始
git checkout develop
git checkout -b feature/new-feature

# 機能完了後
git checkout develop
git merge feature/new-feature
git branch -d feature/new-feature
```
