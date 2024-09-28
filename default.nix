{
  stdenv,
  uhd,
  fmt,
  meson,
  ninja,
  pkg-config,
  boost,
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
  ];

  buildInputs = [
    boost
    fmt
    uhd
  ];

}
