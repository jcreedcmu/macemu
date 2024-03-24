// resource 'ICN#' (128) {
//   {
//   	 $"FFFF FFFF 0000 0000 FFFF FFFF 0000 0000", // actual icon
//     $"EEEE EEEE EEEE EEEE EEEE EEEE EEEE EEEE"  // mask
//   }
// };

// resource 'icl8' (128) {
//   	 $"FFFF FFFF 0000 0000 FFFF FFFF 0000 0000"
// };

// Color palette information here:
// http://belkadan.com/blog/2018/01/Color-Palette-8/

const fs = require('fs');
const toks = fs.readFileSync('/tmp/twelf-icon.ppm', 'utf8')
		.split('\n')
		.map(x => x.replace(/#.*/, ''))
		.filter(x => x.match(/\S/) && !x.match("#"))
		.join(' ').split(/\s+/)
		.filter(x => x.match(/\S/));
toks.splice(0,4); // P3 W H C
const nums = toks.map(x => parseInt(x));

const img = [];
const mask = [];

for (let x = 0; x < 32; x++) {
  for (let y = 0; y < 32; y++) {
	 const ix = y * 32 + x;
	 img[ix] = [nums[ix * 3], nums[ix * 3+1], nums[ix*3+2]];
  }
}

const icl8Bytes = [];
img.forEach((pixel, ix) => {
  // interpret cyan as transparent
  if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 255) {
	 icl8Bytes.push(0);
	 mask[ix] = 0;
  }
  else {
	 icl8Bytes.push(0xff); // XXX look up best color
	 mask[ix] = 1;
  }
});

function hexOfByte(x) {
  let s = x.toString(16);
  while (s.length < 2)
	 s = '0' + s;
  return s;
}

const icl8Body = icl8Bytes.map(x => {
  const str = hexOfByte(x);
  return `$"${str}"`
}).join(" ");

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

const bwData = `$"FFFF FFFF 0000 0000 FFFF FFFF 0000 0000"`;
const maskData = `$"EEEE EEEE EEEE EEEE EEEE EEEE EEEE EEEE"`;
console.log(`
resource 'ICN#' (128) {
  {
  	 ${bwData},
    ${maskData},
  }
};

resource 'icl8' (128) {
  ${icl8Body}
};
`);
