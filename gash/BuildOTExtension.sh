#!/bin/sh

#  build_miracl.sh
#  gash
#
#  Created by Xiaoting Tang on 6/18/18.
#  Copyright © 2018 Xiaoting Tang. All rights reserved.

echo "Starting building libmiracl.a..."
DIR=$(echo $PWD)
if [ ! -f $DIR/miracl.a ]; then
    echo "miracl.a not found, building it..."
    OTExtEncryptoPath="${DIR}/OTExtension/ENCRYPTO_utils"
    MIRACL_MAKE="linux64"
    CFLAG="-fPIC -c -m64 -O2 -mdynamic-no-pic"

    find ${OTExtEncryptoPath}/Miracl/ -type f -exec cp '{}' ${OTExtEncryptoPath}/miracl_lib \;

    cd ${OTExtEncryptoPath}/miracl_lib ;

    # bash ${OTExtEncryptoPath}/miracl_lib/${MIRACL_MAKE} ;
    rm -rf *.exe
    rm -rf miracl.a
    cp mirdef.h64 mirdef.h
    gcc -fPIC -c -m64 -O2 mrcore.c
    gcc -fPIC -c -m64 -O2 mrarth0.c
    gcc -fPIC -c -m64 -O2 mrarth1.c
    gcc -fPIC -c -m64 -O2 mrarth2.c
    gcc -fPIC -c -m64 -O2 mralloc.c
    gcc -fPIC -c -m64 -O2 mrsmall.c
    gcc -fPIC -c -m64 -O2 mrio1.c
    gcc -fPIC -c -m64 -O2 mrio2.c
    gcc -fPIC -c -m64 -O2 mrgcd.c
    gcc -fPIC -c -m64 -O2 mrjack.c
    gcc -fPIC -c -m64 -O2 mrxgcd.c
    gcc -fPIC -c -m64 -O2 mrarth3.c
    gcc -fPIC -c -m64 -O2 mrbits.c
    gcc -fPIC -c -m64 -O2 mrrand.c
    gcc -fPIC -c -m64 -O2 mrprime.c
    gcc -fPIC -c -m64 -O2 mrcrt.c
    gcc -fPIC -c -m64 -O2 mrscrt.c
    gcc -fPIC -c -m64 -O2 mrmonty.c
    gcc -fPIC -c -m64 -O2 mrpower.c
    gcc -fPIC -c -m64 -O2 mrsroot.c
    gcc -fPIC -c -m64 -O2 mrcurve.c
    gcc -fPIC -c -m64 -O2 mrfast.c
    gcc -fPIC -c -m64 -O2 mrshs.c
    gcc -fPIC -c -m64 -O2 mrshs256.c
    gcc -fPIC -c -m64 -O2 mrshs512.c
    gcc -fPIC -c -m64 -O2 mrsha3.c
    gcc -fPIC -c -m64 -O2 mrfpe.c
    gcc -fPIC -c -m64 -O2 mraes.c
    gcc -fPIC -c -m64 -O2 mrgcm.c
    gcc -fPIC -c -m64 -O2 mrlucas.c
    gcc -fPIC -c -m64 -O2 mrzzn2.c
    gcc -fPIC -c -m64 -O2 mrzzn2b.c
    gcc -fPIC -c -m64 -O2 mrzzn3.c
    gcc -fPIC -c -m64 -O2 mrzzn4.c
    gcc -fPIC -c -m64 -O2 mrecn2.c
    gcc -fPIC -c -m64 -O2 mrstrong.c
    gcc -fPIC -c -m64 -O2 mrbrick.c
    gcc -fPIC -c -m64 -O2 mrebrick.c
    gcc -fPIC -c -m64 -O2 mrec2m.c
    gcc -fPIC -c -m64 -O2 mrgf2m.c
    gcc -fPIC -c -m64 -O2 mrflash.c
    gcc -fPIC -c -m64 -O2 mrfrnd.c
    gcc -fPIC -c -m64 -O2 mrdouble.c
    gcc -fPIC -c -m64 -O2 mrround.c
    gcc -fPIC -c -m64 -O2 mrbuild.c
    gcc -fPIC -c -m64 -O2 mrflsh1.c
    gcc -fPIC -c -m64 -O2 mrpi.c
    gcc -fPIC -c -m64 -O2 mrflsh2.c
    gcc -fPIC -c -m64 -O2 mrflsh3.c
    gcc -fPIC -c -m64 -O2 mrflsh4.c
    cp mrmuldv.g64 mrmuldv.c
    gcc -fPIC -c -m64 -O2 mrmuldv.c
    ####
    g++ -fPIC -c -m64 -O2 big.cpp
    g++ -fPIC -c -m64 -O2 zzn.cpp
    g++ -fPIC -c -m64 -O2 ecn.cpp
    g++ -fPIC -c -m64 -O2 ec2.cpp
    g++ -fPIC -c -m64 -O2 crt.cpp
    ####
    ar rc miracl.a mrcore.o mrarth0.o mrarth1.o mrarth2.o mralloc.o mrsmall.o mrzzn2.o mrzzn3.o
    ar r miracl.a mrio1.o mrio2.o mrjack.o mrgcd.o mrxgcd.o mrarth3.o mrbits.o mrecn2.o mrzzn4.o
    ar r miracl.a mrrand.o mrprime.o mrcrt.o mrscrt.o mrmonty.o mrcurve.o mrsroot.o mrzzn2b.o
    ar r miracl.a mrpower.o mrfast.o mrshs.o mrshs256.o mraes.o mrlucas.o mrstrong.o mrgcm.o
    ar r miracl.a mrflash.o mrfrnd.o mrdouble.o mrround.o mrbuild.o
    ar r miracl.a mrflsh1.o mrpi.o mrflsh2.o mrflsh3.o mrflsh4.o
    ar r miracl.a mrbrick.o mrebrick.o mrec2m.o mrgf2m.o mrmuldv.o mrshs512.o mrsha3.o mrfpe.o
    ####
    ar r miracl.a big.o zzn.o ecn.o ec2.o crt.o
    ####
    rm mr*.o
    gcc -fPIC -m64 -O2 bmark.c miracl.a -o bmark
    gcc -fPIC -m64 -O2 fact.c miracl.a -o fact
    # g++ -c -m64 -O2 big.cpp
    # g++ -c -m64 -O2 zzn.cpp
    # g++ -c -m64 -O2 ecn.cpp
    # g++ -c -m64 -O2 ec2.cpp
    # g++ -c -m64 -O2 crt.cpp
    g++ -fPIC -m64 -O2 mersenne.cpp big.o miracl.a -o mersenne
    g++ -fPIC -m64 -O2 brent.cpp big.o zzn.o miracl.a -o brent
    g++ -fPIC -c -m64 -O2 flash.cpp
    g++ -fPIC -m64 -O2 sample.cpp flash.o miracl.a -o sample
    g++ -fPIC -m64 -O2 ecsgen.cpp ecn.o big.o miracl.a -o ecsgen
    g++ -fPIC -m64 -O2 ecsign.cpp ecn.o big.o miracl.a -o ecsign
    g++ -fPIC -m64 -O2 ecsver.cpp ecn.o big.o miracl.a -o ecsver
    g++ -fPIC -m64 -O2 pk-demo.cpp ecn.o big.o miracl.a -o pk-demo
    g++ -fPIC -c -m64 -O2 polymod.cpp
    g++ -fPIC -c -m64 -O2 poly.cpp
    g++ -fPIC -m64 -O2 schoof.cpp polymod.o poly.o ecn.o crt.o zzn.o big.o miracl.a -o schoof

    # find . -type f -not -name '*.a' -not -name '*.h' -not -name '*.o' -not -name '.git*' | xargs rm

    ranlib miracl.a
    cp miracl.a $DIR/miracl.a

fi

echo "Starting to build OTExtension"
cd $DIR/OTExtension && make && ranlib libotext.a && cp libotext.a $DIR/libotext.a
