#!/bin/sh

# This shell script installs symbolic links in icon theme directories
# $1 is the source directory
# $2 is the install directory

# Reference theme names
ref_theme1=gnome-theme
ref_theme2=kde-theme

# Link theme names
link_theme1=default-theme
link_theme2=xfce-theme

echo Installing icon links...

# Loop on PNG files
for iconfile in $1/icons/$ref_theme1/*.png
do

	# Get icon name
	iconname=`basename "$iconfile"`

	# Install a link if the icon does not exist in the source theme directory

	if test ! -f $1/icons/$link_theme1/$iconname
	then
		echo ln -s -f ../$ref_theme1/$iconname $2/icons/$link_theme1/$iconname;
		ln -s -f ../$ref_theme1/$iconname $2/icons/$link_theme1/$iconname
	fi

	if test ! -f $1/icons/$link_theme2/$iconname
	then
		echo ln -s -f ../$ref_theme1/$iconname $2/icons/$link_theme2/$iconname;
		ln -s -f ../$ref_theme1/$iconname $2/icons/$link_theme2/$iconname
	fi

done
