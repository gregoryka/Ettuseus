{
  stdenv,
  uhd,
  fmt,
  clang_18,
  meson,
  ninja,
  pkg-config,
  ...
}:

stdenv.mkDerivation {
  name = "Ettuseus";
  src = ./.;
  outputs = [
    "out"
    "dev"
  ];

  mesonBuildType = "release";

  nativeBuildInputs = [
    pkg-config
    meson
    ninja
    clang_18
  ];

  buildInputs = [
    uhd.dev
    fmt
  ];

}
