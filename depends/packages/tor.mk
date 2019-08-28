package=tor
$(package)_version=0.4.1.5
$(package)_download_path=https://dist.torproject.org
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=a864e0b605fb933fcc167bf242eed4233949e8a1bf23ac8e0381b106cd920425
$(package)_dependencies=zlib libevent openssl
$(package)_patches=remove_libcap.patch

define $(package)_set_vars
  $(package)_config_opts=--disable-asciidoc --disable-tool-name-check --disable-seccomp --with-openssl-dir=$(host_prefix)/etc/openssl
endef

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/remove_libcap.patch
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install && \
  mkdir -p $($(package)_staging_prefix_dir)/lib && \
  cp $($(package)_build_dir)/src/core/libtor-app.a $($(package)_staging_prefix_dir)/lib/libtor-app.a && \
  cp $($(package)_build_dir)/src/lib/libtor-ctime.a $($(package)_staging_prefix_dir)/lib/libtor-ctime.a && \
  cp $($(package)_build_dir)/src/lib/libtor-crypt-ops.a $($(package)_staging_prefix_dir)/lib/libtor-crypt-ops.a && \
  cp $($(package)_build_dir)/src/lib/libtor-dispatch.a $($(package)_staging_prefix_dir)/lib/libtor-dispatch.a && \
  cp $($(package)_build_dir)/src/lib/libcurve25519_donna.a $($(package)_staging_prefix_dir)/lib/libcurve25519_donna.a && \
  cp $($(package)_build_dir)/src/lib/libtor-log.a $($(package)_staging_prefix_dir)/lib/libtor-log.a && \
  cp $($(package)_build_dir)/src/lib/libtor-string.a $($(package)_staging_prefix_dir)/lib/libtor-string.a && \
  cp $($(package)_build_dir)/src/lib/libtor-malloc.a $($(package)_staging_prefix_dir)/lib/libtor-malloc.a && \
  cp $($(package)_build_dir)/src/lib/libtor-thread.a $($(package)_staging_prefix_dir)/lib/libtor-thread.a && \
  cp $($(package)_build_dir)/src/lib/libtor-err.a $($(package)_staging_prefix_dir)/lib/libtor-err.a && \
  cp $($(package)_build_dir)/src/lib/libtor-lock.a $($(package)_staging_prefix_dir)/lib/libtor-lock.a && \
  cp $($(package)_build_dir)/src/lib/libtor-wallclock.a $($(package)_staging_prefix_dir)/lib/libtor-wallclock.a && \
  cp $($(package)_build_dir)/src/lib/libtor-intmath.a $($(package)_staging_prefix_dir)/lib/libtor-intmath.a && \
  cp $($(package)_build_dir)/src/lib/libtor-buf.a $($(package)_staging_prefix_dir)/lib/libtor-buf.a && \
  cp $($(package)_build_dir)/src/lib/libtor-compress.a $($(package)_staging_prefix_dir)/lib/libtor-compress.a && \
  cp $($(package)_build_dir)/src/lib/libtor-container.a $($(package)_staging_prefix_dir)/lib/libtor-container.a && \
  cp $($(package)_build_dir)/src/lib/libtor-encoding.a $($(package)_staging_prefix_dir)/lib/libtor-encoding.a && \
  cp $($(package)_build_dir)/src/lib/libtor-evloop.a $($(package)_staging_prefix_dir)/lib/libtor-evloop.a && \
  cp $($(package)_build_dir)/src/lib/libtor-fdio.a $($(package)_staging_prefix_dir)/lib/libtor-fdio.a && \
  cp $($(package)_build_dir)/src/lib/libtor-fs.a $($(package)_staging_prefix_dir)/lib/libtor-fs.a && \
  cp $($(package)_build_dir)/src/lib/libtor-geoip.a $($(package)_staging_prefix_dir)/lib/libtor-geoip.a && \
  cp $($(package)_build_dir)/src/lib/libtor-math.a $($(package)_staging_prefix_dir)/lib/libtor-math.a && \
  cp $($(package)_build_dir)/src/lib/libtor-memarea.a $($(package)_staging_prefix_dir)/lib/libtor-memarea.a && \
  cp $($(package)_build_dir)/src/lib/libtor-meminfo.a $($(package)_staging_prefix_dir)/lib/libtor-meminfo.a && \
  cp $($(package)_build_dir)/src/lib/libtor-net.a $($(package)_staging_prefix_dir)/lib/libtor-net.a && \
  cp $($(package)_build_dir)/src/lib/libtor-osinfo.a $($(package)_staging_prefix_dir)/lib/libtor-osinfo.a && \
  cp $($(package)_build_dir)/src/lib/libtor-process.a $($(package)_staging_prefix_dir)/lib/libtor-process.a && \
  cp $($(package)_build_dir)/src/lib/libtor-pubsub.a $($(package)_staging_prefix_dir)/lib/libtor-pubsub.a && \
  cp $($(package)_build_dir)/src/lib/libtor-sandbox.a $($(package)_staging_prefix_dir)/lib/libtor-sandbox.a && \
  cp $($(package)_build_dir)/src/lib/libtor-smartlist-core.a $($(package)_staging_prefix_dir)/lib/libtor-smartlist-core.a && \
  cp $($(package)_build_dir)/src/lib/libtor-term.a $($(package)_staging_prefix_dir)/lib/libtor-term.a && \
  cp $($(package)_build_dir)/src/lib/libtor-time.a $($(package)_staging_prefix_dir)/lib/libtor-time.a && \
  cp $($(package)_build_dir)/src/lib/libtor-tls.a $($(package)_staging_prefix_dir)/lib/libtor-tls.a && \
  cp $($(package)_build_dir)/src/lib/libtor-trace.a $($(package)_staging_prefix_dir)/lib/libtor-trace.a && \
  cp $($(package)_build_dir)/src/lib/libtor-version.a $($(package)_staging_prefix_dir)/lib/libtor-version.a && \
  cp $($(package)_build_dir)/src/trunnel/libor-trunnel.a $($(package)_staging_prefix_dir)/lib/libor-trunnel.a && \
  cp $($(package)_build_dir)/src/ext/ed25519/donna/libed25519_donna.a $($(package)_staging_prefix_dir)/lib/libed25519_donna.a && \
  cp $($(package)_build_dir)/src/ext/ed25519/ref10/libed25519_ref10.a $($(package)_staging_prefix_dir)/lib/libed25519_ref10.a && \
  cp $($(package)_build_dir)/src/ext/keccak-tiny/libkeccak-tiny.a $($(package)_staging_prefix_dir)/lib/libkeccak-tiny.a
endef

define $(package)_postprocess_cmds
endef

