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
for (let x = 0; x < 32; x++) {
  for (let y = 0; y < 32; y++) {
	 const ix = y * 32 + x;
	 img[ix] = [nums[ix * 3], nums[ix * 3+1], nums[ix*3+2]];
  }
}

console.log(`
resource 'ICN#' (128) {
  {
  	 $"FFFF FFFF 0000 0000 FFFF FFFF 0000 0000", // actual icon
    $"EEEE EEEE EEEE EEEE EEEE EEEE EEEE EEEE"  // mask
  }
};
`);

console.log(`resource 'icl8' (128) {`);
img.forEach(pixel => {
  if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 255) {
	 console.log('$"00"');
  }
  else {
	 console.log('$"FF"');
  }
});
console.log(`};`);
