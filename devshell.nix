{ inputs, ... }:

{
  perSystem =
    {
      self',
      pkgs,
      ...
    }:
    {
      devShells.default = pkgs.mkShell {

        inputsFrom = [ self'.packages.default ];

        packages = with pkgs; [
          clang-tools_18
          mesonlsp
          include-what-you-use
        ];
      };
    };
}
