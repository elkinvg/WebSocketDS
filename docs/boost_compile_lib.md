boost_compile
=============

```
./b2.exe toolset=msvc-12.0 --with-thread --with-chrono --with-date_time --with-regex link=static  address-model=64 --layout=tagged threading=multi runtime-link=static,shared variant=debug,release stage
```

and

```
./b2.exe toolset=msvc-12.0 --with-thread --with-chrono --with-date_time --with-regex link=static  address-model=64 --layout=versioned threading=multi runtime-link=static,shared variant=debug,release stage
```