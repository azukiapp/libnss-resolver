/**
 * Documentation: http://docs.azk.io/Azkfile.js
 */

var envs = {
  DNS_DOMAIN: "test.resolver",
  DNS_IP: "127.0.0.2",
  TERM: env.TERM,
  BUILD_FOLDER: "/azk/build",
}

var build_system = function(image) {
  return {
    depends: ["dns"],
    image: { docker: image },
    workdir: "/azk/#{manifest.dir}",
    command: "# command to run app",
    shell: "/bin/bash",
    mounts: {
      "/azk/#{manifest.dir}" : '.',
      "/etc/nsswitch.conf"   : './mocker/#{system.name}-nsswitch.conf',
      "/etc/resolver"        : './mocker/resolver',
      "/azk/build"           : persistent('build-#{system.name}'),
      "/azk/lib"             : '/usr/lib'
    },
    envs: envs,
  };
}

// Adds the systems that shape your system
systems({
  ubuntu14: build_system("azukiapp/libnss-resolver:ubuntu14"),
  ubuntu12: build_system("azukiapp/libnss-resolver:ubuntu12"),
  fedora20: build_system("azukiapp/libnss-resolver:fedora20"),

  package: {
    image: { docker: "azukiapp/fpm" },
    workdir: "/azk/#{manifest.dir}",
    shell: "/bin/bash",
    mounts: {
      "/azk/#{manifest.dir}" : '.',
    },
    envs: envs,
  },

  dns: {
    image: { docker: "azukiapp/azktcl:0.0.2" },
    command: "dnsmasq --no-daemon --address=/$DNS_DOMAIN/$DNS_IP",
    wait: false,
    ports: {
      dns: "53/udp",
    },
    envs: envs,
  },
});
