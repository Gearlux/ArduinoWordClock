My first idea for a three button config is here

| Button | Event       | Command           |
|--------|-------------|-------------------|
| 1      | Clicked     | show_seconds      |
|        | LongPressed | clock_mode        |
| 2      | Clicked     | toggle_brightness |
|        | LongPressed | brightness_mode   |
| 3      | Clicked     | toggle_rgb        |
|        | LongPressed | config_mode       |

For the *clock_mode*

| Button     | Event       | Command           |
|------------|-------------|-------------------|
| 1          | Clicked     | increase_hour     |
|            | LongPressed | decrease_hour     |
| 2          | Clicked     | increase_5min     |
|            | LongPressed | decrease_5min     |
| 3          | Clicked     | increase_1min     |
|            | LongPressed | decrease_1min     |
| (Inactive) | 60s         | main_mode         |

In clock_mode, the seconds will go around the outer circle of the clock.

For the *brightness mode*

| Button     | Event       | Command                 |
|------------|-------------|-------------------------|
| 1          | Clicked     | brightness_down         |
| 2          | Clicked     | toggle_brightness_input |
|            | LongPressed | main_mode               |
| 3          | Clicked     | brightness_up           |
| (Inactive) | 60s         | main_mode               |

For the *config mode*

| Button     | Event       | Command                 |
|------------|-------------|-------------------------|
| 1          | Clicked     | toggle_it_is            |
|            | LongPressed | toggle_twenty           |
| 2          | Clicked     | toggle_over             |
|            | LongPressed | toggle_dot              |
| 3          | Clicked     | toggle_backlight        |
|            | LongPressed | main_mode               |
| (Inactive) | 60s         | main_mode               |
