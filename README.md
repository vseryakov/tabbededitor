# Tabbed Editor for Qt Creator 3.x

This is a fork of the project http://sourceforge.net/projects/tabbededitor/

# Build

Provide the QTC sources and where it produced its plugins, for example

	qmake QTC_SOURCE=../QtCreator IDE_BUILD_TREE=$HOME/Developer/Qt/qtcreator-Desktop_Qt_5_4_0_clang_64bit-Release/

# Add to installed enterprise version

 - Compile Qt Creator form sources with the same version as installed 
   - git checkout 3.3
   - mkdir build && cd build && qmake -r ../qtcreator.pro
 - Copy the plugin into Plugins of the installed Qt Creator
 - Update paths using this shell script on Mac OS X:

	    f=libTabbedEditor.dylib	
	    q=`qmake -v|grep Using|awk {'print $6}'`
	    for l in `otool -L $f | grep $q | awk '{print $1}'`; do p=`echo $l | awk -v q="$q" '{print "@executable_path/../Frameworks"substr($1,length(q)+1)}'`; install_name_tool -change $l $p $f; done


# Author
Vlad Seryakov
