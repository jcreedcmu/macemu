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
              unpackPhase = "true";

              buildPhase = ''
true
              '';

              configurePhase = "true";

              installPhase =
                ''
                # mkdir -p $out/bin
                # cp $src/hello-flake $out/bin/hello-flake
                # chmod +x $out/bin/hello-flake
                mkdir -p $out/files
                echo ${ builtins.toString (builtins.attrNames Retro68.packages.${system}.standalone) } > $out/files/outputfile
              '';
            };
            default = twelf-bin;
          };
        }
    )
  );
}
