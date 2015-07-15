#!/bin/bash
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#
PROG=`basename $0`
OLDDIR=
OLDVER=
NEWDIR=
NEWVER=
TMPDIR="${TMPDIR:-/tmp}/${PROG}_`date +%Y_%m_%d`"
RUNEXTRACT=true	#undocumented feature
XSLT=xsltproc

XSLTFILE=$TMPDIR/rptdiff.xslt

usage () {
  cat > /dev/stderr <<-EOF
	$PROG -h
	$PROG [ -o oldversiondir -n newversiondir ] [ -t temporarydir ]
	$PROG [ -o oldtag -n newtag ] [ -t temporarydir ] reports-subdirs

	-h	print this usage message
	-t	directory in which to store extracted queries [$TMPDIR]

        directory-based comparison (default):
	-o	directory to search for the old versions of the reports
	-n	directory to search for the new versions of the reports

        tag-based comparison (tags should look like vX.Y.Z[optional-suffix]):
	-o	git tag for checking out the old versions of the reports
	-n	git tag for checking out the new versions of the reports
EOF
}

log () {
  echo "$*" > /dev/stderr
}

# x is an undocumented option

while getopts ho:n:t:x OPTION ; do
  case $OPTION in
    h)  usage
        exit 0
        ;;
    o)  if [ -d "$OPTARG" ] ; then
          OLDDIR="$OPTARG"
        elif [[ "$OPTARG" =~ ^v[1-9]+\.[0-9]+\.[0-9]+ ]] ; then
          OLDVER=$OPTARG
        else
          OLDVER=$OPTARG
          log "$PROG: cannot find directory $OPTARG so guessing this is a git commit-ish"
        fi
        ;;
    n)  if [ -d "$OPTARG" ] ; then
          NEWDIR="$OPTARG"
        elif [[ "$OPTARG" =~ ^v[1-9]+\.[0-9]+\.[0-9]+ ]] ; then
          NEWVER=$OPTARG
        else
          NEWVER=$OPTARG
          log "$PROG: cannot find directory $OPTARG so guessing this is a git commit-ish"
        fi
        ;;
    t)  TMPDIR=$2
        XSLTFILE=$TMPDIR/rptdiff.xslt
        shift
        ;;
    x)  RUNEXTRACT=false	#undocumented feature
        ;;
  esac
done
shift $[$OPTIND - 1]

if [ -z "$TMPDIR" ] ; then
  log "$PROG: no temporarydir given"
  usage
  exit 1
elif [ -e "$TMPDIR" -a ! -d "$TMPDIR" ] ; then
  log "$PROG: temporarydir is not a directory"
  usage
  exit 1
elif ! mkdir -p $TMPDIR/old || ! mkdir -p $TMPDIR/new ; then
  exit 2
fi

if [ -n "$OLDDIR" -a -n "$NEWVER" ] ; then
  log "$PROG: please don't mix directories and tags"
  usage
  exit 1
elif [ -n "$OLDVER" -a -n "$NEWDIR" ] ; then
  log "$PROG: please don't mix directories and tags"
  usage
  exit 1
elif [ -n "$OLDDIR" -a -z "$NEWDIR" ] ; then
  log "$PROG: please give two directories"
  usage
  exit 1
elif [ -z "$OLDDIR" -a -n "$NEWDIR" ] ; then
  log "$PROG: please give two directories"
  usage
  exit 1
elif [ -n "$OLDVER" -a -z "$NEWVER" ] ; then
  log "$PROG: please give two version tags"
  usage
  exit 1
elif [ -z "$OLDVER" -a -n "$NEWVER" ] ; then
  log "$PROG: please give two version tags"
  usage
  exit 1
elif [ -z "$OLDVER" -a -z "$NEWVER" -a -z "$OLDDIR" -a -z "$NEWDIR" ] ; then
  log "$PROG: please give two directories or two version tags"
  usage
  exit 1
fi

# convert tag-based to directory-based:
#       check out the code & copy the reports
if [ -n "$OLDVER" ] ; then
  if [ "$OLDVER" = "$NEWVER" ] ; then
    log "$PROG: old version equals new version so doing nothing"
    exit 1
  fi
  if [ $# -lt 1 ] ; then
    log "$PROG: tag-based comparison needs at least one report subdirectory"
    exit 1
  fi
  for VER in $OLDVER $NEWVER ; do
    git checkout $VER                                  || exit 1
    rm -rf $TMPDIR/$VER
    mkdir $TMPDIR/$VER
    for DIR in $* ; do
      if [ `ls $DIR/*.xml | wc -l` -le 0 ] ; then
        log "$PROG: $DIR doesn't seem to have any xml files in it"
      else
        cp $DIR/*.xml $TMPDIR/$VER
      fi
    done
  done

  OLDDIR=$TMPDIR/$OLDVER
  NEWDIR=$TMPDIR/$NEWVER
fi



if [ `ls $OLDDIR/*.xml | wc -l` -le 0 ] ; then
  log "$PROG: $OLDDIR doesn't seem to have any xml files in it"
  exit 1
fi
if [ `ls $NEWDIR/*.xml | wc -l` -le 0 ] ; then
  log "$PROG: $NEWDIR doesn't seem to have any xml files in it"
  exit 1
fi

cat > $XSLTFILE <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >
  <xsl:output indent="yes" method="text" />

  <xsl:template match="text()"/>

  <xsl:template match="report/name">
====================================================================
REPORT: <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="report/querysource">
QUERY: <xsl:value-of select="name"/><xsl:text>
</xsl:text>
    <xsl:value-of select="sql"     />
    <xsl:if test="mqlgroup">== MetaSQL statement </xsl:if>
    <xsl:value-of select="mqlgroup"/>
    <xsl:if test="mqlname">-</xsl:if>
    <xsl:value-of select="mqlname" />
--------------------------------------------------------------------
  </xsl:template>

</xsl:stylesheet>
EOF

if $RUNEXTRACT ; then
for DIR in $OLDDIR $NEWDIR ; do
  if [ "$DIR" = "$OLDDIR" ] ; then
    DESTDIR="$TMPDIR/old"
  elif [ "$DIR" = "$NEWDIR" ] ; then
    DESTDIR="$TMPDIR/new"
  else
    log "$PROG: I'm confused and don't know where to write my output"
    exit 3
  fi
  if ! rm -f "$DESTDIR/*" ; then
    exit 4
  fi

  FILELIST=`ls $DIR/*.xml`
  if [ -z "$FILELIST" ] ; then
    log no .xml files in $DIR
    exit 2
  fi

  for FILE in $FILELIST ; do
    INPUTFILE=$FILE
    OUTPUTFILE=$DESTDIR/`basename $INPUTFILE`
    log processing $INPUTFILE > /dev/stderr

    if [ "$XSLT" = Xalan ] ; then
      "$XSLT" "$INPUTFILE" "$XSLTFILE" > "$OUTPUTFILE"
    elif [ "$XSLT" = xsltproc ] ; then
      "$XSLT" "$XSLTFILE" "$INPUTFILE" > "$OUTPUTFILE"
    else
      Cannot find XSLT Processor "$XSLT"
      exit 2
    fi

  done
done
fi	# if $RUNEXTRACT

RMLIST=
NEWLIST=
CHANGEDLIST=

if [ -s $TMPDIR/diffs ] && ! rm $TMPDIR/diffs ; then
  exit 3
fi

for FILE in `ls $TMPDIR/old/* $TMPDIR/new/* | xargs -J X -n 1 basename X | sort -u` ; do
  if  [ -f $TMPDIR/old/$FILE -a ! -f $TMPDIR/new/$FILE ] ; then
    REPORTNAME="`head -3 $TMPDIR/old/$FILE | tail -1 | cut -f2 -d:`"
    RMLIST="$RMLIST $REPORTNAME"
  elif [ ! -f $TMPDIR/old/$FILE -a -f $TMPDIR/new/$FILE ] ; then
    REPORTNAME="`head -3 $TMPDIR/new/$FILE | tail -1 | cut -f2 -d:`"
    NEWLIST="$NEWLIST $REPORTNAME"
  elif ! cmp -s $TMPDIR/old/$FILE $TMPDIR/new/$FILE ; then
    REPORTNAME="`head -3 $TMPDIR/new/$FILE | tail -1 | cut -f2 -d:`"
    CHANGEDLIST="$CHANGEDLIST $REPORTNAME"
    diff -w --unchanged-line-format='   %L' \
                  --old-line-format='-  %L' \
                  --new-line-format='+  %L' $TMPDIR/{old,new}/$FILE >> $TMPDIR/diffs
  fi
done

if [ -n "$RMLIST" ] ; then
  echo ====================================================================
  echo REMOVED REPORTS:
  echo $RMLIST | tr -s [:blank:] "\n"
fi

if [ -n "$NEWLIST" ] ; then
  echo ====================================================================
  echo NEW REPORTS:
  echo $NEWLIST | tr -s [:blank:] "\n"
fi

if [ -n "$CHANGEDLIST" ] ; then
  echo ====================================================================
  echo CHANGED REPORTS:
  echo $CHANGEDLIST | tr -s [:blank:] "\n"
  echo
  cat $TMPDIR/diffs
fi
