# Android build + device verification module (feature 011-ahwb-external-image, T033/T034).
# Device targets need a connected device/emulator and ANDROID_NDK_HOME for builds.
# All targets are prefixed `android-` so this module can never clash with others.
$(call register_module,android)
$(call register_target,android-build)
$(call register_target,android-demo)
$(call register_target,android-pull)
$(call register_target,android-list)
$(call register_target,android-diff)
$(call register_target,android-view)
$(call register_target,android-results)
$(call register_target,android-demo-live)
$(call register_target,android-verify)

.PHONY: android-build android-demo android-pull android-list android-diff android-view android-results android-demo-live android-verify

android-build: ## Android arm64 cross-build of libs + demo (no device; needs ANDROID_NDK_HOME)
	bash $(V)/android_build.sh

android-demo: ## Device closed loop: build + push + run + decode-back pixel-diff (T033)
	bash $(V)/android_demo.sh all

android-pull: ## Pull the last run's artifacts (MP4 + PNGs) from device into out/
	bash $(V)/android_demo.sh pull

android-list: ## List the demo artifacts on the device (/data/local/tmp)
	bash $(V)/android_demo.sh list

android-diff: ## Decode-back pixel-diff: MP4 frame 0 vs the CPU snapshot (T033)
	bash $(V)/android_demo.sh diff

android-view: ## Open the pulled MP4 + PNGs for visual comparison (macOS)
	bash $(V)/android_demo.sh view

android-results: ## Pull + diff + view the latest results
	bash $(V)/android_demo.sh pull
	bash $(V)/android_demo.sh diff
	bash $(V)/android_demo.sh view

android-demo-live: ## Live-update perf run, 30 Hz / 60 s, prints frame ms + VmRSS (T034)
	bash $(V)/android_demo.sh all --live

android-verify: ## Full device validation = android-build + android-demo
	bash $(V)/android_build.sh && bash $(V)/android_demo.sh all
