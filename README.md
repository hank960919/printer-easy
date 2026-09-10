# printer-easy

一個用 Win32 GDI 寫成的簡易小畫家程式（`printer/printer.cpp`），並附上功能對應的網頁版。

## 網頁版小畫家（免編譯，直接用瀏覽器開啟）

[`printer/printer.html`](printer/printer.html) 是把 `printer.cpp` 的小畫家功能改寫成的純 HTML/CSS/JavaScript 網頁版，**不需要 Visual Studio、不需要編譯、也不需要安裝任何東西**：

1. 下載或 clone 這個 repo
2. 直接用瀏覽器打開 `printer/printer.html`（雙擊即可）即可使用

### 功能

- 滑鼠 / 觸控 / 手寫筆拖曳畫線
- 選單列：檔案／色彩／動作／說明（比照原程式選單）
- 選擇顏色（快捷鍵 `C`）
- 畫筆／橡皮擦切換（`Ctrl+P` / `Ctrl+E`）
- 復原上一筆（`Ctrl+Z`）
- 匯入圖片並自由拖曳、等比縮放位置，確認後才會落版到畫布上（`Ctrl+I`）
  - 在已放置的圖片上按滑鼠右鍵，可直接刪除該圖片，不必逐筆復原疊在它上面的畫線
- 儲存圖片為 PNG / JPEG / BMP（`Ctrl+S`）

## 原始 Win32 專案

`printer/printer.cpp` 是使用 Win32 API + GDI（儲存圖片則用 WIC）寫成的原生小畫家程式，需要以 Visual Studio 開啟 `printer.slnx` 編譯執行。
