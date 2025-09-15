{
	description = "print-util devenv";

	inputs = {
		nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
	};

	outputs = { self, nixpkgs }:
		let
			system = "aarch64-linux";
			pkgs = import nixpkgs { inherit system; };
		in {
			devShells.aarch64-linux.default = pkgs.mkShell {
				name = "print-util";

				buildInputs = with pkgs; [
					clang-tools
					gcc15
					glibc
					libusb1
					llvmPackages_latest.clang
					llvmPackages_latest.libllvm
				];

				shellHook = let
					build_command_setup = ''cd $DIR && mkdir -p $DIR/build/ && cd $DIR/build/'';
					build_command_reset = ''cd $DIR'';

					source_files = ''$DIR/src/main.c $DIR/src/pu.c'';

					out_filename = ''print-util'';

					build_command = ''
						${build_command_setup} && gcc ${source_files} -L${pkgs.libusb1}/lib -lusb-1.0 -lsystemd -o${out_filename};
						${build_command_reset}
					'';
				in ''
					echo "print-util development shell";
					echo ${pkgs.udev}
					export DIR=$(pwd);
					echo -e "-lsystemd" > $DIR/compile_flags.txt;
					alias build="${build_command}";
				'';
			};
	};
}
