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
    		#'apk upgrade',
		'apk add build-base cmake git libcap-dev libuv-dev libsodium-dev perl sqlite-dev unbound-dev m4 zeromq-dev libtool automake autoconf curl-dev tar',
    #build Lokinet
    		'git clone --recursive https://github.com/necro-nemesis/lokinet.git',
		'cd lokinet/',
		'git checkout alpine/3.12',
		'export LDFLAGS="-static-libstdc++ -static-libgcc"',
		'mkdir /drone/src/lokinet/build',
		'cd /drone/src/lokinet/build',
		'cmake .. -DWITH_SETCAP=OFF -DBUILD_STATIC_DEPS=ON -DBUILD_SHARED_LIBS=OFF -DSTATIC_LINK=ON -DNATIVE_BUILD=OFF -DWITH_SYSTEMD=OFF -DWITH_LTO=OFF -DWITH_TESTS=OFF -DWITH_BOOTSTRAP=OFF -DCMAKE_BUILD_TYPE=Release',
		'make -j' + jobs,
		'mkdir -p /drone/src/lokinet/build/contents',
		'make DESTDIR=/drone/src/lokinet/build/contents install',
    #add built Lokinet binaries to OpenWrt package base files	
		'cp -r /drone/src/lokinet/build/contents/usr/ /drone/src/lokinet/contrib/openwrt/base/',
		'cp /drone/src/lokinet/contrib/bootstrap/mainnet.signed /drone/src/lokinet/contrib/openwrt/base/usr/contrib/bootstrap.signed',
    #build ipkg package
    		'mkdir /drone/src/openwrt',
		'cd ../contrib/openwrt/',
		'./ipkg-build.sh /drone/src/lokinet/contrib/openwrt/base/ /drone/src/openwrt/',
		'echo "openwrt package directory contents"',
		'ls -la /drone/src/openwrt/'
            ]
        }
    ]
};

[
    apk_pipeline(distro_docker),
    #apk_pipeline("i386/" + distro_docker, buildarch='amd64', apkarch='i386'),
    apk_pipeline("arm64v8/" + distro_docker, buildarch='arm64', apkarch="arm64", jobs=4),
    #apk_pipeline("arm32v7/" + distro_docker, buildarch='arm64', apkarch="armhf", jobs=4),

]
