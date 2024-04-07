function hexOfByte(x) {
  let s = x.toString(16);
  while (s.length < 2)
	 s = '0' + s;
  return s;
}
exports.hexOfByte = hexOfByte;

function rezOfBytes(bytes) {
  return bytes.map(x => {
	 const str = hexOfByte(x);
	 return `$"${str}"`
  }).join(" ");
}
exports.rezOfBytes = rezOfBytes;
