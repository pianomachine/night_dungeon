# PCG + GC Fatal Error 修正 思考プロセス記録

## 概要
- 日付: 2025-12-31
- タスク種別: エラー修正 / パフォーマンス最適化
- 難易度: 高
- 所要時間: 約2時間

---

## フェーズ1: 問題の認識

t+0: [トリガー]
     UE5ゲームで2秒周期のカクつき（スタッター）が発生
     さらにPCG実行中に Fatal Error でクラッシュ

t+1: [分類]
     - 種類: ランタイムエラー + パフォーマンス問題
     - 緊急度: 高（ゲームがクラッシュする）
     - 影響範囲: 広範囲（ゲーム全体のプレイアビリティに影響）

---

## フェーズ2: 原因の特定

t+2: [仮説]
     「PCGのGPU実行とGarbage Collectionが競合しているのではないか」

     エラーメッセージ:
     ```
     Illegal call to StaticFindObjectFast() while garbage collecting
     ```

t+3: [検証方法の選択]

| 選択肢 | 方法 | 採用 |
|--------|------|------|
| A | UE5ソースコードを調査 | |
| B | stat dumphitches でボトルネック特定 | ✓ |
| C | PCG設定を変更して再現テスト | ✓ |

選択理由: 実際の動作を計測し、問題箇所を特定する方が効率的

t+4: [調査結果]
     - `stat dumphitches` で以下を発見:
       - Physics body creation: 2-16ms（SM_Shroom_*, SM_Rock_*）
       - BP_FMPerception_Main.MainTrace: 2-5ms
       - HZB Map Results: 206ms（GPU-CPU同期待ち）
       - LandscapeSubsystem Tick: 44ms
     - PCGがGPUモードで実行中にGCが走るとクラッシュ

---

## フェーズ3: 解決策の設計

t+5: [候補の比較]

| 案 | 内容 | メリット | デメリット | 採用 |
|----|------|----------|------------|------|
| 1 | PCG GPU実行を無効化 | 確実にクラッシュ回避 | GPU高速化の恩恵なし | ✓ |
| 2 | GC頻度を下げる | 根本解決 | メモリ使用量増加 | △ |
| 3 | PCGを非同期実行しない | 確実 | PCG実行が遅くなる | ✓ |

t+6: [選択理由]
     - PCG GPU実行の無効化が最も安全で確実
     - 非同期実行も無効化して競合を完全に回避
     - GC設定も併せて調整してバランスを取る

---

## フェーズ4: 実装と検証

t+7: [実装内容]

**変更ファイル:** `Config/DefaultEngine.ini`

```ini
[/Script/PCG.PCGSubsystem]
bDisableGPU=True
bDisableAsyncExecution=True

[/Script/PCG.PCGSettings]
bForceDisableGPU=True

[ConsoleVariables]
pcg.graph.DisableGPUExecution=1
pcg.GPU.Enable=0
pcg.DisableAsyncDataPreparation=1
```

**追加対応:** Box Simplified Collision の一括適用

```python
# UE5 Python スクリプト
import unreal

asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
static_mesh_subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)

filter = unreal.ARFilter(
    package_paths=[
        "/Game/ScifiJungle/Models/Nature/Foliage/Shrooms",
        "/Game/ScifiJungle/Models/Nature/Rocks"
    ],
    recursive_paths=True,
    class_names=["StaticMesh"]
)
assets = asset_registry.get_assets(filter)

count = 0
for asset_data in assets:
    mesh = asset_data.get_asset()
    if mesh and "SM_" in str(asset_data.asset_name):
        static_mesh_subsystem.remove_collisions(mesh)
        static_mesh_subsystem.add_simple_collisions(mesh, unreal.ScriptingCollisionShapeType.BOX)
        count += 1

print(f"Done! {count} meshes")
```

t+8: [検証結果]
     - 検証方法: PIE で `stat dumphitches` 実行
     - 結果: クラッシュ解消、ヒッチ 26回/3037ms → 16回/1868ms に改善

---

## フェーズ5: 学びと次への活用

### 今回の判断軸

| 軸 | 説明 |
|----|------|
| 安定性優先 | パフォーマンスよりクラッシュ回避を優先 |
| 段階的対処 | 一度に全て直そうとせず、問題を分離して対処 |
| 計測駆動 | stat dumphitches で数値を見て判断 |

### 次に活かすポイント

1. **PCG + GC の競合パターンを認識**
   - PCG GPU実行中のGCは危険
   - 大量のアセット生成時は特に注意

2. **stat dumphitches の活用**
   - ヒッチの原因を特定するのに最適
   - 改善前後で数値比較する習慣

3. **Python一括処理の威力**
   - 24メッシュのCollision適用を自動化
   - 手作業では数時間かかる作業を数秒で

### パターン化

このタイプの問題が再発したら:
1. まず `stat dumphitches` でボトルネック特定
2. 次にエラーメッセージからキーワード抽出（今回: GC, StaticFindObject）
3. DefaultEngine.ini で関連設定を無効化
4. 改善を計測で確認

---

## 残課題

| 問題 | 状態 | 備考 |
|------|------|------|
| HZB Map Results (206ms) | 未解決 | GPU-CPU同期の根本問題 |
| BP_FMPerception_Main (2-5ms) | 軽減のみ | C++化で更に改善可能 |
| Cache Uniform Expressions (14-30ms) | 未解決 | マテリアル数削減が必要 |
