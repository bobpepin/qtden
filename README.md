qtden
=====

GUI for Clustering, Dendrogram and Heatmap using Self-Organizing Maps.


Compilation
===========

The application uses the Qt toolkit and compilation requires the qmake tool.

Compilation requires the cprob module from the Cephes Math Library from http://www.netlib.org/cephes/cprob.tgz as well as the libesom library from https://github.com/bobpepin/libesom.

After unpacking, apply the patch in cprob.diff to cprob and compile cprob using qmake.
Then, compile libesom using qmake as well.

Finally, adjust the paths to libesom and cprob in qtden.pro and compile QtDen using qmake.
