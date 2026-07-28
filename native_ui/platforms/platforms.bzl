def config_setting_and_platform(name, constraint_values, parents = None):
    native.config_setting(
        name = name,
        constraint_values = constraint_values,
    )
    native.platform(
        name = name + "_platform",
        constraint_values = constraint_values,
        parents = parents,
    )

def native_ui_select(select_map):
    return select(select_map)
