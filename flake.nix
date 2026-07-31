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

          libx11
          libxrandr
          libxinerama
          libxcursor
          libxi

          wayland
          wayland-protocols
          libffi
          libxkbcommon
          libdecor

          assimp

          doxygen
          graphviz

          (python3.withPackages (ps: [ ps.libclang ]))

          # OpenGL wrapper for non-NixOS
          pkgs.nixgl.auto.nixGLDefault
        ];

        LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
          pkgs.libGL
          pkgs.wayland
          pkgs.libxkbcommon
          pkgs.libdecor
          pkgs.libx11
          pkgs.libxrandr
          pkgs.libxcursor
          pkgs.libxi
        ];

        shellHook = ''
          export SHELL=$(getent passwd $USER | cut -d: -f7)
          exec $SHELL
        '';
      };
    };
}
