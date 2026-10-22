#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
cd "$repo_dir"

install_deps=true
while [ "$#" -gt 0 ]; do
	case "$1" in
		--skip-deps)
			install_deps=false
			shift
			;;
		-h|--help)
			printf '%s\n' \
				'Usage: ./rebuild.sh [--skip-deps] [--] [Meson setup options...]' \
				'' \
				'Install missing Ubuntu/Debian build packages, rebuild swaylock, and run tests.' \
				'APT uses sudo when needed. Use --skip-deps for an offline rebuild.' \
				'Example: ./rebuild.sh --skip-deps -Dbuildtype=release'
			exit 0
			;;
		--)
			shift
			break
			;;
		*)
			break
			;;
	esac
done

run_as_root() {
	if [ "$(id -u)" -eq 0 ]; then
		"$@"
	else
		sudo "$@"
	fi
}

install_dependencies() {
	for tool in apt-get dpkg-query; do
		if ! command -v "$tool" >/dev/null 2>&1; then
			printf '%s\n' \
				'Automatic dependency installation requires Ubuntu/Debian with APT.' \
				'Install the dependencies in README.md, then rerun with --skip-deps.' >&2
			exit 1
		fi
	done

	# Function arguments hold only missing packages; preserve the Meson options.
	set --
	for package in build-essential meson ninja-build pkg-config git \
			libwayland-dev libwayland-bin wayland-protocols libxkbcommon-dev \
			libcairo2-dev libgdk-pixbuf-2.0-dev libpam0g-dev libcrypt-dev \
			scdoc fonts-dejavu-core; do
		status=$(dpkg-query -W -f='${Status}' "$package" 2>/dev/null) || status=
		if [ "$status" != 'install ok installed' ]; then
			set -- "$@" "$package"
		fi
	done

	if [ "$#" -eq 0 ]; then
		return
	fi
	if [ "$(id -u)" -ne 0 ] && ! command -v sudo >/dev/null 2>&1; then
		printf 'sudo is required to install missing packages: %s\n' "$*" >&2
		exit 1
	fi

	printf 'Installing missing build dependencies: %s\n' "$*"
	run_as_root apt-get update
	run_as_root apt-get install --yes --no-install-recommends "$@"
}

if [ "$install_deps" = true ]; then
	install_dependencies
fi

# Reuse development dependencies unpacked for the original local build.
deps_dir="$repo_dir/build/deps/usr"
if [ -d "$deps_dir" ]; then
	PATH="$deps_dir/bin:$PATH"
	for pkgconfig_dir in "$deps_dir"/lib/*/pkgconfig \
			"$deps_dir/lib/pkgconfig" "$deps_dir/share/pkgconfig"; do
		if [ -d "$pkgconfig_dir" ]; then
			PKG_CONFIG_PATH="$pkgconfig_dir${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
		fi
	done
	export PATH PKG_CONFIG_PATH
fi

for tool in meson ninja pkg-config; do
	if ! command -v "$tool" >/dev/null 2>&1; then
		printf 'Missing build tool: %s. See README.md for dependencies.\n' "$tool" >&2
		exit 1
	fi
done

# Keep existing settings; optional arguments are passed to Meson setup.
if [ -f build/meson-private/coredata.dat ]; then
	meson setup --reconfigure build "$@"
else
	meson setup build "$@"
fi

meson compile -C build
meson test -C build --no-rebuild --print-errorlogs

printf '\nRebuilt swaylock: %s/build/swaylock\n' "$repo_dir"
