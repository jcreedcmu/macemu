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
          retro68 = Retro68.packages.${system}.standalone;
      in
        {
          packages = rec {
            gmp-m68k = Retro68.legacyPackages.${system}.pkgsCross.m68k.gmp.dev;
            mlton-m68k-bin = import ./nix/mlton-m68k-bin.nix { inherit pkgs mlton-src; };
            mlton-m68k-runtime = import ./nix/mlton-m68k-runtime.nix { inherit pkgs gmp-m68k mlton-src retro68; };
            twelf-bin = import ./nix/twelf-bin.nix { inherit pkgs retro68; };
            default = twelf-bin;
          };
        }
    )
  );
}
