{ inputs, ... }:

{
  imports = [
    inputs.devshell.flakeModule
  ];
  perSystem =
    {
      pkgs,
      ...
    }:
    {
      devshells.default = {
        packages = with pkgs; [
          clang_18
          clang-tools_18
          uhd.dev
          soapyuhd
          soapysdr-with-plugins
          meson
          mesonlsp
          include-what-you-use
        ];
      };
    };
}
