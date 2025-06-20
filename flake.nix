{
  description = "Custom touchpad and touchscreen shortcuts";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/9e83b64f727c88a7711a2c463a7b16eedb69a84c";
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
