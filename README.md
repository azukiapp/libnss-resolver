# libnss-resolver

A Linux extension that adds support to the `/etc/resolver/[suffix]`. It supports different "nameservers" to specific suffixes. It emulates the corresponding native functionality from Mac OS X.

This is still a work in progress!

## Installing

### From package

Download and install the appropriate package for your Linux distribution [here](https://github.com/azukiapp/libnss-resolver/releases).

### From the source (with azk)

First: install [https://azk.io][azk].

And after that:

```bash
$ git clone git@github.com:azukiapp/libnss-resolver.git
# or
$ git clone https://github.com/azukiapp-samples/libnss-resolver
$ cd libnss-resolver

# build
# [so] options: debian8-0, ubuntu12, ubuntu14, ubuntu15, fedora20, fedora23
$ azk shell [so]


# using debian8-0, ubuntu12, ubuntu14 or ubuntu15 as [so]:

# run tests
$ azk start dns # Run a mock dns server for testing
$ azk shell [so] -t -c "scons run-test"
# or debug
$ azk shell [so] -t -c "scons run-test -Q define=DEBUG"

# install local
$ azk shell [so] -c "scons install"


# using fedora20 or fedora23 as [so]:

# run tests
$ azk start dns # Run a mock dns server for testing
$ azk shell [so] -t -c "scons run-test -Q prefix=/usr/lib64"
# or debug
$ azk shell [so] -t -c "scons run-test -Q define=DEBUG -Q prefix=/usr/lib64"

# install local
$ azk shell [so] -c "scons install -Q prefix=/usr/lib64"
```

Now you can add the resolver-nss in a resolution pipe:

```bash
$ sudo sed -i -re 's/^(hosts: .*files)(.*)$/\1 resolver\2/' /etc/nsswitch.conf
```

or edit `/etc/nsswitch.conf`:

```bash
# normally   ↓
hosts: files resolver dns
# but, if you have avahi (Zeroconf) installed
#            ↓
hosts: files resolver mdns4_minimal [NOTFOUND=return] dns mdns4 resolver
```

### From the source (without azk)

Dependencies: scons and clang

```bash
$ git clone git@github.com:azukiapp/libnss-resolver.git
# or
$ git clone https://github.com/azukiapp/libnss-resolver.git
$ cd libnss-resolver

# build
$ scons local-install
# or, if you're using Fedora as SO
$ scons local-install -Q prefix=/usr/lib64
```

Now you can add the resolver-nss in a resolution pipe:

```bash
$ sudo sed -i -re 's/^(hosts: .*$)/\1 resolver/' /etc/nsswitch.conf
```

or edit `/etc/nsswitch.conf`:

```bash
# normally   ↓
hosts: files resolver dns
# but, if you have avahi (Zeroconf) installed
#            ↓
hosts: files resolver mdns4_minimal [NOTFOUND=return] dns mdns4 resolver
```

## Configuring

After installing you can create many `suffix` zones in `/etc/resolver/`, like:

```bash
$ echo "nameserver 127.0.0.1:5353" | sudo tee -a /etc/resolver/test.resolver
```

Installing dnsmasq is a good way to test it:

```bash
$ sudo yum install dnsmasq
$ dnsmasq --bind-interfaces -p 5353 --no-daemon --address=/test.resolver/127.0.0.1
```

Now you can try this:

```bash
# ping sufix
$ ping test.resolver
# or any "subdomain"
$ ping any.test.resolver
```

## Test and build (azk only)

This project uses clang for compiling, scons for mounting and valgrind for analyzing code runtime.

All of these tools are actually available behind the [azk][azk]. So just use the commands below to get a development environment:

```bash
$ azk start dns
# [so] options: debian8-0, ubuntu12, ubuntu14, fedora20, fedora23
$ azk shell [so] -t -c "scons local-install"
# or, if [so] == fedora20 or fedora23
$ azk shell [so] -t -c "scons local-install -Q prefix=/usr/lib64"
```

After that, the following scons targets are available:

```bash
# using debian8-0, ubuntu12, ubuntu14 or ubuntu15 as so

# build and run tests
$ scons run-test -Q [define=DEBUG] [valgrind="valgrind options"]
# install in azk instance
$ scons local-install
# test:
$ ping test.resolver


# using fedora20 or fedora23 as so

# build and run tests
$ scons run-test -Q prefix=/usr/lib64 -Q [define=DEBUG] [valgrind="valgrind options"]
# install in azk instance
$ scons local-install -Q prefix=/usr/lib64
# test:
$ ping test.resolver
```

## References

* Inspiration code: https://github.com/danni/docker-nss
* Mac OS X resolver feature: [https://developer.apple.com/library/...](https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man5/resolver.5.html)
* Simple c-ares example: https://gist.github.com/mopemope/992777
* Using a dns server in c-ares: https://github.com/bagder/c-ares/blob/master/adig.c
* Ip and port formats: https://sourceware.org/bugzilla/show_bug.cgi?id=14242
* Using blocks in linux: http://mackyle.github.io/blocksruntime/

## License

"Azuki", "Azk" and the Azuki logo are copyright (c) 2013-2016 Azuki Serviços de Internet LTDA.

Azk source code is released under Apache 2 License.

Check LEGAL and LICENSE files for more information.

[azk]: http://azk.io
