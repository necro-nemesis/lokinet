local distro = "openwrt";
local distro_name = 'OpenWrt';
local distro_docker = 'alpine:3.12';
#local distro_docker = 'alpine:latest';
#latest 3.15 tested on v21.02 incompatible install

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
                'apk update --quiet',
    		'apk add build-base cmake git libcap-dev libuv-dev libsodium-dev perl sqlite-dev unbound-dev m4 zeromq-dev libtool automake autoconf curl-dev tar openssh',
    #build Lokinet
		'export LDFLAGS="-static-libstdc++ -static-libgcc"',
		'mkdir -p /drone/src/build/contents',
		'cd /drone/src/build',
		'cmake .. -DWITH_SETCAP=OFF -DBUILD_STATIC_DEPS=ON -DBUILD_SHARED_LIBS=OFF -DSTATIC_LINK=ON -DNATIVE_BUILD=OFF -DWITH_SYSTEMD=OFF -DWITH_LTO=OFF -DBUILD_LIBLOKINET=OFF -DWITH_TESTS=OFF -DWITH_BOOTSTRAP=OFF -DCMAKE_BUILD_TYPE=Release',
		'make -j' + jobs,
		'make DESTDIR=/drone/src/build/contents install',
    #add built Lokinet binaries to OpenWrt package base files
		'cp -r /drone/src/build/contents/usr/ /drone/src/contrib/openwrt/base/'+ apkarch,
		'cp /drone/src/contrib/bootstrap/mainnet.signed /drone/src/contrib/openwrt/base/' + apkarch + '/usr/contrib/bootstrap.signed',
    'cp /drone/src/contrib/openwrt/banner /drone/src/contrib/openwrt/base/' + apkarch + '/usr/contrib/banner',
    #build ipkg package
    		'mkdir -p /drone/src/openwrt/' + apkarch,
		'cd ../contrib/openwrt/',
		'./ipkg-build.sh /drone/src/contrib/openwrt/base/' + apkarch +' /drone/src/openwrt/' + apkarch,
		'echo "openwrt package directory contents"',
		'ls -la /drone/src/openwrt/' + apkarch,
		'./ci-upload.sh ' + distro + ' ' + apkarch,
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
