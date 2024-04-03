{ pkgs, retro68 }:
pkgs.stdenv.mkDerivation rec {
  name = "twelf-bin";
  src = ./.;

  buildInputs = [
    retro68
  ];

  buildPhase = ''
  cd docker/app
  make -f Makefile.nix
  '';

  configurePhase = "true";

  installPhase = ''
  mkdir -p $out
  '';
}
