# User Interface

## Visual Studio Code

After building the prototype, it's time to get serious about Arduino development.
Let's start by setting up [Visual Studio Code](https://github.com/microsoft/vscode-arduino)

<div class="panel panel-warning">
**Warning**
{: .panel-heading}
<div class="panel-body">

Don't use the Windows Store's version of the Arduino IDE.

</div>
</div>

I had some troubles configuring the Arduino tools, most likely because I installed the Windows Store's version and already installed the extension. I had to manually add the paths to the Visual Studio Code. The "Board Type" can be configured with "Ctrl+Shift+P > Arduino: Change board type".
I also changed the default baud rate to 9600 and did not go for the cli (yet).

Changing ino files was a little bit difficult. It seems that with each workspace, only one ino file is associated.
The easiest way to change your main ino file is editing the .vscode/arduino.json file, deleting the "sketch" entry and manually choosing the sketch file when clicking "verify" or "upload".

## Keep it simple

Let's simplify the board first and remove the thermometer - a clock is not a thermometer - and remove one of the buttons.

My first idea for a three button config is here

| Button | Event       | Command           |
|--------|-------------|-------------------|
| 1      | Clicked     | show_seconds      |
|        | LongPressed | clock_mode        |
| 2      | Clicked     | toggle_brightness |
|        | LongPressed | brightness_mode   |
| 3      | Clicked     | toggle_rgb        |
|        | LongPressed | toggle_itis       |

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








