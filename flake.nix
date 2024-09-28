{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    { flake-parts, ... }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [
        ./devshell.nix
        ./formatter.nix
      ];
      systems = [ "x86_64-linux" ];

      perSystem =
        { pkgs, ... }:
        {

          packages.default = pkgs.callPackage ./default.nix { };
        };
    };
}
