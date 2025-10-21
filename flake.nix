{
	description = "print-util devenv";

	inputs = {
		nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
	};

	outputs = { self, nixpkgs }:
		let
			system = "aarch64-linux";
			pkgs = import nixpkgs { inherit system; };

			lib = nixpkgs.lib;

			sanitize_newlines = string:
				let
					split_string = lib.strings.splitString "\n" string;
					out = lib.strings.concatStringsSep "\\n" split_string;
				in out;
		in {
			devShells.aarch64-linux.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
				name = "print-util";

				packages = with pkgs; [
					clang-tools
					libusb1
					gnumake
				];

				shellHook = let
					makefile_contents = "# DO NOT EDIT! THIS FILE IS AUTO-GENERATED!\\n\\n" +
						(sanitize_newlines (builtins.readFile ./Makefile.template));
				in ''
					echo "print-util development shell";
					export DIR=$(pwd);

					echo -e '${makefile_contents}' > $DIR/Makefile;
				'';
			};
	};
}
