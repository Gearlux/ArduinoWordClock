My first idea for a three button config is here

| Button | Event       | Command           |
|--------|-------------|-------------------|
| 1      | Clicked     | show_seconds      |
|        | LongPressed | clock_mode        |
| 2      | Clicked     | toggle_brightness |
|        | LongPressed | brightness_mode   |
| 3      | Clicked     | toggle_rgb        |
|        | LongPressed | rgb_mode          |
| 4      | Clicked     | Don't Buy Happpiness Surf |
|        | LongPressed | config_mode       |

For the *clock_mode*

| Button     | Event       | Command           |
|------------|-------------|-------------------|
| 1          | Clicked     | decrease          |
| 2          | Clicked     | toggle hour/5min/min     |
|            | LongPressed | reset_seconds     |
| 3          | Clicked     | increase          |
| 4          | LongPressed | main_mode         |
| (Inactive) | 60s         | main_mode         |

In clock_mode, the seconds will go around the outer circle of the clock.

For the *brightness mode*

| Button     | Event       | Command                 |
|------------|-------------|-------------------------|
| 1          | Clicked     | brightness_down         |
| 2          | Clicked     | toggle_brightness_input |
| 3          | Clicked     | brightness_up           |
| 4          | LongPressed | main_mode               |
| (Inactive) | 60s         | main_mode               |

For the *rgb mode*

| Button     | Event       | Command                 |
|------------|-------------|-------------------------|
| 1          | Clicked     | hue_sat_up              |
| 2          | Clicked     | toggle_hue_sat          |
| 3          | Clicked     | hue_sat_down            |
| 4          | LongPressed | main_mode               |
| (Inactive) | 60s         | main_mode               |

For the *config mode*

| Button     | Event       | Command                 |
|------------|-------------|-------------------------|
| 1          | Clicked     | toggle_it_is            |
|            | LongPressed | toggle_twenty           |
| 2          | Clicked     | toggle_over             |
|            | LongPressed | toggle_dot              |
| 3          | Clicked     | toggle_backlight        |
|            | LongPressed | toggle_barseconds       |
| 4          | LongPressed | main_mode               |
| (Inactive) | 60s         | main_mode               |
