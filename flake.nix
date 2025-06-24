{
  description = "Custom mouse and touchpad gestures for Hyprland, Plasma 6 Wayland";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, ... }@inputs: inputs.utils.lib.eachSystem [
    "x86_64-linux" "aarch64-linux"
  ] (system: let
    pkgs = import nixpkgs {
      inherit system;
    };
  in rec {
    packages = rec {
      default = pkgs.kdePackages.callPackage ./nix/package-kwin.nix { };
      inputactions-kwin = default;
      inputactions-hyprland = pkgs.callPackage ./nix/package-hyprland.nix { };
    };

    devShells.default = pkgs.mkShell {
      inputsFrom = [ packages.inputactions-kwin packages.inputactions-hyprland ];
      packages = [ pkgs.gtest ];
    };
  });
}
