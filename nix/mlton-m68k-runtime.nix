{ pkgs, gmp-m68k, retro68, mlton-src }:

pkgs.stdenv.mkDerivation {
  name = "mlton-m68k-runtime";
  src = mlton-src;

  buildInputs = [
    gmp-m68k
    retro68
  ];

  configurePhase = "true";

  patchPhase = ''
  patchShebangs bin/host-arch
  patchShebangs bin/host-os
  patchShebangs bin/platform
  patchShebangs bin/clean
  patchShebangs mlnlffigen/gen-cppcmd
  patchShebangs bin/find-ignore
  '';

  buildPhase = ''
  make \
      CC=gcc \
      AR=gcc-ar \
      RANLIB=gcc-ranlib \
      USE_PREGEN=true \
      TARGET_OS=macos \
      TARGET_ARCH=m68k \
      TARGET=m68k-apple-macos \
      WITH_GMP_DIR=${ gmp-m68k } \
      dirs runtime
  '';

  installPhase = ''
  make \
      TARGET_OS=macos \
      TARGET_ARCH=m68k \
      TARGET=m68k-apple-macos \
      PREFIX=$out install-runtime
  '';
}
