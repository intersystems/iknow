FROM gcc:8

ENV ICU_VERSION 65.1

ENV ICUDIR /usr/src/iknow/thirdparty/icu/
ENV IKNOWPLAT lnxubuntux64

COPY . /usr/src/iknow/

RUN echo "http://github.com/unicode-org/icu/releases/download/release-${ICU_VERSION}/icu4c-${ICU_VERSION}-Ubuntu18.04-x64.tgz" | sed -E "s,release-([0-9]+)\.([0-9]+)/icu4c-([0-9]+)\.([0-9]+),release-\1-\2/icu4c-\3_\4," | xargs curl -o /tmp/icu.tgz -LJO \
    && mkdir /usr/src/iknow/thirdparty \
    && mkdir /usr/src/iknow/thirdparty/icu \
    && tar -xzf /tmp/icu.tgz -C /usr/src/iknow/thirdparty/icu --strip-components=4 ./icu/usr/local/ \
    && rm /tmp/icu.tgz