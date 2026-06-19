{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    nixgl.url = "github:nix-community/nixGL";
    nixgl.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { nixpkgs, nixgl, ... }:
    let
      pkgs = import nixpkgs { system = "x86_64-linux"; overlays = [ nixgl.overlay ]; };
    in
    {
      devShells.x86_64-linux.default = pkgs.mkShell {
        nativeBuildInputs = with pkgs; [
          cmake
          pkg-config
          gcc
          clang-tools
          wayland-scanner
        ];

        buildInputs = with pkgs; [
          libGL
          mesa

          # imgui needs these to compile :(
          libx11
          libxrandr

          wayland
          wayland-protocols
          libffi
          libxkbcommon
          libdecor

          assimp

          doxygen
          graphviz

          python3

          # OpenGL wrapper for non-NixOS
          pkgs.nixgl.auto.nixGLDefault
        ];

        LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
          pkgs.libGL
          pkgs.wayland
          pkgs.libxkbcommon
          pkgs.libdecor
        ];

      };
    };
}
