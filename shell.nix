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

    raylib
    emscripten

    # Wayland
    wayland
    wayland-protocols
    libxkbcommon

    # OpenGL/EGL tools
    mesa
    mesa-demos

    nixgl.auto.nixGLDefault
  ];
}
