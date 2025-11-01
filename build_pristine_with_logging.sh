rm -rf build
west build -s app -d build/left -b eyelash_sofle_leftt -S zmk-usb-logging -- -DZMK_CONFIG=/workspaces/zmk-config/config -DZMK_EXTRA_MODULES=/workspaces/zmk-config
west build -s app -d build/right -b eyelash_sofle_right -S zmk-usb-logging -- -DZMK_CONFIG=/workspaces/zmk-config/config -DZMK_EXTRA_MODULES=/workspaces/zmk-config