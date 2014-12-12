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
 - Update paths using this shell script

        rpath=`qmake -v|grep Using|awk {'print $6}'`
	    for lib in `otool -L $1 |grep "$2" | awk '{print $1}'`; do
  	        mod=`echo $lib | awk -v v="$rpath" '{print "@executable_path/../Frameworks"substr($1,length(v)+1)}'`
    	    install_name_tool -change $lib $mod $1
        done


# Author
Vlad Seryakov
