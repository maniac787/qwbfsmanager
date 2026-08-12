###########################################################################################
##      Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##  Author    : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##  Project   : qwbfs
##  FileName  : qwbfs.pro
##  Date      : 2010-04-04T11:04:05
##  License   : GPL2
##  Comment   : Creating using Monkey Studio RAD
##  Home Page   : https://github.com/pasnox/qwbfsmanager
##
###########################################################################################

TEMPLATE = subdirs
CONFIG *= ordered

BUILD_PATH = build

include( shared.pri )

!build_pass:message( "Building without external fresh submodule." )

SUBDIRS *= libwbfs \
    qwbfs
