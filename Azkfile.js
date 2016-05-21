/**
 * Documentation: http://docs.azk.io/Azkfile.js
 */

var envs = {
  DNS_DOMAIN: "test.resolver",
  DNS_IP: "127.0.0.2",
  TERM: env.TERM,
  BUILD_FOLDER: "/azk/build",
}

// Adds the systems that shape your system
systems({
  ubuntu12: {
    depends: ["dns"],
    image: { dockerfile: "./Dockerfiles/ubuntu12" },
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
  },

  ubuntu16: {
    extends: "ubuntu12",
    image: { dockerfile: "./Dockerfiles/ubuntu16" },
  },

  ubuntu15: {
    extends: "ubuntu12",
    image: { dockerfile: "./Dockerfiles/ubuntu15" },
  },

  ubuntu14: {
    extends: "ubuntu12",
    image: { dockerfile: "./Dockerfiles/ubuntu14" },
  },

  'debian8-0': {
    extends: "ubuntu12",
    image: { dockerfile: "./Dockerfiles/debian8.0" },
  },

  fedora20: {
    extends: "ubuntu12",
    image: { dockerfile: "./Dockerfiles/fedora20" },
  },

  fedora23: {
    extends: "ubuntu12",
    image: { dockerfile: "./Dockerfiles/fedora23" },
  },

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
      80: disable,
    },
    envs: envs,
  },
});
