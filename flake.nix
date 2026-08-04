{
  description = "Terminal-based ASCII 3D model viewer";

  inputs = {
    nixpkgs.url = "https://github.com/NixOS/nixpkgs/archive/643809054d65fdd466a63e3155b8c498cb483c04.tar.gz";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ self.overlays.default ];
        };
      in
      {
        packages = {
          inherit (pkgs) voxcii;
          default = pkgs.voxcii;
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ pkgs.voxcii ];
        };
      }
    )
    // {
      overlays.default = final: prev: {
        voxcii = (prev.callPackage ./pkgs/voxcii.nix { }).overrideAttrs {
          src = self;
        };
      };
    };
}
