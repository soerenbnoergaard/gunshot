#!/bin/bash
#
# set -e
#
# if [ -d bin ]; then
#   cd bin
# else
#   echo "Please run this script from the root folder"
#   exit
# fi
#
# DPF_DIR=../dpf/
# PLUGINS=$(ls | grep vst.dylib)
#
# rm -rf *.vst/
#
# for i in $PLUGINS; do
#   BUNDLE=$(echo ${i} | awk 'sub("-vst.dylib","")')
#   cp -r ${DPF_DIR}/utils/plugin.vst/ ${BUNDLE}.vst
#   mv ${i} ${BUNDLE}.vst/Contents/MacOS/${BUNDLE}
#   rm -f ${BUNDLE}.vst/Contents/MacOS/deleteme
#   sed -i -e "s/X-PROJECTNAME-X/${BUNDLE}/" ${BUNDLE}.vst/Contents/Info.plist
#   rm -f ${BUNDLE}.vst/Contents/Info.plist-e
# done
#
# cd ..

BUNDLE=gunshot
BIN_DIR=../../bin
DPF_DIR=../../dpf
VST_DIR=${BIN_DIR}/${BUNDLE}.vst

cp -r ${DPF_DIR}/utils/plugin.vst/ ${VST_DIR}
mv ${BIN_DIR}/${BUNDLE}-vst.dylib ${VST_DIR}/Contents/MacOS/${BUNDLE}
rm -f ${VST_DIR}/Contents/MacOS/deleteme
sed -i -e "s/X-PROJECTNAME-X/${BUNDLE}/" ${VST_DIR}/Contents/Info.plist
rm -f ${VST_DIR}/Contents/Info.plist-e
