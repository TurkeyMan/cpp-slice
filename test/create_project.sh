# prompt user of no option given on the command line
if [[ $1 ]] ; then
	ACTION=$1
else
	echo "Select the type of project you would like to create:"
	echo "1. GNU Makefile"
	echo "2. Visual Studio 2015 Solution"
	echo "3. CodeLite"
	read -p "[1-4] " -n 1 -r
	echo
	ACTION=$REPLY
fi

if [ $OSTYPE == "msys" ]; then # Windows, MingW
	PREMAKE=bin/premake/premake5.exe
else
	PREMAKE=bin/premake/premake5
fi

# parse string actions into numerics
if [[ $ACTION == "make" ]] || [[ $ACTION == "gmake" ]] ; then
	ACTION="1"
elif [[ $ACTION == "codelite" ]] ; then
	ACTION="3"
fi

# perform action
if [[ $ACTION == "1" ]] ; then
	echo "Creating GNU Makefile..."
	$PREMAKE gmake
elif [[ $ACTION == "2" ]] ; then
	echo "Creating VS2015 Project..."
	$PREMAKE vs2015
elif [[ $ACTION == "3" ]] ; then
	echo "Creating CodeLite project..."
	$PREMAKE codelite
else
	echo "Invalid input: "$REPLY
fi
