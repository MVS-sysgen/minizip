del miniunz.exe
del *.o

call stdcomp miniunz.c
call stdcomp unzip.c
call stdcomp extra.c
call stdcomp unshrink.c

call stdcomp ../../crc32.c
call stdcomp ../../inflate.c
call stdcomp ../../infblock.c
call stdcomp ../../infcodes.c
call stdcomp ../../inftrees.c
call stdcomp ../../infutil.c
call stdcomp ../../inffast.c
call stdcomp ../../zutil.c
call stdcomp ../../adler32.c

rem gcc -s -nostdlib -o miniunz.exe *.o ../../../pdos/pdpclib/pdpwin32.a -lkernel32 -lgcc


rem del minizip.exe
rem del *.o

call stdcomp minizip.c
call stdcomp zip.c
call stdcomp ../../deflate.c
call stdcomp ../../crc32.c
call stdcomp ../../adler32.c
call stdcomp ../../trees.c
call stdcomp ../../zutil.c

rem gcc -s -nostdlib -o minizip.exe *.o ../../../pdos/pdpclib/pdpwin32.a -lkernel32 -lgcc
