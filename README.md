# abc2ps
Updated version of abc2ps plus scripts

The scripts `tab.sh`, `tab.bat`, and `tab.awk`, in conjunction with the updated version of `abc2ps.c` produce music notation with added melodeon tablature from abc source files.

```shell
Syntax: $0 <abc file> <tune number> [<box key(s)>] [<cross fingering>]
```
This awk script adds melodeon tablature to a selected tune from an
input file in ABC format. See Chris Walshaw's ABC pages for an
explanation of ABC. The tablature is added as two lines of vocals after
each line of music. The abc2ps conversion program will automagically
line up each word of vocals under a note in the line of generated
music. If the word begins with a number, it is assumed to be a verse
number and is offset to the left. To prevent this, and to draw a line
between the two rows of numbers, I have created a modified version of
abc2ps.

For an explanation of melodeon tablature see Bernard Loffet's web site
http://www.diato.org/exptab2.htm.

This script can be run from a two line batch file tab.bat:

  @echo off
  gawk -f tab.awk -v select=%2 box=%3 cross=%4 %5 %6 %1

The command line is:

  tab <input abc file> <tune number> [<box key(s)>] [<cross fingering>]

The input file looks like this:
```abc
X:12
T:Scottich
M:4/4
L:1/4
K:G
"G"BGBd|"C"c2c2|"G"BGBd|"D"A2GA|\
"G"BGBd|"C"cBAc|"G"BGBd|1"D"A2GA:|2"D"A2GF|]
```
The output from the command line:

  tab tunes.abc 12

without cross fingering looks like this:
```abc
X:12
T:Scottich
M:4/4
L:1/4
K:G
"G"BGBd|"C"c2c2|"G"BGBd|"D"A2GA|\
w:4' 3' 4' 5'|**|4' 3' 4' 5'|*3'*|
w:****|4' 4'|****|3'*3'|
"G"BGBd|"C"cBAc|"G"BGBd|1"D"A2GA:|2"D"A2GF|]
w:4' 3' 4' 5'|*4'**|4' 3' 4' 5'|*3'*|*3'*|
w:****|4'*3' 4'|****|3'*3'|3'*2'|
```
The first four bars of tablature will look like this:
```
|><  4' 3' 4' 5'       4' 3' 4' 5'    3'
+-------------------------------------------
|<>              4' 4'             3'    3'
```
The output from the command line:

  tab tunes.abc 12 y

with cross fingering looks like this:
```abc
X:12
T:Scottich
M:4/4
L:1/4
K:G
"G"BGBd|"C"c2c2|"G"BGBd|"D"A2GA|\
w:4' 3' 4' 5'|**|4' 3' 4' 5'|***|
w:****|4' 4'|****|3' 4 3'|
"G"BGBd|"C"cBAc|"G"BGBd|1"D"A2GA:|2"D"A2GF|]
w:4' 3' 4' 5'|****|4' 3' 4' 5'|***|***|
w:****|4' 5 3' 4'|****|3' 4 3'|3' 4 2'|
```
The first four bars of tablature will look like this:
```
|><  4' 3' 4' 5'       4' 3' 4' 5'
+-------------------------------------------
|<>              4' 4'             3' 4  3'
```
The algorithm for generating tablature without cross fingering is
fairly simple:
```
Find the home row from the key, then
  Look for each note
    first on the home row on the push
    then on the home row on the pull
    then on the other row on the push
    then on the other row on the pull.
```
The algorithm for cross fingering is more complex:
```
Find the home row from the key, then
  For each bar
    Work out if pushing or pulling
      first from the chord
      then from the first note

    Look for each note in the bar
      If pushing
	      first on the home row on the push
	      then on the other row on the push
	      then on the home row on the pull
	      then on the other row on the pull.

      If pulling
	      first on the home row on the pull
	      then on the other row on the pull.
	      then on the home row on the push
	      then on the other row on the push
```
