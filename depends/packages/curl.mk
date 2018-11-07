package=curl
$(package)_version=7.61.1
$(package)_download_path=https://github.com/curl/curl/releases/download/curl-7_61_1/
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=eaa812e9a871ea10dbe8e1d3f8f12a64a8e3e62aeab18cb23742e2f1727458ae

define $(package)_set_vars
  $(package)_config_opts=--disable-shared
  $(package)_config_opts_release=--disable-debug-mode
  $(package)_config_opts_linux=--with-pic
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
endef
