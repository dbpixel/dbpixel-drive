// Imagens do instalador (NSIS/MUI2): lateral das páginas de boas-vindas e conclusão (164x314) e cabeçalho (150x57).
// Uso: node gerar-imagens.js  (precisa do playwright; depois converta os PNG em BMP de 24 bits com converter-bmp.ps1)
const { chromium } = require('playwright');
const path = require('path');
const fs = require('fs');

const fonts = path.join(__dirname, '..', '..', 'src', 'gui', 'fonts');
const font = (f) => 'data:font/ttf;base64,' + fs.readFileSync(path.join(fonts, f)).toString('base64');
const TILE = (c) => `<svg width="100%" height="100%" viewBox="0 0 24 24"><polygon points="0,0 18.7,0 24,5.3 24,24 5.3,24 0,18.7" fill="${c}"/><path d="M7.4 16.6h9.2a3.3 3.3 0 0 0 .2-6.6 4.7 4.7 0 0 0-9 1.1 2.8 2.8 0 0 0-.4 5.5z" fill="#fff"/></svg>`;
const css = `@font-face{font-family:M;font-weight:800;src:url(${font('Montserrat-800.ttf')})}@font-face{font-family:M;font-weight:600;src:url(${font('Montserrat-600.ttf')})}
*{margin:0;box-sizing:border-box}body{width:var(--w);height:var(--h);overflow:hidden;font-family:M}`;

const side = `<style>${css}
.s{position:relative;width:164px;height:314px;background:#041F46;overflow:hidden}
.c1{position:absolute;right:-46px;top:-46px;width:92px;height:92px;background:#6F8BB8;transform:rotate(45deg)}
.c2{position:absolute;left:-40px;bottom:-40px;width:80px;height:80px;background:#0B2A57;transform:rotate(45deg)}
.m{position:absolute;left:22px;top:118px;display:flex;align-items:center;gap:10px}.t{width:34px;height:34px}
.m b{font-weight:800;font-size:23px;color:#fff;letter-spacing:-.01em}
.e{position:absolute;left:22px;top:166px;font-weight:600;font-size:8.5px;letter-spacing:.16em;color:#9DB6DC}.e b{color:#fff}</style>
<div class="s"><i class="c1"></i><i class="c2"></i><div class="m"><span class="t">${TILE('#6F8BB8')}</span><b>Drive</b></div><div class="e">UM PRODUTO <b>DBPIXEL</b></div></div>`;

const head = `<style>${css}
.h{width:150px;height:57px;background:#fff;display:flex;align-items:center;justify-content:flex-end;padding-right:10px;gap:8px}
.t{width:26px;height:26px}.h b{font-weight:800;font-size:18px;color:#0A1222}</style>
<div class="h"><span class="t">${TILE('#6F8BB8')}</span><b>Drive</b></div>`;

(async () => {
  const b = await chromium.launch();
  for (const [name, html, w, h] of [['lateral', side, 164, 314], ['cabecalho', head, 150, 57]]) {
    const p = await b.newPage({ viewport: { width: w, height: h }, deviceScaleFactor: 1 });
    await p.setContent(`<html style="--w:${w}px;--h:${h}px"><body>${html}</body></html>`);
    await p.evaluate(() => document.fonts.ready);
    await p.screenshot({ path: path.join(__dirname, `${name}.png`) });
  }
  await b.close();
  console.log('ok');
})();
