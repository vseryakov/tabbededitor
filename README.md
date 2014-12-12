# Tabbed Editor for Qt Creator 3.x

This is a fork of the project http://sourceforge.net/projects/tabbededitor/

# Build

Provide the QTC sources and where it produced its plugins, for example

        qmake QTC_SOURCE=../QtCreator IDE_BUILD_TREE=../QtCreator/build

# Add to installed Qt Enterprise version

 - Compile Qt Creator form sources with the same version as installed 
   - git clone https://gitorious.org/qt-creator/qt-creator.git QtCreator
   - git checkout 3.3
   - mkdir build && cd build && qmake -r ../qtcreator.pro
 - Compile and copy the plugin into Plugins/ of the installed Qt Creator app
 - Update paths using this shell script on Mac OS X:

	    f=libTabbedEditor.dylib	
	    q=`qmake -v|grep Using|awk {'print $6}'`
	    for l in `otool -L $f | grep $q | awk '{print $1}'`; do p=`echo $l | awk -v q="$q" '{print "@executable_path/../Frameworks"substr($1,length(q)+1)}'`; install_name_tool -change $l $p $f; done


# Author
Vlad Seryakov
