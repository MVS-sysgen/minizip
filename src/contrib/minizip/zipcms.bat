del all.zip
del alljcl.jcl
del output.txt

zip -0 -X -ll -j all *.c *.h *.exec *.parm *.txt ../../*.c ../../*.h
