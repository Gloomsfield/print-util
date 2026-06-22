{
	description = "print-util devenv";

	inputs = {
		nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
	};

	outputs = { self, nixpkgs }:
		let
			lib = nixpkgs.lib;

			forAllSystems = lib.genAttrs [ "x86_64-linux" "aarch64-linux" ];

			collapseMultiline = { separator, string }:
				let
					splitString = lib.strings.splitString "\n" string;
					trimString = line: lib.strings.trimWith { start = true; end = true; } line;
					stringArray = map (line: trimString line) splitString;
					out = lib.strings.concatStringsSep separator stringArray;
				in out;
		in {
			devShells = forAllSystems (
				system: let
					pkgs = import nixpkgs { inherit system; };
				in {
					default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
						name = "print-util";

						packages = with pkgs; [
							clang-tools
							gnumake

							libusb1

							gdb
						];

						shellHook = let
							clangdFlags = collapseMultiline {
								separator = "\n";
								string = ''
									-isystem${pkgs.clangStdenv.cc.libc.dev}/include/
									-isystem${pkgs.libusb1.dev}/include/
								'';
							};
						in ''
							echo -e "${clangdFlags}" > compile_flags.txt
						'';
					};
				}
			);
	};
}
