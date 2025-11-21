{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.stdenv.cc         # incluye gcc + glibc + todo
    pkgs.pkg-config
    pkgs.gtk4
    pkgs.gtkmm4
    pkgs.meson
    pkgs.ninja
  ];

  # Opcional: añadir flags de compilación si necesitas incluir algo explícito
  # NIX_CFLAGS_COMPILE = [ "-I${pkgs.glibc.dev}/include" ];
}

