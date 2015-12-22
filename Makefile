
HASH_GIT=2.0.3

hash:
	@if [ ! -d "$(CURDIR)/ext/sparsehash/$(HASH_GIT)" ]; then \
		git clone http://github.com/mkn/google.sparsehash.git --branch $(HASH_GIT) ext/sparsehash/$(HASH_GIT); \
		cd ./ext/sparsehash/$(HASH_GIT); bash ./configure --prefix=$(CURDIR)/ext/sparsehash/$(HASH_GIT); \
		$(MAKE) -C $(CURDIR)/ext/sparsehash/$(HASH_GIT); \
		$(MAKE) -C $(CURDIR)/ext/sparsehash/$(HASH_GIT) install; \
	fi;

clean:
	rm -rf bin

clean-all:
	rm -rf $(CURDIR)/ext
