# coding: utf-8

import os
import sys

extensions = [".h", ".cpp", ".c"]

files = []
for parentDir, _, fileNames in os.walk(os.getcwd()):
    for fileName in fileNames:
        filepath = os.path.join(parentDir, fileName)
        filepath = filepath.replace("\\", "/")
        files.append(filepath)
pass

cppfiles = []
for f in files:
    ext = os.path.splitext(f)[1]
    ext = str(ext).lower()
    if ext in extensions:
        cppfiles.append(f)
        pass
    pass

srcfiles = []
for f in cppfiles:
    index = f.find("Runtime/")
    srcfiles.append(f[index:])
    pass

files = {}
tlist = set()
for f in srcfiles:
    dirname = os.path.dirname(f)
    srctag  = dirname.replace("/", "_") + "_SRCS"
    inctag  = dirname.replace("/", "_") + "_HDRS"

    if not files.get(srctag):
        files[srctag] = []
        pass

    if not files.get(inctag):
        files[inctag] = []
        pass
    
    ext = str(os.path.splitext(f)[1]).lower()
    if ext == ".h":
        files[inctag].append(f)
        pass
    else:
        files[srctag].append(f)
        pass
    
    tlist.add(dirname)
    pass

tagtext = ""

for tag in tlist:
    srctag = tag.replace("/", "_") + "_SRCS"
    inctag = tag.replace("/", "_") + "_HDRS"

    tagtext += "set(" + inctag + "\n"
    filelist = files[inctag]
    for f in filelist:
        tagtext += "    " + f + "\n"
        pass 
    tagtext += ")\n"

    tagtext += "set(" + srctag + "\n"
    filelist = files[srctag]
    for f in filelist:
        tagtext += "    " + f + "\n"
        pass 
    tagtext += ")\n\n"

    pass

libtext = "add_library(FlyCore STATIC\n"
for tag in tlist:
    srctag = tag.replace("/", "_") + "_SRCS"
    inctag = tag.replace("/", "_") + "_HDRS"
    libtext += "    ${" + inctag + "}\n"
    libtext += "    ${" + srctag + "}\n"
    libtext += "\n"
libtext += ")\n\n"

grouptext = ""
for tag in tlist:
    srctag = tag.replace("/", "_") + "_SRCS"
    inctag = tag.replace("/", "_") + "_HDRS"
    grouptext += "source_group(" + tag.replace("/", "\\\\") + " FILES " + "${" + inctag + "} " + "${" + srctag + "})\n"
    pass

open("../CMakeLists.txt", "w").write(tagtext + libtext + grouptext)