rm -rf build
west build -s app -d build/left -b eyelash_sofle_leftt -- -DZMK_CONFIG=/workspaces/zmk-config/config -DZMK_EXTRA_MODULES=/workspaces/zmk-config
west build -s app -d build/right -b eyelash_sofle_right -- -DZMK_CONFIG=/workspaces/zmk-config/config -DZMK_EXTRA_MODULES=/workspaces/zmk-config