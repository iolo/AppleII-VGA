import fs from 'fs';
const srcfile = process.argv[2] || 'han_wcomm.fnt';
const outfile = process.argv[3] || 'cvt_' + srcfile;
const src = new Uint8Array(fs.readFileSync(srcfile));
const out = new Uint8Array(src.length);
for (let i = 0; i < src.length; i+= 32) {
  for (let j = 0; j < 16; j++) {
    // abcdefgh ijklmnop --> 0abcdefg 0hijklmn
    out[i + j] = (src[i + j] >> 1) & 0b01111111;
    out[i + j + 16] = ((src[i + j] << 6) & 0b01000000) | ((src[i + j + 16] >> 2) & 0b00111111);
  }
}
fs.writeFileSync(outfile, Buffer.from(out.buffer));

