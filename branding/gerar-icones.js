// Gera os ícones do Drive (um produto DBPixel) seguindo o manual da marca, seções 2.8, 2.9 e 9.2:
// quadrado chanfrado azul-aço #6F8BB8 (cortes de 45°, 22% do lado, em cima à direita e embaixo à esquerda) com nuvem branca.
// Uso: node branding/gerar-icones.js  (precisa de sharp e playwright no npm global, com NODE_PATH apontando para ele)
const sharp = require('sharp');
const { chromium } = require('playwright');
const fs = require('fs');
const path = require('path');

const OUT = path.join(__dirname, 'theme');
const NAME = 'drive';
const STEEL = '#6F8BB8';
// fundo do ícone do programa (bandeja, menu Iniciar, instalador, Explorer): navy da marca, pedido do dono em 28/10/2026.
// A marca dentro das telas continua azul-aço, porque ali ela fica sobre a faixa navy.
const NAVY = '#041F46';
const CLOUD = '<path d="M7.4 16.6h9.2a3.3 3.3 0 0 0 .2-6.6 4.7 4.7 0 0 0-9 1.1 2.8 2.8 0 0 0-.4 5.5z" fill="#FFFFFF"/>';
const tile = (size, fill = NAVY) =>
  `<svg xmlns="http://www.w3.org/2000/svg" width="${size}" height="${size}" viewBox="0 0 24 24">` +
  `<polygon points="0,0 18.7,0 24,5.3 24,24 5.3,24 0,18.7" fill="${fill}"/>${CLOUD}</svg>`;

const png = (size) => sharp(Buffer.from(tile(size)), { density: 144 }).resize(size, size).png().toBuffer();

const svgWithImage = (w, h, buf) =>
  `<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="${w}" height="${h}" viewBox="0 0 ${w} ${h}">` +
  `<image width="${w}" height="${h}" xlink:href="data:image/png;base64,${buf.toString('base64')}"/></svg>\n`;

// marca do assistente: quadrado + "Drive" em Montserrat 800 branco, sobre o cabeçalho navy (renderizado com a fonte da marca)
async function wizardMark() {
  const browser = await chromium.launch();
  const page = await browser.newPage({ deviceScaleFactor: 3 });
  await page.setContent(`<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Montserrat:wght@800&display=swap">
    <div id="m" style="display:inline-flex;align-items:center;gap:12px;padding:2px">${tile(40, STEEL)}
    <b style="font:800 34px/1 Montserrat,sans-serif;color:#FFFFFF;letter-spacing:-.01em">Drive</b></div>`, { waitUntil: 'networkidle' });
  await page.evaluate(() => document.fonts.ready);
  const buf = await page.locator('#m').screenshot({ omitBackground: true });
  await browser.close();
  return buf;
}

(async () => {
  fs.rmSync(OUT, { recursive: true, force: true });
  fs.mkdirSync(path.join(OUT, 'colored'), { recursive: true });
  fs.mkdirSync(path.join(OUT, 'universal'), { recursive: true });
  for (const s of [16, 24, 32, 48, 64, 128, 256, 512, 1024]) {
    fs.writeFileSync(path.join(OUT, 'colored', `${s}-${NAME}-icon.png`), await png(s));
  }
  // lateral do Explorer: o mesmo quadrado, que já é a marca do produto
  for (const s of [16, 32, 64, 128, 256, 512, 1024]) {
    fs.writeFileSync(path.join(OUT, 'colored', `${s}-${NAME}-sidebar.png`), await png(s));
  }
  fs.writeFileSync(path.join(OUT, 'universal', `${NAME}-icon.svg`), tile(512) + '\n');
  const mark = await wizardMark();
  const m = await sharp(mark).metadata();
  fs.writeFileSync(path.join(OUT, 'universal', 'wizard_logo.svg'), svgWithImage(Math.round(m.width / 3), Math.round(m.height / 3), mark));
  console.log('ícones gerados em', OUT);
})().catch((e) => { console.error(e); process.exit(1); });
