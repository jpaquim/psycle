TEMPLATE = subdirs

# include the base stuff shared amongst all qmake projects.
include(../build-systems/qmake/common.pri)

addSubdirs(../psycle-helpers)
addSubprojects(qmake/*.pro, ../psycle-helpers)

include($$COMMON_DIR/display-vars.pri)
