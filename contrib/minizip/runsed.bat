sed 's/REGION=CLASS/CLASS/g' <%1 >sed.tmp
del %1
ren sed.tmp %1
