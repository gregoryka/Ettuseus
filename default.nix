{
  stdenv,
  uhd,
  fmt,
  meson,
  ninja,
  pkg-config,
  boost,
  python312Packages,
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
    python312Packages.pybind11
    uhd
  ];

}
