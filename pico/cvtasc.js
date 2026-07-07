import fs from 'fs';
const srcfile = process.argv[2] || 'asc_wcomm.fnt';
const outfile = process.argv[3] || 'cvt_' + srcfile;
const src = new Uint8Array(fs.readFileSync(srcfile));
const out = new Uint8Array(src.length);
for (let i = 0; i < src.length; i++) {
  // abcdefgh -> 0abcdefg
  out[i] = src[i] >> 1;
}
fs.writeFileSync(outfile, Buffer.from(out.buffer));

