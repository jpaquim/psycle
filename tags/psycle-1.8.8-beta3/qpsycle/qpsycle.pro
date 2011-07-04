TEMPLATE = subdirs

# include the base stuff shared amongst all qmake projects.
include(../build-systems/qmake/common.pri)

addSubdirs(../psycle-core)
addSubdirs(qmake, ../psycle-core)

