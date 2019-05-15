package=gperftools
$(package)_version=2.7
$(package)_name=$(package)-$($(package)_version)
$(package)_download_path=https://github.com/$(package)/$(package)/releases/download/$($(package)_name)
$(package)_file_name=$($(package)_name).tar.gz
$(package)_sha256_hash=1ee8c8699a0eff6b6a203e59b43330536b22bbcbe6448f54c7091e5efb0763c9

define $(package)_set_vars
  $(package)_config_opts=
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
