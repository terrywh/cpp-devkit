### certificate
```
mkdir -p cert
keyfile=cert/server.key
certfile=cert/server.crt

openssl req -newkey rsa:2048 -x509 -nodes -keyout "$keyfile" -new -out "$certfile" -subj /CN=localhost
```

### library
#### dependencies
``` bash
yum install zlib-devel libev-devel libevent-devel
```

#### msquic



#### boringssl
```
git clone --depth=1 https://boringssl.googlesource.com/boringssl
cd boringssl/
mkdir stage && cd stage
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=1 -DCMAKE_INSTALL_PREFIX=/data/vendor/boringssl ../
make -j4
make install
```

#### lsquic
```
BORINGSSL=/data/vendor/boringssl
git clone --depth=1 https://github.com/litespeedtech/lsquic.git
cd lsquic
git submodule update --init --recursive --depth=1
mkdir stage && cd stage
cmake -DLSQUIC_SHARED_LIB=1 -DBORINGSSL_DIR=$BORINGSSL -DBORINGSSL_LIB=$BORINGSSL/lib64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/data/vendor/lsquic ../
make -j4
make install
```


#### nghttp3
```
mkdir stage && cd stage
../configure --prefix=/data/vendor/nghttp3 --with-pic --disable-shared
make -j4 
make install
```

#### ngtcp2
```
BORINGSSL=/data/vendor/boringssl
NGHTTP3=/data/vendor/nghttp3
LIBNGHTTP3_CFLAGS="-I$NGHTTP3/include" \
LIBNGHTTP3_LIBS="-L$NGHTTP3/lib -lnghttp3" \
BORINGSSL_LIBS="-L$BORINGSSL/lib64 -lssl -lcrypto" \
BORINGSSL_CFLAGS="-I$BORINGSSL/include" \
../configure --prefix=/data/vendor/ngtcp2 --with-pic --with-libev --with-libnghttp3 --with-boringssl 
```

####
``` bash
LD_LIBRARY_PATH=/data/vendor/boringssl/lib64:$LD_LIBRARY_PATH
./bsslserver "*" 12345 /data/htdocs/github.com/terrywh/cpp-devkit/var/server.pkey /data/htdocs/github.com/terrywh/cpp-devkit/var/server.cert
```