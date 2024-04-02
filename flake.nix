{
  description = "Twelf on Classic Mac";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    Retro68.url = "github:autc04/Retro68";
    flake-utils.url = "github:numtide/flake-utils";
  };


  outputs = { self, nixpkgs, Retro68, flake-utils }: (
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
      in
        {
          packages = rec {
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
