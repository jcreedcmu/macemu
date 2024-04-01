// set-bits.js
//
// Modifies some fixed-offset bits in MacBinary files to set the HFS
// flags to "not inited" and "has a BNDL resource". It's not clear if
// this is necessary to get application icons to show up correctly.

const fs = require('fs');

const filePath = process.argv[2];
const file = fs.readFileSync(filePath);

// Byte offset in .bin format where HFS flags are stored
// http://files.stairways.com/other/macbinaryii-standard-info.txt
const FLAGS = 73;
const FLAG_BUNDLE = 1 << 5;
const FLAG_INITED = 1 << 0;

file[FLAGS] = (file[FLAGS] | FLAG_BUNDLE) & ~FLAG_INITED;

fs.writeFileSync(filePath, file);
