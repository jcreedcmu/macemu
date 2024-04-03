{
  description = "Twelf on Classic Mac";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    Retro68.url = "github:agoode/Retro68/mlton";
    flake-utils.url = "github:numtide/flake-utils";
    mlton-src = { url = "github:agoode/mlton/mac"; flake = false; };
  };


  outputs = { self, nixpkgs, Retro68, flake-utils, mlton-src }: (
    with builtins;
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
          mlton-m68k-deriv = pkgs.stdenv.mkDerivation {
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
          };
          gmp-m68k = Retro68.legacyPackages.${system}.pkgsCross.m68k.gmp.dev;
      in
        {
          packages = rec {
            mlton-m68k = mlton-m68k-deriv;
            mlton-m68k-runtime = pkgs.stdenv.mkDerivation {
              name = "mlton-m68k-runtime";
              src = builtins.fetchGit {
                url = "https://github.com/agoode/mlton.git";
                rev = "8401144962437491018343888a3eaa20b1b8fe37";
                ref = "mac";
              };

              buildInputs = [
                mlton-m68k
                gmp-m68k
                Retro68.packages.${system}.standalone
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
            };

            twelf-bin = pkgs.stdenv.mkDerivation rec {
              name = "twelf-bin";
              src = ./.;

              buildInputs = [
                Retro68.packages.${system}.standalone
              ];

              buildPhase = ''
              cd docker/app
              make -f Makefile.nix
              '';

              configurePhase = "true";

              installPhase = ''
              mkdir -p $out
              '';
            };
            default = twelf-bin;
          };
        }
    )
  );
}
