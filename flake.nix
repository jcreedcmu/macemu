{
  description = "Twelf on Classic Mac";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    Retro68.url = "github:agoode/Retro68/mlton";
    flake-utils.url = "github:numtide/flake-utils";
  };


  outputs = { self, nixpkgs, Retro68, flake-utils }: (
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
      in
        {
          packages = rec {
            mlton-m68k = pkgs.stdenv.mkDerivation {
              name = "mlton-m68k";
              src = builtins.fetchGit {
                url = "https://github.com/agoode/mlton.git";
                rev = "8401144962437491018343888a3eaa20b1b8fe37";
                ref = "mac";
              };

              buildInputs = [
                # Retro68.packages.${system}.standalone
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
