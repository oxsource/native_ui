# Host verification module (feature 011-ahwb-external-image, T032).
# Targets are prefixed `host-`.
$(call register_module,host)
$(call register_target,host-verify)

.PHONY: host-verify

host-verify: ## Run the full host test suite (T032)
	bash $(V)/host.sh
