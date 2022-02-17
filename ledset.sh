#!/bin/sh
# Configure device
stty -F /dev/ttyACM0 -hup || sleep 1
stty -F /dev/ttyACM0 ospeed 115200 || sleep 1

# Check for keywords
case "$@" in
  red) ARD_CMD='$S#FF0000';;
  deep_orange) ARD_CMD='$S#FF1F00';;
  orange) ARD_CMD='$S#FF2F00';;
  gold) ARD_CMD='$S#FF4F00';;
  yellow) ARD_CMD='$S#FF7F00';;
  lime) ARD_CMD='$S#FFFF00';;
  green) ARD_CMD='$S#00FF00';;
  aqua | cyan) ARD_CMD='$S#00FF7F';;
  sky_blue) ARD_CMD='$S#00FFFF';;
  azure) ARD_CMD='$S#007FFF';;
  blue) ARD_CMD='$S#0000FF';;
  deep_purple) ARD_CMD='$S#3F00FF';;
  purple) ARD_CMD='$S#7F00FF';;
  magenta | fuchsia) ARD_CMD='$S#FF00FF';;
  pink) ARD_CMD='$S#FF007F';;
  hot_pink) ARD_CMD='$S#FF003F';;

  cool_white | max) ARD_CMD='$S#FFFFFF';;
  white) ARD_CMD='$S#FFAF7F';;
  warm_white) ARD_CMD='#S#FF7F3F';;
  black | off) ARD_CMD='$S#000000';;
  fluorescent) ARD_CMD='$S#FF4F13';;
  incandescent) ARD_CMD='$S#FF3803';;
  rainbow) ARD_CMD='$H#FF';;
  *) ARD_CMD="$@"
esac

# Send command
echo -n "$ARD_CMD" > /dev/ttyACM0
