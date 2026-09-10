(function () {
  const LOGICAL_W = 1024;
  const LOGICAL_H = 768;

  const canvas = document.getElementById('cv') as HTMLCanvasElement;
  const dpr = window.devicePixelRatio || 1;
  canvas.width = LOGICAL_W * dpr;
  canvas.height = LOGICAL_H * dpr;

  const ctx = canvas.getContext('2d') as CanvasRenderingContext2D;
  ctx.scale(dpr, dpr);
  ctx.lineCap = 'round';
  ctx.lineJoin = 'round';

  const items: any[] = [];
  let currentStroke: any = null;
  let isDrawing = false;
  let isEraser = false;
  let currentColor = '#000000';
  const penThickness = 3;
  const eraserThickness = 20;
  let activeImage: any = null;

  const colorInput = document.getElementById('colorInput') as HTMLInputElement;
  const importInput = document.getElementById('importInput') as HTMLInputElement;
  const imgOverlay = document.getElementById('imgOverlay') as HTMLDivElement;
  const imgBox = document.getElementById('imgBox') as HTMLDivElement;
  const imgToolbar = document.getElementById('imgToolbar') as HTMLDivElement;
  const modeLabel = document.getElementById('modeLabel') as HTMLSpanElement;
  const colorSwatch = document.getElementById('colorSwatch') as HTMLSpanElement;
  const miPenChk = document.querySelector('#mi-pen .chk') as HTMLElement;
  const miEraserChk = document.querySelector('#mi-eraser .chk') as HTMLElement;
  const saveModal = document.getElementById('saveModal') as HTMLDivElement;
  const fileNameInput = document.getElementById('fileNameInput') as HTMLInputElement;
  const formatSelect = document.getElementById('formatSelect') as HTMLSelectElement;
  const aboutModal = document.getElementById('aboutModal') as HTMLDivElement;

  function clearCanvas(): void {
    ctx.fillStyle = '#ffffff';
    ctx.fillRect(0, 0, LOGICAL_W, LOGICAL_H);
  }

  function drawStroke(s: any): void {
    if (s.points.length < 2) return;
    ctx.strokeStyle = s.color;
    ctx.lineWidth = s.thickness;
    ctx.beginPath();
    ctx.moveTo(s.points[0].x, s.points[0].y);
    for (let i = 1; i < s.points.length; i++) {
      ctx.lineTo(s.points[i].x, s.points[i].y);
    }
    ctx.stroke();
  }

  function drawImageItem(it: any): void {
    ctx.drawImage(it.img, it.x, it.y, it.w, it.h);
  }

  function redrawAll(): void {
    clearCanvas();
    items.forEach((it) => {
      if (it.type === 'image') {
        drawImageItem(it);
      } else {
        drawStroke(it);
      }
    });
    if (activeImage) {
      drawImageItem(activeImage);
    }
  }

  function drawSegment(a: any, b: any, color: string, thickness: number): void {
    ctx.strokeStyle = color;
    ctx.lineWidth = thickness;
    ctx.beginPath();
    ctx.moveTo(a.x, a.y);
    ctx.lineTo(b.x, b.y);
    ctx.stroke();
  }

  function getPos(e: PointerEvent): { x: number; y: number } {
    const rect = canvas.getBoundingClientRect();
    return {
      x: (e.clientX - rect.left) * (LOGICAL_W / rect.width),
      y: (e.clientY - rect.top) * (LOGICAL_H / rect.height),
    };
  }

  canvas.addEventListener('pointerdown', (e: PointerEvent) => {
    if (activeImage) {
      commitActiveImage();
      return;
    }
    canvas.setPointerCapture(e.pointerId);
    isDrawing = true;
    const pos = getPos(e);
    currentStroke = {
      type: 'stroke',
      color: isEraser ? '#ffffff' : currentColor,
      thickness: isEraser ? eraserThickness : penThickness,
      points: [pos],
    };
    items.push(currentStroke);
  });

  canvas.addEventListener('pointermove', (e: PointerEvent) => {
    if (!isDrawing || !currentStroke) return;
    const pos = getPos(e);
    const last = currentStroke.points[currentStroke.points.length - 1];
    currentStroke.points.push(pos);
    drawSegment(last, pos, currentStroke.color, currentStroke.thickness);
  });

  function endStroke(e: PointerEvent): void {
    isDrawing = false;
    currentStroke = null;
    try {
      canvas.releasePointerCapture(e.pointerId);
    } catch (_err) {
      // noop
    }
  }

  canvas.addEventListener('pointerup', endStroke);
  canvas.addEventListener('pointercancel', endStroke);
  canvas.addEventListener('pointerleave', (e: PointerEvent) => {
    if (isDrawing) endStroke(e);
  });

  canvas.addEventListener('contextmenu', (e: MouseEvent) => {
    e.preventDefault();
    if (activeImage) return;
    const pos = getPos(e as unknown as PointerEvent);
    for (let i = items.length - 1; i >= 0; i--) {
      const it = items[i];
      if (it.type === 'image' && pos.x >= it.x && pos.x <= it.x + it.w && pos.y >= it.y && pos.y <= it.y + it.h) {
        if (confirm('要刪除這張圖片嗎？此動作無法用「復原」還原。')) {
          items.splice(i, 1);
          redrawAll();
        }
        break;
      }
    }
  });

  const menus = document.querySelectorAll('.menu');
  menus.forEach((m: Element) => {
    const btn = m.querySelector('.menu-btn') as HTMLButtonElement;
    btn.addEventListener('click', (ev: MouseEvent) => {
      ev.stopPropagation();
      const wasOpen = m.classList.contains('open');
      menus.forEach((x: Element) => x.classList.remove('open'));
      if (!wasOpen) m.classList.add('open');
    });
  });

  document.addEventListener('click', () => {
    menus.forEach((x: Element) => x.classList.remove('open'));
  });

  function updateModeUI(): void {
    modeLabel.textContent = '模式：' + (isEraser ? '橡皮擦' : '畫筆');
    colorSwatch.style.background = isEraser ? '#ffffff' : currentColor;
    miPenChk.textContent = isEraser ? '' : '✓';
    miEraserChk.textContent = isEraser ? '✓' : '';
  }

  function setEraser(v: boolean): void {
    isEraser = v;
    updateModeUI();
  }

  document.getElementById('mi-pen')?.addEventListener('click', () => setEraser(false));
  document.getElementById('mi-eraser')?.addEventListener('click', () => setEraser(true));

  function undo(): void {
    if (activeImage) {
      cancelActiveImage();
      return;
    }
    if (items.length) {
      items.pop();
      redrawAll();
    }
  }

  document.getElementById('mi-undo')?.addEventListener('click', undo);

  document.getElementById('mi-color')?.addEventListener('click', () => colorInput.click());
  colorInput.addEventListener('input', () => {
    currentColor = colorInput.value;
    isEraser = false;
    updateModeUI();
  });

  document.getElementById('mi-import')?.addEventListener('click', () => importInput.click());

  importInput.addEventListener('change', () => {
    const file = importInput.files && importInput.files[0];
    if (!file) return;

    if (activeImage) commitActiveImage();

    const reader = new FileReader();
    reader.onload = () => {
      const img = new Image();
      img.onload = () => {
        const scale = Math.min(LOGICAL_W / img.naturalWidth, LOGICAL_H / img.naturalHeight, 1);
        const w = img.naturalWidth * scale;
        const h = img.naturalHeight * scale;
        activeImage = {
          img,
          x: (LOGICAL_W - w) / 2,
          y: (LOGICAL_H - h) / 2,
          w,
          h,
        };
        redrawAll();
        updatePlacementOverlay();
      };
      img.src = reader.result as string;
    };
    reader.readAsDataURL(file);
    importInput.value = '';
  });

  function commitActiveImage(): void {
    if (!activeImage) return;
    items.push({
      type: 'image',
      img: activeImage.img,
      x: activeImage.x,
      y: activeImage.y,
      w: activeImage.w,
      h: activeImage.h,
    });
    activeImage = null;
    redrawAll();
    updatePlacementOverlay();
  }

  function cancelActiveImage(): void {
    if (!activeImage) return;
    activeImage = null;
    redrawAll();
    updatePlacementOverlay();
  }

  document.getElementById('imgCommit')?.addEventListener('click', (e: Event) => {
    e.stopPropagation();
    commitActiveImage();
  });

  document.getElementById('imgCancel')?.addEventListener('click', (e: Event) => {
    e.stopPropagation();
    cancelActiveImage();
  });

  function updatePlacementOverlay(): void {
    if (!activeImage) {
      imgOverlay.hidden = true;
      return;
    }

    imgOverlay.hidden = false;
    const scaleX = canvas.offsetWidth / LOGICAL_W;
    const scaleY = canvas.offsetHeight / LOGICAL_H;
    const left = canvas.offsetLeft + activeImage.x * scaleX;
    const top = canvas.offsetTop + activeImage.y * scaleY;
    const w = activeImage.w * scaleX;
    const h = activeImage.h * scaleY;

    imgBox.style.left = left + 'px';
    imgBox.style.top = top + 'px';
    imgBox.style.width = w + 'px';
    imgBox.style.height = h + 'px';
    imgToolbar.style.left = left + 'px';
    imgToolbar.style.top = top + 'px';
  }

  window.addEventListener('resize', updatePlacementOverlay);

  let placeDrag: any = null;

  imgBox.addEventListener('pointerdown', (e: PointerEvent) => {
    if ((e.target as HTMLElement).classList.contains('handle')) return;
    e.stopPropagation();
    placeDrag = { mode: 'move', startX: e.clientX, startY: e.clientY, orig: Object.assign({}, activeImage) };
    imgBox.setPointerCapture(e.pointerId);
  });

  imgBox.querySelectorAll('.handle').forEach((handle: Element) => {
    handle.addEventListener('pointerdown', (e: PointerEvent) => {
      e.stopPropagation();
      placeDrag = {
        mode: 'resize',
        dir: (handle as HTMLElement).dataset.dir,
        startX: e.clientX,
        startY: e.clientY,
        orig: Object.assign({}, activeImage),
      };
      handle.setPointerCapture(e.pointerId);
    });
  });

  document.addEventListener('pointermove', (e: PointerEvent) => {
    if (!placeDrag || !activeImage) return;
    const scaleX = canvas.offsetWidth / LOGICAL_W;
    const scaleY = canvas.offsetHeight / LOGICAL_H;
    const dx = (e.clientX - placeDrag.startX) / scaleX;
    const dy = (e.clientY - placeDrag.startY) / scaleY;
    const o = placeDrag.orig;

    if (placeDrag.mode === 'move') {
      activeImage.x = o.x + dx;
      activeImage.y = o.y + dy;
    } else {
      const aspect = o.w / o.h;
      const minSize = 20;
      let newW = o.w;
      let newH = o.h;
      let newX = o.x;
      let newY = o.y;

      switch (placeDrag.dir) {
        case 'se':
          newW = Math.max(minSize, o.w + dx);
          newH = newW / aspect;
          break;
        case 'nw':
          newW = Math.max(minSize, o.w - dx);
          newH = newW / aspect;
          newX = o.x + (o.w - newW);
          newY = o.y + (o.h - newH);
          break;
        case 'ne':
          newW = Math.max(minSize, o.w + dx);
          newH = newW / aspect;
          newY = o.y + (o.h - newH);
          break;
        case 'sw':
          newW = Math.max(minSize, o.w - dx);
          newH = newW / aspect;
          newX = o.x + (o.w - newW);
          break;
      }

      activeImage.w = newW;
      activeImage.h = newH;
      activeImage.x = newX;
      activeImage.y = newY;
    }

    redrawAll();
    updatePlacementOverlay();
  });

  document.addEventListener('pointerup', () => {
    placeDrag = null;
  });

  document.addEventListener('pointercancel', () => {
    placeDrag = null;
  });

  function openSaveModal(): void {
    fileNameInput.value = 'untitled';
    formatSelect.value = 'png';
    saveModal.hidden = false;
    setTimeout(() => {
      fileNameInput.focus();
      fileNameInput.select();
    }, 0);
  }

  document.getElementById('mi-save')?.addEventListener('click', openSaveModal);
  document.getElementById('saveCancel')?.addEventListener('click', () => {
    saveModal.hidden = true;
  });

  function buildExportCanvas(): HTMLCanvasElement {
    const c = document.createElement('canvas');
    c.width = LOGICAL_W;
    c.height = LOGICAL_H;
    const cx = c.getContext('2d') as CanvasRenderingContext2D;
    cx.lineCap = 'round';
    cx.lineJoin = 'round';
    cx.fillStyle = '#ffffff';
    cx.fillRect(0, 0, LOGICAL_W, LOGICAL_H);

    items.forEach((it) => {
      if (it.type === 'image') {
        cx.drawImage(it.img, it.x, it.y, it.w, it.h);
        return;
      }
      if (it.points.length < 2) return;
      cx.strokeStyle = it.color;
      cx.lineWidth = it.thickness;
      cx.beginPath();
      cx.moveTo(it.points[0].x, it.points[0].y);
      for (let i = 1; i < it.points.length; i++) {
        cx.lineTo(it.points[i].x, it.points[i].y);
      }
      cx.stroke();
    });
    return c;
  }

  function encodeBMP(imageData: ImageData, width: number, height: number): Blob {
    const rowSize = Math.floor((width * 3 + 3) / 4) * 4;
    const pixelArraySize = rowSize * height;
    const fileSize = 54 + pixelArraySize;
    const buffer = new ArrayBuffer(fileSize);
    const view = new DataView(buffer);

    view.setUint8(0, 0x42);
    view.setUint8(1, 0x4d);
    view.setUint32(2, fileSize, true);
    view.setUint32(6, 0, true);
    view.setUint32(10, 54, true);
    view.setUint32(14, 40, true);
    view.setInt32(18, width, true);
    view.setInt32(22, height, true);
    view.setUint16(26, 1, true);
    view.setUint16(28, 24, true);
    view.setUint32(30, 0, true);
    view.setUint32(34, pixelArraySize, true);
    view.setInt32(38, 2835, true);
    view.setInt32(42, 2835, true);
    view.setUint32(46, 0, true);
    view.setUint32(50, 0, true);

    const data = imageData.data;
    let offset = 54;
    for (let y = height - 1; y >= 0; y--) {
      for (let x = 0; x < width; x++) {
        const idx = (y * width + x) * 4;
        view.setUint8(offset++, data[idx + 2]);
        view.setUint8(offset++, data[idx + 1]);
        view.setUint8(offset++, data[idx]);
      }
      const pad = rowSize - width * 3;
      for (let p = 0; p < pad; p++) {
        view.setUint8(offset++, 0);
      }
    }
    return new Blob([buffer], { type: 'image/bmp' });
  }

  function ensureName(name: string): string {
    const clean = (name || 'untitled').trim();
    return clean.length ? clean.replace(/\.[a-zA-Z0-9]+$/, '') : 'untitled';
  }

  function downloadBlob(blob: Blob, filename: string): void {
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    a.remove();
    setTimeout(() => URL.revokeObjectURL(url), 2000);
  }

  function doSave(): void {
    const base = ensureName(fileNameInput.value);
    const fmt = formatSelect.value;
    const exportC = buildExportCanvas();

    if (fmt === 'png') {
      exportC.toBlob((blob) => {
        if (blob) downloadBlob(blob, base + '.png');
      }, 'image/png');
    } else if (fmt === 'jpeg') {
      exportC.toBlob((blob) => {
        if (blob) downloadBlob(blob, base + '.jpg');
      }, 'image/jpeg', 0.92);
    } else {
      const cx = exportC.getContext('2d') as CanvasRenderingContext2D;
      const imgData = cx.getImageData(0, 0, LOGICAL_W, LOGICAL_H);
      const blob = encodeBMP(imgData, LOGICAL_W, LOGICAL_H);
      downloadBlob(blob, base + '.bmp');
    }

    saveModal.hidden = true;
  }

  document.getElementById('saveConfirm')?.addEventListener('click', doSave);
  fileNameInput.addEventListener('keydown', (e: KeyboardEvent) => {
    if (e.key === 'Enter') {
      e.preventDefault();
      doSave();
    }
    e.stopPropagation();
  });

  document.getElementById('mi-about')?.addEventListener('click', () => {
    aboutModal.hidden = false;
  });

  document.getElementById('aboutClose')?.addEventListener('click', () => {
    aboutModal.hidden = true;
  });

  document.addEventListener('keydown', (e: KeyboardEvent) => {
    const typing = !!(e.target && (e.target instanceof HTMLInputElement || e.target instanceof HTMLSelectElement || e.target instanceof HTMLTextAreaElement));
    const ctrl = e.ctrlKey || e.metaKey;
    const key = e.key.toLowerCase();

    if (activeImage && !typing) {
      if (key === 'enter') {
        e.preventDefault();
        commitActiveImage();
        return;
      }
      if (key === 'escape') {
        e.preventDefault();
        cancelActiveImage();
        return;
      }
    }

    if (ctrl && key === 'z') {
      e.preventDefault();
      undo();
    } else if (ctrl && key === 's') {
      e.preventDefault();
      openSaveModal();
    } else if (ctrl && key === 'e') {
      e.preventDefault();
      setEraser(true);
    } else if (ctrl && key === 'p') {
      e.preventDefault();
      setEraser(false);
    } else if (ctrl && key === 'i') {
      e.preventDefault();
      importInput.click();
    } else if (!ctrl && !typing && key === 'c') {
      colorInput.click();
    }
  });

  clearCanvas();
  updateModeUI();
})();
