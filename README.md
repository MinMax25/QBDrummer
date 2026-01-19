# QBDrummer
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](Resources/license.txt)
[![VST3](https://img.shields.io/badge/VST3-MIDI%20Plugin-green)](https://steinberg.net/)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue)](#)

DAW（Studio One、Cubase）で使用する **ドラム用 MIDI プラグイン** です。  
QBDrumMap で作成したドラムマップを使用し、  
ドラム MIDI をリアルタイムに変換します。

A **drum MIDI plugin** for DAWs such as **Studio One** and **Cubase**.  
It converts drum MIDI in real time using drum maps created with **QBDrumMap**.

<img src="QBDrummer/resource/images/screenshot_main.png" />

---

## 🎯 役割 | Role

- **QBDrumMap**：ドラムマップの作成・編集（スタンドアロン）
- **QBDrummer**：ドラムマップの適用・変換（VST3 プラグイン）

---

- **QBDrumMap**: Drum map creation and editing (standalone tool)  
- **QBDrummer**: Drum map playback and MIDI translation (VST3 plugin)

---

## 🧠 概要 | Overview

QBDrummer は、  
異なるドラム音源間で発生する **ノート配置の違い** を吸収するための  
**MIDI ユーティリティプラグイン**です。

ドラムマップを介して  
「ドラムの意味（Articulation）」を基準に変換を行うため、  
同じ MIDI データを複数のドラム音源で使い回すことができます。

---

QBDrummer is a **MIDI utility plugin** designed to absorb  
differences in drum note layouts between drum instruments.

By converting MIDI through **drum articulations**,  
the same drum MIDI data can be reused across multiple instruments.

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

## 📦 ダウンロード・インストール | Download & Installation

### 手順 | Steps

1. このページ右側の **Releases** からダウンロードしてください。  
2. ダウンロードした zip を展開します。  
3. 生成された **VST3 ファイル** を  
   VST3 フォルダに配置してください。

---

1. Download from **Releases** on this page.  
2. Extract the zip file.  
3. Copy the **VST3 file** to your VST3 directory.

### VST3 インストール先 | VST3 Location

- Windows  
  `C:\Program Files\Common Files\VST3`

---

## 🧪 動作確認済み DAW | Tested DAWs

- Studio One  
- Cubase  

※ 他の VST3 対応 DAW でも動作するはずです  
Other VST3-compatible DAWs should work as well.

---

## ⚠️ 注意事項 | Notes

- QBDrummer は **MIDI 専用 VST3 プラグイン**です  
- Audio の加工は行いません  
- 別途 **QBDrumMap** が必要です  

---

- QBDrummer is a **MIDI-only VST3 plugin**  
- Audio is not processed  
- **QBDrumMap** is required

  
---

## Build Requirements

This project is a VST3 MIDI plugin built using the Steinberg VST3 SDK.

To build QBDrummer from source, you will need:

- Windows 10 or later
- Visual Studio 2022
- Steinberg VST3 SDK

The VST3 SDK is **not included** in this repository.
Please download it from the official Steinberg website:

https://developer.steinberg.help/display/VST/VST+3+SDK

After downloading the SDK, configure your project to reference the local
VST3 SDK path according to your development environment.

---

## Third-Party Notices

This product uses the Steinberg VST3 SDK.
VST is a trademark of Steinberg Media Technologies GmbH.

---

## 📄 ライセンス | License

MIT License

---

## ✍️ 作者 | Author

MinMax
