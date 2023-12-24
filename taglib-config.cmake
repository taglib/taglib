#!/bin/sh

usage()
{
  echo "usage: $0 [OPTIONS]"
cat << EOH

options:
  [--libs]
  [--cflags]
  [--version]
  [--prefix]
EOH
  exit 1
}

# Looks useless as it is, but could be replaced with a "pcfiledir" by Buildroot.
prefix=
exec_prefix=

if test -z "$prefix"; then
  includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@
else
  includedir=${prefix}/@CMAKE_INSTALL_INCLUDEDIR@
fi
if test -z "$exec_prefix"; then
  libdir=@CMAKE_INSTALL_FULL_LIBDIR@
else
  libdir=${exec_prefix}/@CMAKE_INSTALL_LIBDIR@
fi

flags=""

if test $# -eq 0 ; then
  usage
fi

while test $# -gt 0
do
  case $1 in
    --libs)
      flags="$flags -L$libdir -ltag@TAGLIB_INSTALL_SUFFIX@ @ZLIB_LIBRARIES_FLAGS@"
      ;;
    --cflags)
      flags="$flags -I$includedir -I$includedir/taglib@TAGLIB_INSTALL_SUFFIX@"
      ;;
    --version)
      echo @TAGLIB_LIB_VERSION_STRING@
      ;;
    --prefix)
      echo ${prefix:-@CMAKE_INSTALL_PREFIX@}
      ;;
    *)
      echo "$0: unknown option $1"
      echo
      usage
      ;;
  esac
  shift
done

if test -n "$flags"
then
  echo $flags
fi
