@echo off

if %1/==/ goto usage
if %2/==/ goto usage

gawk -f "%~dp0tab.awk" -v select=%2 box=%3 cross=%4 %5 %6 %1

goto end

:usage

echo "Usage: tab <abc file> <tune number> [<box key(s)>] [<cross fingering>]"

:end
