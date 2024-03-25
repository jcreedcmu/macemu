const fs = require('fs');
const palette = require('./palette');

function iconsOfPpm(ppmFile) {
  const toks = fs.readFileSync('/tmp/twelf-icon.ppm', 'utf8')
		  .split('\n')
		  .map(x => x.replace(/#.*/, ''))
		  .filter(x => x.match(/\S/) && !x.match("#"))
		  .join(' ').split(/\s+/)
		  .filter(x => x.match(/\S/));
  const width = toks[1];
  const height = toks[2];
  toks.splice(0,4); // P3 W H C
  const nums = toks.map(x => parseInt(x));

  const img = [];
  const mask = [];
  const ic8Bytes = [];
  const ic4Nybs = [];
  const ic1Bits = [];
  const expandMask = [];

  for (let x = 0; x < width; x++) {
	 for (let y = 0; y < height; y++) {
		const ix = y * width + x;
		img[ix] = [nums[ix * 3], nums[ix * 3+1], nums[ix*3+2]];
	 }
  }

  // cheating a bit with the color choices to match the icon as I've drawn it:
  palette.clut4[8] = [17*6, 17*9, 17*6];
  palette.clut4[9] = [11, 49, 10];

  img.forEach((pixel, ix) => {
	 // interpret cyan as transparent
	 if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 255) {
		ic8Bytes.push(0);
		ic4Nybs.push(0);
		mask[ix] = 0;
	 }
	 else {
		ic8Bytes.push(palette.getIndex(pixel, palette.clut8));
		ic4Nybs.push(palette.getIndex(pixel, palette.clut4));
		mask[ix] = 1;
	 }
	 ic1Bits.push((pixel[0] + pixel[1] + pixel[2] > 3*128) ? 0 : 1);
  });

  function pixelAt(x, y) {
	 return (x >= 0 && x < width && y >= 0 && y < height) ? mask[width * y + x] : 0;
  }

  // expand mask by one pixel all around
  for (let x = 0; x < width; x++) {
	 for (let y = 0; y < height; y++) {
		expandMask[width * y + x] = (pixelAt(x, y) || pixelAt(x+1,y) || pixelAt(x,y+1) || pixelAt(x-1,y) || pixelAt(x,y-1)) ? 1 : 0;
	 }
  }

  return {
	 pixels:img, width, height, mask: expandMask, ic8Bytes, ic4Nybs, ic1Bits
  };
}

const largeIcon = iconsOfPpm('/tmp/twelf-icon.ppm');
const smallIcon = iconsOfPpm('/tmp/twelf-icon-small.ppm');

function bytesOfBits(bits) {
  const bytes = [];
  for (let i = 0; i < bits.length / 8; i++) {
	 let b = 0;
	 for (let j = 0; j < 8; j++) {
		b = 2 * b + bits[i * 8 + j];
	 }
	 bytes.push(b);
  }
  return bytes;
}

function bytesOfNybbles(nybbles) {
  const bytes = [];
  for (let i = 0; i < nybbles.length / 2; i++) {
	 let b = 0;
	 for (let j = 0; j < 2; j++) {
		b = 16 * b + nybbles[i * 2 + j];
	 }
	 bytes.push(b);
  }
  return bytes;
}

function hexOfByte(x) {
  let s = x.toString(16);
  while (s.length < 2)
	 s = '0' + s;
  return s;
}

function rezOfBytes(bytes) {
  return bytes.map(x => {
	 const str = hexOfByte(x);
	 return `$"${str}"`
  }).join(" ");
}

console.log(`
resource 'BNDL' (128) {
  'TWLF', 0;
  {
    'FREF', { 0, 128 };
    'ICN#', { 0, 128 };
  }
};

resource 'FREF' (128) {
  'APPL', 0, "";
};
`);

console.log(`
resource 'ICN#' (128) {
  {
    ${rezOfBytes(bytesOfBits(largeIcon.ic1Bits))},
    ${rezOfBytes(bytesOfBits(largeIcon.mask))},
  }
};

resource 'icl8' (128) {
  ${rezOfBytes(largeIcon.ic8Bytes)}
};

resource 'icl4' (128) {
  ${rezOfBytes(bytesOfNybbles(largeIcon.ic4Nybs))}
};

`);
