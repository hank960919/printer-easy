# printer-easy

這是一個以 HTML / CSS / TypeScript 分離的網頁版小畫家，保留原本的繪圖、選色、匯入圖片、儲存圖片等功能。

## 專案結構

- [printer/index.html](printer/index.html)：頁面結構
- [printer/styles.css](printer/styles.css)：樣式與版面
- [printer/app.ts](printer/app.ts)：繪圖與互動邏輯
- [printer/app.js](printer/app.js)：瀏覽器可直接執行的 JavaScript（可直接開啟）

## 立即使用（最簡單方式）

你不需要安裝任何套件，也不需要編譯：

1. 下載或 clone 這個 repository
2. 開啟瀏覽器
3. 直接開啟 [printer/index.html](printer/index.html)
4. 若你的瀏覽器對本地檔案有安全限制，建議改用本機伺服器方式開啟

## 透過本機伺服器開啟

若你希望更穩定地跑這個頁面，可以在專案根目錄執行：

```bash
cd printer
python -m http.server 8000
```

然後在瀏覽器打開：

```text
http://localhost:8000/index.html
```

## 功能說明

- 畫筆與橡皮擦切換
- 顏色選擇
- 復原上一筆
- 匯入圖片並拖曳／縮放
- 右鍵點擊已放置圖片即可刪除
- 儲存為 PNG、JPEG、BMP
- 支援快捷鍵：
  - `Ctrl + P`：畫筆
  - `Ctrl + E`：橡皮擦
  - `Ctrl + Z`：復原
  - `Ctrl + I`：匯入圖片
  - `Ctrl + S`：儲存圖片
  - `C`：選色

## GitHub 上傳說明

如果你要把這個專案上傳到 GitHub，請先在終端機執行：

```bash
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://github.com/你的帳號/你的專案.git
git push -u origin main
```

如果你已經有遠端倉庫，則改用：

```bash
git remote set-url origin https://github.com/你的帳號/你的專案.git
git push -u origin main
```

## 開發與編譯

如果你想從 TypeScript 來源重新產生 JavaScript，可以在專案根目錄執行：

```bash
npx tsc --project tsconfig.json
```

這會將 [printer/app.ts](printer/app.ts) 編譯成 [printer/app.js](printer/app.js)。

## 授權

本專案僅供學習與個人使用，請依照你的實際需求決定是否加入商業授權或公開條款。
