{
  description = "Twelf on Classic Mac";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    Retro68.url = "github:jcreedcmu/Retro68/mlton";
    flake-utils.url = "github:numtide/flake-utils";
    mlton-src = { url = "github:agoode/mlton/mac"; flake = false; };
  };


  outputs = { self, nixpkgs, Retro68, flake-utils, mlton-src }: (
    with builtins;
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
          pkgs-retro68 = import nixpkgs { inherit system; overlays = [ Retro68.overlays.default ]; };
          retro68 = pkgs-retro68.retro68.monolithic;
      in
        {
          packages = rec {
            gmp-m68k = Retro68.legacyPackages.${system}.pkgsCross.m68k.gmp.dev;
            mlton = import ./nix/mlton.nix { inherit pkgs mlton-src; }; # mlton with some more patches
            mlton-m68k-runtime = import ./nix/mlton-m68k-runtime.nix { inherit pkgs gmp-m68k mlton-src retro68; };
            twelf-bin = import ./nix/twelf-bin.nix { inherit pkgs retro68; };
            default = twelf-bin;
          };
        }
    )
  );
}
