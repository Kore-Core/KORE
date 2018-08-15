package=curl
$(package)_version=7.60.0
$(package)_download_path=http://curl.haxx.se/download
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=e9c37986337743f37fd14fe8737f246e97aec94b39d1b71e8a5973f72a9fc4f5
$(package)_dependencies=openssl

define $(package)_set_vars
$(package)_config_opts=--disable-shared  --disable-manual 
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
