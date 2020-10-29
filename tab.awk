#!/bin/awk
###############################################################################
#
#  tab.awk  1.1  18 January 2007
#
#  Awk script to add melodeon tablature to ABC format music
#
#  Copyright (C) 2007 Bill Farmer
#
#  Version 1.1  9 August 2007  Change continuation line processing.
#
#  This awk script adds melodeon tablature to a selected tune from an
#  input file in ABC format. See Chris Walshaw's ABC pages for an
#  explanation of ABC. The tablature is added as two lines of vocals after
#  each line of music. The abc2ps conversion program will automagically
#  line up each word of vocals under a note in the line of generated
#  music. If the word begins with a number, it is assumed to be a verse
#  number and is offset to the left. To prevent this, and to draw a line
#  between the two rows of numbers, I have created a modified version of
#  abc2ps.
#
#  For an explanation of melodeon tablature see Bernard Loffet's web site
#  http://www.diato.org/exptab2.htm.
#
#  This script can be run from a two line batch file tab.bat:
#
#    @echo off
#    gawk -f tab.awk -v select=%2 box=%3 cross=%4 %5 %6 %1
#
#  The command line is:
#
#    tab <input abc file> <tune number> [<box key(s)>] [<cross fingering>]
#
#  The input file looks like this:
#
#  X:12
#  T:Scottich
#  M:4/4
#  L:1/4
#  K:G
#  "G"BGBd|"C"c2c2|"G"BGBd|"D"A2GA|\
#  "G"BGBd|"C"cBAc|"G"BGBd|1"D"A2GA:|2"D"A2GF|]
#
#  The output from the command line:
#
#    tab tunes.abc 12
#
#  without cross fingering looks like this:
#
#  X:12
#  T:Scottich
#  M:4/4
#  L:1/4
#  K:G
#  "G"BGBd|"C"c2c2|"G"BGBd|"D"A2GA|\
#  w:4' 3' 4' 5'|**|4' 3' 4' 5'|*3'*|
#  w:****|4' 4'|****|3'*3'|
#  "G"BGBd|"C"cBAc|"G"BGBd|1"D"A2GA:|2"D"A2GF|]
#  w:4' 3' 4' 5'|*4'**|4' 3' 4' 5'|*3'*|*3'*|
#  w:****|4'*3' 4'|****|3'*3'|3'*2'|
#
#  The first four bars of tablature will look like this:
#
#  |><  4' 3' 4' 5'       4' 3' 4' 5'    3'
#  +-------------------------------------------
#  |<>              4' 4'             3'    3'
#
#  The output from the command line:
#
#    tab tunes.abc 12 y
#
#  with cross fingering looks like this:
#
#  X:12
#  T:Scottich
#  M:4/4
#  L:1/4
#  K:G
#  "G"BGBd|"C"c2c2|"G"BGBd|"D"A2GA|\
#  w:4' 3' 4' 5'|**|4' 3' 4' 5'|***|
#  w:****|4' 4'|****|3' 4 3'|
#  "G"BGBd|"C"cBAc|"G"BGBd|1"D"A2GA:|2"D"A2GF|]
#  w:4' 3' 4' 5'|****|4' 3' 4' 5'|***|***|
#  w:****|4' 5 3' 4'|****|3' 4 3'|3' 4 2'|
#
#  The first four bars of tablature will look like this:
#
#  |><  4' 3' 4' 5'       4' 3' 4' 5'
#  +-------------------------------------------
#  |<>              4' 4'             3' 4  3'
#
#  The algorithm for generating tablature without cross fingering is
#  fairly simple:
#
#  Find the home row from the key, then
#    Look for each note
#      first on the home row on the push
#      then on the home row on the pull
#      then on the other row on the push
#      then on the other row on the pull.
#
#  The algorithm for cross fingering is more complex:
#
#  Find the home row from the key, then
#    For each bar
#      Work out if pushing or pulling
#	 first from the chord
#	 then from the first note
#
#      Look for each note in the bar
#	 If pushing
#	   first on the home row on the push
#	   then on the other row on the push
#	   then on the home row on the pull
#	   then on the other row on the pull.
#
#	 If pulling
#	   first on the home row on the pull
#	   then on the other row on the pull.
#	   then on the home row on the push
#	   then on the other row on the push
#
###############################################################################

BEGIN {
    FS=":";

#   Default to D/G box

    box = (box)? box: "DG";

#   One row in C

    if (box ~ /^C$/)
    {
#	One row in C notes

	split("E-/G-/C/E/G/c/e/g/c+/e+", push, "/");
	split("G-/B-/D/F/A/B/d/f/a/b/", pull, "/");

	for (i in push)
	{
	    pushnotes[push[i],"C"] = i;
	    pullnotes[pull[i],"C"] = i;
	}

#	One row in C chords

	split("C/F", push, "/");
	split("G/F", pull, "/");

	for (i in push)
	{
	    pushchords[push[i],"C"] = i;
	    pullchords[pull[i],"C"] = i;
	}
    }

#   One row in D

    else if (box ~ /^D$/)
    {
#	One row in D notes

	split("F-/A-/D/F/A/d/f/a/d+/f+/a+", push, "/");
	split("B-/C/E/G/B/c/e/g/b/c+/e+", pull, "/");

	for (i in push)
	{
	    pushnotes[push[i],"D"] = i;
	    pullnotes[pull[i],"D"] = i;
	}

#	One row in C chords

	split("D/G", push, "/");
	split("A/G", pull, "/");

	for (i in push)
	{
	    pushchords[push[i],"D"] = i;
	    pullchords[pull[i],"D"] = i;
	}
    }

#   One row in G

    else if (box ~ /^G$/)
    {
#	One row in G notes

	split("B-/D/G/B/d/g/b/d+/g+/b+", push, "/");
	split("D/F/A/c/e/f/a/c+/e+/f+", pull, "/");

	for (i in push)
	{
	    pushnotes[push[i],"G"] = i;
	    pullnotes[pull[i],"G"] = i;
	}

#	One row in G chords

	split("G/C", push, "/");
	split("D/C", pull, "/");

	for (i in push)
	{
	    pushchords[push[i],"G"] = i;
	    pullchords[pull[i],"G"] = i;
	}
    }

#   Two row in G/C

    else if (box ~ /^G.*C$/)
    {
#	Two row in G/C C row notes

	split("E-/G-/C/E/G/c/e/g/c+/e+", push, "/");
	split("G-/B-/D/F/A/B/d/f/a/b/", pull, "/");

	for (i in push)
	{
	    pushnotes[push[i],"C"] = i "'";
	    pullnotes[pull[i],"C"] = i "'";
	}

#	Accidentals on the G row

	pullnotes["F#-","C"] = "2";
	pullnotes["F#","C"] = "6";
	pullnotes["f#","C"] = "10";

#	Two row in G/C C row chords

	split("C/F/E/G", push, "/");
	split("G/F/A/D", pull, "/");

	for (i in push)
	{
	    pushchords[push[i],"C"] = i;
	    pullchords[pull[i],"C"] = i;
	}

#	Two row in G/C G row notes

	split("B--/D-/G-/B-/D/G/B/d/g/b/d+", push, "/");
	split("D-/F-/A-/C/E/F/A/c/e/f/a", pull, "/");

	for (i in push)
	{
	    pushnotes[push[i],"G"] = i;
	    pullnotes[pull[i],"G"] = i;
	}

#	Accidentals on the C row

	pullnotes["F","G"] = "4'";
	pullnotes["f","G"] = "8'";

#	Two row in G/C G row chords

	split("C/F/E/G", push, "/");
	split("G/F/A/D", pull, "/");

	for (i in push)
	{
	    pushchords[push[i],"G"] = i;
	    pullchords[pull[i],"G"] = i;
	}
    }

#   Two row in D/G

    else if (box ~ /^D.*G$/)
    {
#	Two row in D/G G row notes

	split("f=/D/G/B/d/g/b/d+/g+/b+", push, "/");
	split("eb/F/A/c/e/f/a/c+/e+/f+", pull, "/");

	for (i in push)
	{
	    pushnotes[push[i],"G"] = i "'";
	    pullnotes[pull[i],"G"] = i "'";
	}

#	Accidentals on the D row

	pullnotes["C#","G"] = "1";
	pullnotes["c#","G"] = "6";
	pullnotes["c#+","G"] = "10";
	pushnotes["B-","G"] = "1";

#	Two row in D/G G row chords

	split("G/C/B/D", push, "/");
	split("D/C/E/A", pull, "/");

	for (i in push)
	{
	    pushchords[push[i],"G"] = i;
	    pullchords[pull[i],"G"] = i;
	}

#	Two row in D/G D row notes

	split("G#/A-/D/F/A/d/f/a/d+/f+/a+", push, "/");
	split("Bb/C/E/G/B/c/e/g/b/c+/e+", pull, "/");

	for (i in push)
	{
	    pushnotes[push[i],"D"] = i;
	    pullnotes[pull[i],"D"] = i;
	}

#	Accidental on the G row

	pullnotes["c=","D"] = "4'";
	pullnotes["c=+","D"] = "8'";
	pushnotes["F-","D"] = "1";

#	Two row in D/G D row chords

	split("G/C/B/D", push, "/");
	split("D/C/E/A", pull, "/");

	for (i in push)
	{
	    pushchords[push[i],"D"] = i;
	    pullchords[pull[i],"D"] = i;
	}
    }

#   Two row in D/G

    else if (box ~ /^C.*F$/)
    {
#	Two row in C/F F row notes

	split("eb/C/F/A/c/f/a/c+/f+/a+", push, "/");
	split("c#/E/G/B/d/e/g/b/d+/e+", pull, "/");

	for (i in push)
	{
	    pushnotes[push[i],"F"] = i "'";
	    pullnotes[pull[i],"F"] = i "'";
	}

#	Accidentals on the C row

	pullnotes["Ab","F"] = "1";
	pullnotes["B=","F"] = "6";
	pullnotes["b=","F"] = "10";
	pushnotes["F#","F"] = "1";

#	Two row in C/F F row chords

	split("F/B/A/C", push, "/");
	split("C/B/D/G", pull, "/");

	for (i in push)
	{
	    pushchords[push[i],"F"] = i;
	    pullchords[pull[i],"F"] = i;
	}

#	Two row in C/F C row notes

	split("F#/G-/C/E/G/c/e/g/c+/e+/g+", push, "/");
	split("Ab/B-/D/F/A/B/d/f/a/b/d+", pull, "/");

	for (i in push)
	{
	    pushnotes[push[i],"C"] = i;
	    pullnotes[pull[i],"C"] = i;
	}

#	Accidental on the F row

	pullnotes["Bb","C"] = "4'";
	pullnotes["bb+","C"] = "8'";

#	Two row in C/F C row chords

	split("F/B/A/C", push, "/");
	split("C/B/D/G", pull, "/");

	for (i in push)
	{
	    pushchords[push[i],"C"] = i;
	    pullchords[pull[i],"C"] = i;
	}
    }
}

/^X:/ { # Only process the selected tune
    processing = (select == $2);
}

/^X:/, /^$/ { # Output the tune
    if (processing)
	print;
}

/^H:/ && processing { # History, ignore any following lines
    do {
	getline;
	print;
    }
    while ($0 !~ /^[A-Za-z]:/);
}

/^%%begintext/ && processing { # Text, ignore until endtext
    while ($0 !~ /^%%endtext/)
    {
	getline;
	print;
    }
}

/^%/ { # Ignore comments
    next;
}

/^$/ { # Stop processing on a blank line
    processing = 0;
}

/^K:/ && processing { # Extract the key
    key = $2;

#   Remove leading and trailing blanks, etc

    sub(/^ +/, "", key);
    sub(/ +.*$/, "", key);

#   One row in C, or two row in G/C, C row

    if ((key ~ /^C/) || (key ~ /^Am/))
    {
	row = "C";
	alt = "G";
    }

#   One row in G, or two row in D/G, G row

    else if ((key ~ /^G/) || (key ~ /^Em/))
    {
	row = "G";
	alt = "D";
    }

#   One row in D, or two row in D/G, D row

    else if ((key ~ /^D/) || (key ~ /^Bm/))
    {
	row = "D";
	alt = "G";
    }

#   Check if minor tune

    minor = ((key ~ /^Am/) || (key ~ /^Em/) || (key ~ /^Bm/))? 1: 0;
}

/^[A-Za-z]:/ { # Ignore any other info
    next;
}

processing { # Process a line of music

#   Initialise the output lines

    line1 = "w:";
    line2 = "w:";

#   Assume a bar at the beginning of the line,
#   unless this is a continuation line

    if (!continuation)
	bar = 1;

    n = split($0, a, "");

    for (i = 1; i <= n; i++)
    {
#	Get each character

	c = a[i];
	c1 = a[i + 1];
	c2 = a[i + 2];

#	Check for continuation

	if (c == "\\")
	{
	    continuation = 1;
	    break;
	}

#	Ignore comments

	else if (c == "%")
	    break;

#	Check for a new bar

	else if (c == "|")
	{
# 	    Reset continuation

	    continuation = 0;

#	    Reset blank flags

	    last1 = 0;
	    last2 = 0;

#	    Copy only one bar line to the output

	    if (!bar)
	    {
		line1 = line1 c;
		line2 = line2 c;
	    }

	    bar = 1;
	}

#	Check for accidentals

	else if (c == "^")
	    accidental = "#";

	else if (c == "_")
	    accidental = "b";

	else if (c == "=")
	    accidental = "=";

#	Check for quotes

	else if (c == "\"")
	    quote = (quote)? 0: 1;

#	Get the current chord

	else if ((c ~ /[A-G]/) && quote)
	    chord = c;

#	Check for grace note

	else if (c == "{")
	    grace = 1;

	else if (c == "}")
	    grace = 0;

#	Check for chords, escape sequence

	else if (c == "[")
	{
	    if ((c1 ~ /[A-Za-z]/) && (c2 == ":"))
		escape = 1;

	    else
		chords = 1;
	}

	else if (c == "]")
	{
	    chords = 0;
	    escape = 0;
	}

#	Process notes

	else if ((c ~ /[A-Ga-g]/) && !quote && !escape)
	{
#	    Check for octave characters

	    if (c1 == "'")
		octave = "+";

	    else if (c1 == ",")
		octave = "-";

#	    Build the current note

	    note = c accidental octave;
	    accidental = "";
	    octave = "";

#	    One row fingering

	    if (!cross)
	    {
#		Not interested in bars for one row

		bar = 0;

#		Check if a push note

		if ((note,row) in pushnotes)
		{
#		    Only include a blank if flagged

		    if (last1)
			line1 = line1 " " pushnotes[note,row];

		    else
			line1 = line1 pushnotes[note,row];

		    if (!grace && !chords)
			line2 = line2 "*";

#		    Set the blank flags

		    if (!grace && !chords)
			last1 = 1;

		    else
			last1 = 0;

		    last2 = 0;
		}

#		Check if a pull note

		else if ((note,row) in pullnotes)
		{
		    if (!grace && !chords)
			line1 = line1 "*";

#		    Only include a blank if flagged

		    if (last2)
			line2 = line2 " " pullnotes[note,row];

		    else
			line2 = line2 pullnotes[note,row];

#		    Set the blank flags

		    last1 = 0;

		    if (!grace && !chords)
			last2 = 1;

		    else
			last2 = 0;
		}

#		Check for a push on the other row

		else if ((note,alt) in pushnotes)
		{
#		    Only include a blank if flagged

		    if (last1)
			line1 = line1 " " pushnotes[note,alt];

		    else
			line1 = line1 pushnotes[note,alt];

		    if (!grace && !chords)
			line2 = line2 "*";

#		    Set the blank flags

		    if (!grace && !chords)
			last1 = 1;

		    else
			last1 = 0;

		    last2 = 0;
		}

#		Check for a pull on the other row

		else if ((note,alt) in pullnotes)
		{
		    if (!grace && !chords)
			line1 = line1 "*";

#		    Only include a blank if flagged

		    if (last2)
			line2 = line2 " " pullnotes[note,alt];

		    else
			line2 = line2 pullnotes[note,alt];

#		    Set the blank flags

		    last1 = 0;

		    if (!grace && !chords)
			last2 = 1;

		    else
			last2 = 0;
		}

#		Note not on the box

		else
		{
#		    Only include a blank if flagged

		    if (last1)
			line1 = line1 " ?";

		    else
			line1 = line1 "?";

		    if (!grace && !chords)
			line2 = line2 "*";

#		    Set the blank flags

		    if (!grace && !chords)
			last1 = 1;

		    else
			last1 = 0;

		    last2 = 0;
		}
	    }

#	    Cross row fingering

	    else
	    {
#		Every new bar work out if pushing or pulling

		if (bar)
		{
#		    If minor tune try pulling first

		    if (minor)
		    {
#			If the chord is available on the pull and not
#			on the push

			if (((chord,alt) in pullchords) &&
			    !((chord,alt) in pushchords))
			    pushing = 0;

#			If the chord is available on the push and not
#			on the pull

			else if  (((chord,alt) in pushchords) &&
			    !((chord,alt) in pullchords))
			    pushing = 1;

#			If the note is available on the alternate row
#			on the pull

			else if ((note,alt) in pullnotes)
			    pushing = 0;

#			Otherwise

			else
			    pushing = 1;
		    }

#		    If major tune try pushing first

		    else
		    {
#			If the chord is available on the push and not
#			on the pull

			if  (((chord,row) in pushchords) &&
			    !((chord,row) in pullchords))
			    pushing = 1;

#			If the chord is available on the pull and not
#			on the push

			else if (((chord,row) in pullchords) &&
			    !((chord,row) in pushchords))
			    pushing = 0;

#			If the note is available on the selected row
#			on the push

			else if ((note,row) in pushnotes)
			    pushing = 1;

#			Otherwise

			else
			    pushing = 0;
		    }

#		    Reset the bar flag

		    bar = 0;
		}

#		Pushing

		if (pushing)
		{
#		    If the note is available on the selected row
#		    on the push

		    if ((note,row) in pushnotes)
		    {
#			Only include a blank if flagged

			if (last1)
			    line1 = line1 " " pushnotes[note,row];

			else
			    line1 = line1 pushnotes[note,row];

			if (!grace && !chords)
			    line2 = line2 "*";

#			Set the blank flags

			if (!grace && !chords)
			    last1 = 1;

			else
			    last1 = 0;

			last2 = 0;
		    }

#		    If the note is available on the alternate row
#		    on the push

		    else if ((note,alt) in pushnotes)
		    {
#			Only include a blank if flagged

			if (last1)
			    line1 = line1 " " pushnotes[note,alt];

			else
			    line1 = line1 pushnotes[note,alt];

			if (!grace && !chords)
			    line2 = line2 "*";

#			Set the blank flags

			if (!grace && !chords)
			    last1 = 1;

			else
			    last1 = 0;

			last2 = 0;
		    }

#		    If the note is available on the selected row
#		    on the pull

		    else if ((note,row) in pullnotes)
		    {
			if (!grace && !chords)
			    line1 = line1 "*";

#			Only include a blank if flagged

			if (last2)
			    line2 = line2 " " pullnotes[note,row];

			else
			    line2 = line2 pullnotes[note,row];

#			Set the blank flags

			last1 = 0;

			if (!grace && !chords)
			    last2 = 1;

			else
			    last2 = 0;
		    }

#		    If the note is available on the alternate row
#		    on the pull

		    else if ((note,alt) in pullnotes)
		    {
			if (!grace && !chords)
			    line1 = line1 "*";

#			Only include a blank if flagged

			if (last2)
			    line2 = line2 " " pullnotes[note,alt];

			else
			    line2 = line2 pullnotes[note,alt];

#			Set the blank flags

			last1 = 0;

			if (!grace && !chords)
			    last2 = 1;

			else
			    last2 = 0;
		    }

#		    Note not on the box

		    else
		    {
#			Only include a blank if flagged

			if (last1)
			    line1 = line1 " ?";

			else
			    line1 = line1 "?";

			if (!grace && !chords)
			    line2 = line2 "*";

#			Set the blank flags

			if (!grace && !chords)
			    last1 = 1;

			else
			    last1 = 0;

			last2 = 0;
		    }
		}

#		Pulling

		else
		{
#		    If the note is available on the selected row
#		    on the pull

		    if ((note,row) in pullnotes)
		    {
			if (!grace && !chords)
			    line1 = line1 "*";

#			Only include a blank if flagged

			if (last2)
			    line2 = line2 " " pullnotes[note,row];

			else
			    line2 = line2 pullnotes[note,row];

#			Set the blank flags

			last1 = 0;

			if (!grace && !chords)
			    last2 = 1;

			else
			    last2 = 0;
		    }

#		    If the note is available on the alternate row
#		    on the pull

		    else if ((note,alt) in pullnotes)
		    {
			if (!grace && !chords)
			    line1 = line1 "*";

#			Only include a blank if flagged

			if (last2)
			    line2 = line2 " " pullnotes[note,alt];

			else
			    line2 = line2 pullnotes[note,alt];

#			Set the blank flags

			last1 = 0;

			if (!grace && !chords)
			    last2 = 1;

			else
			    last2 = 0;
		    }

#		    If the note is available on the selected row
#		    on the push

		    else if ((note,row) in pushnotes)
		    {
#			Only include a blank if flagged

			if (last1)
			    line1 = line1 " " pushnotes[note,row];

			else
			    line1 = line1 pushnotes[note,row];

			if (!grace && !chords)
			    line2 = line2 "*";

#			Set the blank flags

			if (!grace && !chords)
			    last1 = 1;

			else
			    last1 = 0;

			last2 = 0;
		    }

#		    If the note is available on the alternate row
#		    on the push

		    else if ((note,alt) in pushnotes)
		    {
#			Only include a blank if flagged

			if (last1)
			    line1 = line1 " " pushnotes[note,alt];

			else
			    line1 = line1 pushnotes[note,alt];

			if (!grace && !chords)
			    line2 = line2 "*";

#			Set the blank flags

			if (!grace && !chords)
			    last1 = 1;

			else
			    last1 = 0;

			last2 = 0;
		    }

#		    Note not on the box

		    else
		    {
#			Only include a blank if flagged

			if (last1)
			    line1 = line1 " ?";

			else
			    line1 = line1 "?";

			if (!grace && !chords)
			    line2 = line2 "*";

#			Set the blank flags

			if (!grace && !chords)
			    last1 = 1;

			else
			    last1 = 0;

			last2 = 0;
		    }
		}
	    }
	}
    }

#   Output the two lines of tablature

    print line1;
    print line2;
}
