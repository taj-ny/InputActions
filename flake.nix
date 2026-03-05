{
  description = "Linux utility for binding keyboard. mouse, touchpad and touchscreen actions to system actions (KWin compositor plugin implementation)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { nixpkgs, ... }:
    let
      inherit (nixpkgs) lib;
      systems = [
        "aarch64-linux"
        "x86_64-linux"
      ];
      perSystem =
        f:
        lib.genAttrs systems (
          system:
          f {
            pkgs = import nixpkgs { inherit system; };
          }
        );
    in
    {
      packages = perSystem (
        { pkgs }:
        rec {
          inputactions = pkgs.kdePackages.callPackage ./standalone/nix/package.nix { };
          inputactions-ctl = pkgs.kdePackages.callPackage ./ctl/nix/package.nix { };
          inputactions-hyprland = pkgs.callPackage ./hyprland/nix/package.nix { };
          inputactions-kwin = pkgs.kdePackages.callPackage ./kwin/nix/package.nix { };
          inputactions-standalone = inputactions;
          default = inputactions-kwin;
        }
      );
    };
}
