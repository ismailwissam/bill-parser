#!/bin/sh

cp ../config.h ./

../../asn1c/asn1c/asn1c ../siemens.asn1 || exit $?

if [ ! -f Makefile.am.sample ]; then
	echo "Makefile.am.sample is missing"
	exit 1
fi

set -x
cat Makefile.am.sample						\
	| sed -e 's/^CFLAGS.*/CFLAGS += -I. -I..\/ -I..\/..\/asn1c\/skeletons -I..\/..\/asn1c\/libasn1compiler -I..\/..\/asn1c\/libasn1fix -I..\/..\/asn1c\/libasn1parser -I..\/..\/asn1c\/libasn1print -DHAVE_CONFIG_H -DPDU=Sr9Siemens/'	\
	| sed -e 's/converter-sample/..\/siemens/'				\
	| sed -e 's/progname/..\/siemens/'				\
	> Makefile.$$

(	echo
	echo "../siemens.asn1:"
	echo "	@echo The ../siemens.asn1 file is not yet present."
	echo "	@echo Please read the README file on how to obtain this file."
	echo "	@exit 42"
	echo
	echo "DataInterChange.c: ../siemens.asn1 regenerate.Makefile"
	echo "	./regenerate.Makefile"
	echo "	@touch DataInterChange.c"
	echo "	make"
	echo
	echo '$(TARGET).o: ../siemens.c'
	echo '	$(CC) $(CFLAGS) -o $(TARGET).o -c $<'
	echo
	echo "distclean: clean"
	echo '	rm -f $(ASN_MODULE_SOURCES) $(ASN_MODULE_HEADERS)'
	echo "	rm -f Makefile.am.sample"
) >> Makefile.$$

rm Makefile.am.sample || exit $?

mv Makefile.$$ Makefile

set +x
echo
echo "Makefile generation finished"