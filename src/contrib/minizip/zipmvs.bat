del miniinc.zip
del minisrc.zip
del minijcl.zip
del all.zip
del alljcl.jcl
del output.txt
zip -0 -X -ll -j minisrc.zip ../../*.c *.c
zip -0 -X -ll -j miniinc.zip ../../*.h *.h
zip -0 -X -ll -j minijcl.zip *.jcl *.txt
zip -0 -X all *.zip
