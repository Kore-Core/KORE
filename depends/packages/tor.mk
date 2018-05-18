package=tor
$(package)_version=0.3.2.9
$(package)_download_path=https://dist.torproject.org
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=435a7b91aa98d8b1a0ac1f60ca30c0ff3665b18a02e570bab5fe27935829160f
$(package)_dependencies=libevent openssl

define $(package)_set_vars
$(package)_config_opts=--disable-asciidoc
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
