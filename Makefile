
HASH_GIT=2.0.3

CWD:=$(CURDIR)
ifeq ($(strip $(CURDIR)),)
  CWD:=$(.CURDIR)
endif

hash:
	@if [ ! -d "$(CWD)/ext/sparsehash/$(HASH_GIT)" ]; then \
		git clone http://github.com/mkn/google.sparsehash.git --branch $(HASH_GIT) ext/sparsehash/$(HASH_GIT); \
		cd ./ext/sparsehash/$(HASH_GIT); bash ./configure --prefix=$(CWD)/ext/sparsehash/$(HASH_GIT); \
		$(MAKE) -C $(CWD)/ext/sparsehash/$(HASH_GIT); \
		$(MAKE) -C $(CWD)/ext/sparsehash/$(HASH_GIT) install; \
	fi;

clean:
	rm -rf bin

clean-all:
	rm -rf $(CWD)/ext
