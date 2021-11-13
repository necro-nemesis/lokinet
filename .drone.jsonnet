local distro = "alpine";
local distro_name = 'Alpine 3.12';
local distro_docker = 'alpine:3.12.8';

local repo_suffix = '/staging'; // can be /beta or /staging for non-primary repo deps

local submodules = {
    name: 'submodules',
    image: 'drone/git',
    commands: ['git fetch --tags', 'git submodule update --init --recursive --depth=1']
};

local apk_pipeline(image, buildarch='amd64', apkarch='amd64', jobs=6) = {
    kind: 'pipeline',
    type: 'docker',
    name: distro_name + ' (' + apkarch + ')',
    platform: { arch: buildarch },
    steps: [
        submodules,
        {
            name: 'build',
            image: image,
            environment: { SSH_KEY: { from_secret: "SSH_KEY" } },
            commands: [
                'echo "Building on ${DRONE_STAGE_MACHINE}"',
                #'echo "man-db man-db/auto-update boolean false" | debconf-set-selections',
                #'echo deb http://deb.loki.network' + repo_suffix + ' ' + distro + ' main >/etc/apt/sources.list.d/loki.list',
                #'cp debian/deb.loki.network.gpg /etc/apt/trusted.gpg.d',
		'apk update --quiet',
    #upgrade (may break OpenWrt v19.07 install)
    'apk upgrade',
		'apk add build-base cmake git libcap-dev libuv-dev libsodium-dev perl sqlite-dev unbound-dev m4 zeromq-dev libtool automake autoconf curl-dev',
    #build Lokinet
    'git clone --recursive https://github.com/oxen-io/loki-network.git',
    'export LDFLAGS="-static-libstdc++ -static-libgcc"',
		'mkdir /drone/src/loki-network/build',
		'cd /drone/src/loki-network/build',
		'cmake .. -DWITH_SETCAP=OFF -DBUILD_STATIC_DEPS=ON -DBUILD_SHARED_LIBS=OFF -DSTATIC_LINK=ON -DNATIVE_BUILD=OFF -DWITH_SYSTEMD=OFF -DWITH_LTO=OFF -DWITH_TESTS=OFF -DWITH_BOOTSTRAP=OFF -DCMAKE_BUILD_TYPE=Release',
		'make -j' + jobs,
		'mkdir -p /drone/src/loki-network/build/contents',
		'sudo make /drone/src/loki-network/build/contents install'
            ],
        }
    ]
};

[
    apk_pipeline(distro_docker),
    #apk_pipeline("i386/" + distro_docker, buildarch='amd64', debarch='i386'),
    #apk_pipeline("arm64v8/" + distro_docker, buildarch='arm64', debarch="arm64", jobs=4),
    #apk_pipeline("arm32v7/" + distro_docker, buildarch='arm64', debarch="armhf", jobs=4),
    #git on atom sucks
]
