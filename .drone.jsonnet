local distro = "archlinux";
local distro_name = 'Archlinux';
local distro_docker = 'archlinux:latest';

local repo_suffix = '/makepkg';

local submodules = {
    name: 'submodules',
    image: 'drone/git',
    commands: ['git fetch --tags', 'git submodule update --init --recursive --depth=1']
};

local aur_pipeline(image, buildarch='amd64', aurarch='amd64', jobs=6) = {
    kind: 'pipeline',
    type: 'docker',
    name: distro_name + ' (' + aurarch + ')',
    platform: { arch: buildarch },
    steps: [
        submodules,
        {
            name: 'build',
            image: image,
            environment: { SSH_KEY: { from_secret: "SSH_KEY" } },
            commands: [
                'echo "Building on ${DRONE_STAGE_MACHINE}"',
    #install package tools
		'pacman -S --noconfirm base-devel',
    #build Lokinet package
		'mkdir -p /drone/src/build/',
		'cd /drone/src/build',
		'curl -LJO https://raw.githubusercontent.com/oxen-io/lokinet/makepkg/contrib/archlinux/PKGBUILD',
		'makepkg -sr,
    #upload package
		'mkdir -p /drone/src/archlinux/' + aurarch,
		'cp /drone/source/build/lokinet*.* /drone/src/archlinux/' + aurarch,
		'echo "archlinux package directory contents"',
		'ls -la /drone/src/archlinux/' + aurarch,
		'./ci-upload.sh ' + distro + ' ' + aurarch,
            ]
        }
    ]
};

[
    aur_pipeline(distro_docker),
    #aur_pipeline("i386/" + distro_docker, buildarch='amd64', aurarch='i386'),
    #aur_pipeline("arm64v8/" + distro_docker, buildarch='arm64', aurarch="arm64", jobs=4),
    #aur_pipeline("arm32v7/" + distro_docker, buildarch='arm64', aurarch="armhf", jobs=4),

]
