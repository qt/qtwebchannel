%modules = ( # path to module name map
    "QtWebChannel" => "$basedir/src/webchannel",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#   - an empty string to use the same branch under test (dependencies will become "refs/heads/master" if we are in the master branch)
#
%dependencies = (
    "qtbase" => "",
    # optional dependencies:
    "qtdeclarative" => "",
    # TODO: disabled for now as it breaks CI builds on OSX
    #       requires changes to qtqa scripts as discussed with sifalt, sahumada, tronical
    # "qtwebkit" => "",
);
