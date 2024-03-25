// 4-bit palette extracted from
// https://en.wikipedia.org/wiki/List_of_software_palettes#Apple_Macintosh_default_16-color_palette
const clut4 = [
  [255, 255, 255],
  [252, 244, 0],
  [255, 100, 0],
  [221, 2, 2],
  [241, 2, 133],
  [70, 0, 166],
  [0, 0, 213],
  [0, 174, 233],
  [26, 185, 12],
  [0, 100, 8],
  [88, 40, 0],
  [145, 113, 53],
  [193, 193, 193],
  [129, 129, 129],
  [62, 62, 62],
  [0, 0, 0],
];

// 8-bit palette generation code derived from this pseudocode:
// http://belkadan.com/blog/2018/01/Color-Palette-8/

function clut8Color(x) { // x a byte value
  if (x < 215) {
    // Component-based colors, with RGB values in the range 0...5,
    // stored in reverse order (i.e. color #0 is white, (1.0, 1.0, 1.0)).
    // Note that x == 215 would normally produce black, (0.0, 0.0, 0.0),
    // but the paconstte deliberately puts that at the end.
    const red   = (5 - (x / 36))    / 5.0;
    const green = (5 - (x / 6 % 6)) / 5.0;
    const blue  = (5 - (x % 6))     / 5.0;
    return {red, green, blue};

  } else if (x == 255) {
    // Special case: black is last.
    return {red:0,green:0,blue:0};

  } else {
    // Extra shades of "primary" colors: red, green, blue, and grey.

	 const values = [14, 13, 11, 10,  8, 7, 5, 4, 2, 1].map(x => x/15);
    const which = (x - 215) % 10;
    switch (Math.floor((x - 215) / 10)) {
    case 0: return {red: values[which], green: 0.0, blue: 0.0};
    case 1: return {red: 0.0, green: values[which], blue: 0.0};
    case 2: return {red: 0.0, green: 0.0, blue: values[which]};
    case 3: return {red:   values[which],
                    green: values[which],
                    blue:  values[which]};
    default: throw new Error("x must be out of range")
    }
  }
}

function getClut8() {
  const clut = [];
  for (let i = 0; i < 256; i++) {
	 const c = clut8Color(i);
	 clut[i] = [c.red, c.green, c.blue].map(x => Math.floor(255*x));
  }
  return clut;
}

const clut8 = getClut8();

function sq(x) { return x*x; }

function compare(c1, c2) {
  return sq(c1[0]-c2[0]) + sq(c1[1]-c2[1]) + sq(c1[2]-c2[2]);
}

function getIndex(color, clut) {
  let bestScore = 1e16;
  let bestIx = -1;
  for (let i = 0; i < clut.length; i++) {
	 const score = compare(color, clut[i]);
	 if (score < bestScore) {
		bestScore = score;
		bestIx = i;
	 }
  }
  return bestIx;
}

exports.getIndex = getIndex;
exports.clut4 = clut4;
exports.clut8 = clut8;
