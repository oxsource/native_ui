# Friendly short aliases for the canonical prefixed targets (conflict-free, unique).
# Each alias maps to a <module>-<action> target; duplicates abort the build.
$(call register_module,aliases)
$(call register_alias,verify,host-verify)
$(call register_alias,build-android,android-build)
$(call register_alias,demo,android-demo)
$(call register_alias,pull,android-pull)
$(call register_alias,list,android-list)
$(call register_alias,diff,android-diff)
$(call register_alias,view,android-view)
$(call register_alias,results,android-results)
$(call register_alias,demo-live,android-demo-live)
$(call register_alias,verify-android,android-verify)
