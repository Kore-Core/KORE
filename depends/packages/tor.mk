package=tor
$(package)_version=0.3.4.9
$(package)_download_path=https://dist.torproject.org
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=1a171081f02b9a6ff9e28c0898defb7670e5bbb3bdbcaddfcf4e4304aedd164a
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
  cp $($(package)_build_dir)/src/or/libtor.a $($(package)_staging_prefix_dir)/lib/libtor.a && \
  cp $($(package)_build_dir)/src/common/libor.a $($(package)_staging_prefix_dir)/lib/libor.a && \
  cp $($(package)_build_dir)/src/common/libor-ctime.a $($(package)_staging_prefix_dir)/lib/libor-ctime.a && \
  cp $($(package)_build_dir)/src/common/libor-crypto.a $($(package)_staging_prefix_dir)/lib/libor-crypto.a && \
  cp $($(package)_build_dir)/src/common/libor-event.a $($(package)_staging_prefix_dir)/lib/libor-event.a && \
	cp $($(package)_build_dir)/src/common/libcurve25519_donna.a $($(package)_staging_prefix_dir)/lib/libcurve25519_donna.a && \
  cp $($(package)_build_dir)/src/trunnel/libor-trunnel.a $($(package)_staging_prefix_dir)/lib/libor-trunnel.a && \
	cp $($(package)_build_dir)/src/ext/ed25519/donna/libed25519_donna.a $($(package)_staging_prefix_dir)/lib/libed25519_donna.a && \
  cp $($(package)_build_dir)/src/ext/ed25519/ref10/libed25519_ref10.a $($(package)_staging_prefix_dir)/lib/libed25519_ref10.a && \
  cp $($(package)_build_dir)/src/ext/keccak-tiny/libkeccak-tiny.a $($(package)_staging_prefix_dir)/lib/libkeccak-tiny.a
endef

define $(package)_postprocess_cmds
endef

