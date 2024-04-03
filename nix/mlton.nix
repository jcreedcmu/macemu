{ pkgs, mlton-src }:
pkgs.stdenv.mkDerivation {
  name = "mlton-m68k";
  src = mlton-src;

  buildInputs = [
    pkgs.mlton
    pkgs.gmp
  ];

  patchPhase = ''
  patchShebangs bin/host-arch
  patchShebangs bin/host-os
  patchShebangs bin/platform
  patchShebangs bin/clean
  patchShebangs mlnlffigen/gen-cppcmd
  patchShebangs bin/find-ignore
  '';

  buildPhase = ''
  make all
  '';

  installPhase = ''
  make PREFIX=$out install
  '';
}
