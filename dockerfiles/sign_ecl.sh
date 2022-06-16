#!/bin/bash

#BUILD_TAG=$(git describe --exact-match --tags || true)

BUILD_TYPE=Debug
HEAD=$(git rev-parse --short HEAD)
BUILD_LABEL="${HEAD}-${BUILD_TYPE}"

SIGN_MODULES_KEYID="D64CF421C2974E999FA6962A323210012E1BE3B3"

gpg --output=pub.key --batch --no-tty --export ${SIGN_MODULES_KEYID}



echo "Signing ECL for ${BUILD_LABEL}"
