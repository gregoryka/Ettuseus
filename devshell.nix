{ inputs, ... }:

{
  imports = [
    inputs.devshell.flakeModule
  ];
  perSystem =
    {
      self',
      pkgs,
      ...
    }:
    {
      devshells.default = {

        packagesFrom = [ self'.packages.default ];

        packages = with pkgs; [
          clang-tools_18
          mesonlsp
          include-what-you-use
        ];
      };
    };
}
