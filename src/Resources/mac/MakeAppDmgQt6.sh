#!/bin/bash
set -ev

echo "About to create dmg file and fix up"
mkdir GoldenCheetah.app/Contents/Frameworks

# This is a hack to include libicudata.*.dylib, not handled by macdployqt[fix]
cp /usr/local/opt/icu4c/lib/libicudata.*.dylib GoldenCheetah.app/Contents/Frameworks

# Copy python framework and change permissions to fix paths
cp -R /Library/Frameworks/Python.framework GoldenCheetah.app/Contents/Frameworks
chmod -R +w GoldenCheetah.app/Contents/Frameworks
# Update deployed Python framework path
install_name_tool -id @executable_path/../Frameworks/Python.framework/Versions/3.7/Python ./GoldenCheetah.app/Contents/Frameworks/Python.framework/Versions/3.7/Python
# Update GoldenCheetah binary to reference deployed lib
install_name_tool -change /Library/Frameworks/Python.framework/Versions/3.7/Python @executable_path/../Frameworks/Python.framework/Versions/3.7/Python ./GoldenCheetah.app/Contents/MacOS/GoldenCheetah
# Update Python binary to reference deployed lib instead of the Cellar one
OLD_PATH=`otool -L GoldenCheetah.app/Contents/Frameworks/Python.framework/Versions/3.7/bin/python3.7 | grep "Library" | cut -f 1 -d ' '`
echo $OLD_PATH
install_name_tool -change $OLD_PATH "@executable_path/../Python" GoldenCheetah.app/Contents/Frameworks/Python.framework/Versions/3.7/bin/python3.7
install_name_tool -change $OLD_PATH "@executable_path/../../../../Python" GoldenCheetah.app/Contents/Frameworks/Python.framework/Versions/3.7/Resources/Python.app/Contents/MacOS/Python
# Add mandatory Python dependencies
rm -r GoldenCheetah.app/Contents/Frameworks/Python.framework/Versions/3.7/lib/python3.7/site-packages
cp -R ../../site-packages GoldenCheetah.app/Contents/Frameworks/Python.framework/Versions/3.7/lib/python3.7

# Initial deployment using macdeployqt
macdeployqt GoldenCheetah.app -verbose=2 -executable=GoldenCheetah.app/Contents/MacOS/GoldenCheetah

# Final deployment to generate dmg
macdeployqt GoldenCheetah.app -verbose=2 -fs=hfs+ -dmg

echo "Renaming dmg file to branch and build number ready for deploy"
export FINAL_NAME=GoldenCheetah_v3.7_x64Qt6.dmg
mv GoldenCheetah.dmg $FINAL_NAME
ls -l $FINAL_NAME

# Add version information, last commit message and SHA256 to txt file
echo "Generating companion txt file"
GoldenCheetah.app/Contents/MacOS/GoldenCheetah --version 2>GCversionMacOSQt6.txt
git log -1 >> GCversionMacOSQt6.txt
echo "SHA256 hash of $FINAL_NAME:" >> GCversionMacOSQt6.txt
shasum -a 256 $FINAL_NAME | cut -f 1 -d ' ' >> GCversionMacOSQt6.txt
cat GCversionMacOSQt6.txt
