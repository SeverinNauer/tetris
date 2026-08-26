{ pkgs ? import <nixpkgs> {} }:

let
  nixgl = import (builtins.fetchTarball "https://github.com/nix-community/nixGL/archive/main.tar.gz") {};
in
pkgs.mkShell {
  packages = with pkgs; [
    gcc
    pkg-config
    cmake
    gdb
    clang-tools

    glfw
    wayland

    # X11
    libx11
    libx11.dev
    libxcursor
    libxi
    libxinerama
    libxrandr

    nixgl.auto.nixGLDefault
  ];

  LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
    pkgs.wayland
    pkgs.libxkbcommon
    pkgs.alsa-lib
  ];
}
