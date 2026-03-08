#!/bin/sh
set -e

installer_dir="${XDG_DATA_HOME:-$HOME/.local/share}/inputactions-installer"

while [[ $# -gt 0 ]]; do
    case $1 in
        --ctl)
            build_ctl=1
            shift
            ;;
        --ctl-branch)
            ctl_branch=$2
            shift
            shift
            ;;
        --kwin)
            build_kwin=1
            shift
            ;;
        --kwin-branch)
            kwin_branch=$2
            shift
            shift
            ;;
        --standalone)
            build_standalone=1
            shift
            ;;
        --standalone-branch)
            standalone_branch=$2
            shift
            shift
            ;;
        --standalone-no-systemd)
            standalone_no_systemd=1
            shift
            ;;

        --latest)
            latest=1
            shift
            ;;
        --latest-stable)
            latest_stable=1
            shift
            ;;

        -p|--package)
            cpack_generator=$2
            shift
            shift
            ;;

        -d|--debug)
            debug=1
            shift
            ;;

        -h|--help|*)
            cat << EOF
Usage: $0 [options]

Options:
  --ctl                         Build the control tool
  --ctl-branch <branch>         Build a custom Git branch of the control tool (checks out the specified branch)
  --kwin                        Build the KWin implementation (mutually exclusive with --standalone)
  --kwin-branch <branch>        Build a custom Git branch of the KWin implementation (checks out the specified branch)
  --standalone                  Build the standalone implementation (mutually exclusive with --kwin)
  --standalone-branch <branch>  Build a custom Git branch of the standalone implementation (checks out the specified branch)
  --standalone-no-systemd       Disable systemd support

  --latest                      Build the latest Git commits of all projects (checks out the main branch)
  --latest-stable               Build the latest stable releases of all projects (checks out the latest tag or the main branch if there are none)

  -d,--debug                    Build the debug configuration
  -p,--package <generator>      Generate packages using the specified CPack generator instead of installing

  -h,--help                     Show this message and exit

The installer stores data in "$installer_dir".
EOF
            exit 0
            ;;
    esac
done

build_dir="$installer_dir/build"
kwin_dir="$installer_dir/kwin"
standalone_dir="$installer_dir/standalone"

mkdir -p $installer_dir
cmakelists_file="$installer_dir/CMakeLists.txt"
cat > $cmakelists_file << EOF
cmake_minimum_required(VERSION 3.16.0)
project(inputactions_installer)
EOF

cmake_flags="-DCMAKE_INSTALL_PREFIX=/usr"
if [[ -n $debug ]]; then
    cmake_flags="$cmake_flags -DCMAKE_BUILD_TYPE=Debug"
fi
if [[ -n $standalone_no_systemd ]]; then
    cmake_flags="$cmake_flags -DINPUTACTIONS_SYSTEMD=OFF"
fi

cmakelists="cmake_minimum_required(VERSION 3.16.0)\nproject(inputactions_installer)"

function ensure_repository() {
    repo_name="$1"
    repo_branch="$2"

    repo_dir="$installer_dir/$1"
    if [[ ! -d "$repo_dir" ]]; then
        git clone --recursive "https://github.com/InputActions/$repo_name" "$repo_dir"
    fi
    if [[ -n "$repo_branch" ]]; then
        git -C "$repo_dir" fetch
        git -C "$repo_dir" checkout "$repo_branch"
        git -C "$repo_dir" submodule update --init --recursive
    elif [[ -n $latest ]]; then
        git -C "$repo_dir" checkout main
        git -C "$repo_dir" pull
        git -C "$repo_dir" submodule update --init --recursive
    elif [[ -n $latest_stable ]]; then
        git -C "$repo_dir" fetch
        git -C "$repo_dir" checkout "$(git -C "$repo_dir" describe --tags "$(git -C "$repo_dir" rev-list --tags --max-count=1)" 2> /dev/null || echo main)"
        git -C "$repo_dir" submodule update --init --recursive
    fi
}

if [[ -n $build_ctl ]]; then
    echo -e "add_subdirectory(ctl)" >> $cmakelists_file
    ensure_repository "ctl" "$ctl_branch"
fi
if [[ -n $build_kwin ]]; then
    echo -e "add_subdirectory(kwin)" >> $cmakelists_file
    ensure_repository "kwin" "$kwin_branch"
elif [[ -n $build_standalone ]]; then
    echo -e "add_subdirectory(standalone)" >> $cmakelists_file
    ensure_repository "standalone" "$standalone_branch"
fi

if [[ ! -n $build_ctl && ! -n $build_kwin && ! -n $build_standalone ]]; then
    echo "No build target specified."
    exit 1
fi

if [[ -d "$build_dir" ]]; then
    make -C "$build_dir" clean || true
else
    mkdir "$build_dir"
fi

cmake --fresh -B"$build_dir" -S"$installer_dir" $cmake_flags
make -C "$build_dir" -j$(nproc)

if [[ -n "$cpack_generator" ]]; then
    if [[ -n $build_ctl ]]; then
        cpack -B "$installer_dir" -G "$cpack_generator" --config "$build_dir/ctl/CPack.cmake"
    fi
    if [[ -n $build_kwin ]]; then
        cpack -B "$installer_dir" -G "$cpack_generator" --config "$build_dir/kwin/CPack.cmake"
    elif [[ -n $build_standalone ]]; then
        cpack -B "$installer_dir" -G "$cpack_generator" --config "$build_dir/standalone/CPack.cmake"
    fi
else
    sudo make -C "$build_dir" install
fi
