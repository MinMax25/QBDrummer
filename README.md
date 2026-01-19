# QBDrummer

VST3 MIDI Utility Plugin  
Drum MIDI note mapping plugin for different drum instruments

---

## ❓ QBDrummer とは | What is QBDrummer?

**QBDrummer** は、  
ドラムマップ編集ツール **QBDrumMap** と連携して動作する  
**VST3 MIDI ユーティリティプラグイン**です。

QBDrumMap で作成したドラムマップを使用し、  
DAW 内でドラム MIDI を **リアルタイムに変換**します。

> ※ QBDrummer 単体ではマップ編集はできません

---

**QBDrummer** is a **VST3 MIDI utility plugin**  
that works together with **QBDrumMap**, a standalone drum map editor.

It translates drum MIDI notes **in real time** inside your DAW  
using drum maps created with QBDrumMap.

> Note: QBDrummer itself does not provide map editing features.

---

## 🎯 できること | Features

- ドラム音源ごとのノート配置をリアルタイム補正  
- 既存のドラム MIDI を書き換えずに使用可能  
- ドラム音源を差し替えても MIDI 修正不要  
- Audio は一切加工しない（MIDI のみ処理）

---

- Real-time drum MIDI note translation  
- No need to rewrite existing drum MIDI data  
- Switch drum instruments without fixing MIDI  
- Audio is passed through unchanged (MIDI only)

---

## 🧠 仕組み | How it works

QBDrummer は、単純なノート番号の置換ではなく、  
**ドラムの意味（Articulation）**を経由して変換します。


この方式により、  
同じ MIDI を異なるドラム音源で使用しても  
意図したドラム構成を保つことができます。

---

Instead of simple note-to-note remapping,  
QBDrummer uses **drum articulations** as an intermediate layer.

This allows the same MIDI pattern to work correctly  
across different drum instruments.

---

## 🔗 QBDrumMap について | About QBDrumMap

ドラムマップの作成・編集には  
**QBDrumMap（スタンドアロンツール）**が必要です。

👉  
https://github.com/MinMax25/QBDrumMap

---

Drum maps must be created using **QBDrumMap**,  
a standalone drum map editor.

👉  
https://github.com/MinMax25/QBDrumMap

---

## 🎹 DAW での使い方 | Usage in DAW

1. ドラムトラックに **QBDrummer（VST3）** を挿す  
2. その後段にドラム音源を挿す  
3. QBDrumMap で作成したドラムマップを読み込む  

Studio One では **Note FX 的な使い方**になります。

---

1. Insert **QBDrummer (VST3)** on a drum track  
2. Insert a drum instrument after QBDrummer  
3. Load a drum map created with QBDrumMap  

In Studio One, QBDrummer can be used as a **Note FX**.

---

## 💡 こんな人におすすめ | Recommended for

- 複数のドラム音源を使い分けている  
- 市販のドラム MIDI パックをよく使う  
- 過去曲のドラム MIDI を流用したい  
- ドラム音源を変えるたびにノートを直すのに疲れた  

---

- Users who use multiple drum instruments  
- Users who often use commercial drum MIDI packs  
- Users who want to reuse existing drum MIDI data  
- Anyone tired of fixing drum notes every time they change instruments  

---

## 📦 ダウンロード | Download

### バイナリ | Binary
GitHub Releases からダウンロードしてください。  
Download from GitHub Releases.

👉  
https://github.com/MinMax25/QBDrummer/releases

---

## 🧪 動作確認済み DAW | Tested DAWs

- Studio One  
- Cubase  

※ 他の VST3 対応 DAW でも動作するはずです  
Other VST3-compatible DAWs should work as well.

---

## ⚠️ 注意事項 | Notes

- QBDrummer は **MIDI 用 VST3 プラグイン**です  
- Audio の加工は行いません  
- 別途 **QBDrumMap** が必要です  

---

- QBDrummer is a **MIDI-only VST3 plugin**  
- Audio is not processed  
- **QBDrumMap** is required

---

## 📄 ライセンス | License

MIT License

---

## 🙋 サポート・フィードバック | Support & Feedback

- バグ報告・要望：GitHub Issues  
- Bug reports and feature requests: GitHub Issues

---

## ✍️ 作者 | Author

MinMax
