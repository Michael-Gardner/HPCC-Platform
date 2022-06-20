#!/bin/bash

#BUILD_TAG=$(git describe --exact-match --tags || true)

BUILD_TYPE=Debug
HEAD=$(git rev-parse --short HEAD)
BUILD_LABEL="${HEAD}-${BUILD_TYPE}"

SIGN_MODULES_KEYID="D64CF421C2974E999FA6962A323210012E1BE3B3"

# generate pub.key for authentication of signed libraries
gpg --output=pub.key --batch --no-tty --export ${SIGN_MODULES_KEYID}

# list of ecl stdlibs to be signed
modules=(Audit.ecl BLAS.ecl BundleBase.ecl Crypto.ecl Date.ecl File.ecl Math.ecl Metaphone3.ecl Metaphone.ecl Str.ecl Uni.ecl)


echo "Signing ECL for ${BUILD_LABEL}"

docker run -it platform-core:${BUILD_LABEL}
for m in ${modules[@]}; do
  docker cp platform-core:${BUILD_LABEL}/
done
