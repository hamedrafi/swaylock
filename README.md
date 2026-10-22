# swaylock

swaylock is a screen locking utility for Wayland compositors. It is compatible
with any Wayland compositor which implements the ext-session-lock-v1 Wayland
protocol.

See the man page, [swaylock(1)](swaylock.1.scd), for instructions on using swaylock.

The lock screen shows animated Matrix-style green character rain on black by
default. Run `swaylock --no-matrix` to use a static background instead, for example
`swaylock --no-matrix --image wallpaper.png`. Both `matrix` and `no-matrix` can also
be used in the configuration file.

## Release Signatures

Releases are signed with [E88F5E48](https://keys.openpgp.org/search?q=34FF9526CFEF0E97A340E2E40FDE7BE0E88F5E48)
and published [on GitHub](https://github.com/swaywm/swaylock/releases). swaylock
releases are managed independently of sway releases.

## Installation

### From Packages

Swaylock is available in many distributions. Try installing the "swaylock"
package for yours.

### Compiling from Source

Install dependencies:

* meson \*
* wayland
* wayland-protocols \*
* libxkbcommon
* cairo
* gdk-pixbuf2 \*\*
* pam (optional)
* [scdoc](https://git.sr.ht/~sircmpwn/scdoc) (optional: man pages) \*
* git \*

_\* Compile-time dep_  
_\*\* Optional: required for background images other than PNG_

Run these commands:

    meson build
    ninja -C build
    sudo ninja -C build install

On Ubuntu/Debian, configure or rebuild the Matrix version and run its tests with:

    ./rebuild.sh

The script downloads and installs missing build packages using APT and `sudo`
when needed, including PAM support, image support, man-page tools, and a monospace
font. Once the dependencies are available, use `./rebuild.sh --skip-deps` to
rebuild offline or use dependencies installed manually on another distribution.

The script reuses the settings in `build/` and any dependencies already unpacked
in `build/deps/`. Additional arguments are passed to Meson setup, for example
`./rebuild.sh -Dbuildtype=release`. Put `--skip-deps` before any Meson options.
Run `./build/swaylock` to use the rebuilt binary.

##### Without PAM

On systems without PAM, swaylock uses `shadow.h`.

Systems which rely on a tcb-like setup (either via musl's native support or via
glibc+[tcb]), require no further action.

[tcb]: https://www.openwall.com/tcb/

For most other systems, where passwords for all users are stored in `/etc/shadow`,
swaylock needs to be installed suid:

    sudo chmod a+s /usr/local/bin/swaylock

Optionally, on systems where the file `/etc/shadow` is owned by the `shadow`
group, the binary can be made sgid instead:

    sudo chgrp shadow /usr/local/bin/swaylock
    sudo chmod g+s /usr/local/bin/swaylock

Swaylock will drop root permissions shortly after startup.
