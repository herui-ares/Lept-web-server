# Lept-web-server
A lept web server project coding by C++ in Linux system.

**Usage：**

- git clone the code.

```
git clone git@github.com:herui-ares/Lept-web-server.git
```

- cd to the dir.

```
cd ~./Lept-web-server
```

- make the project.

```
make
```

- run target with port name.

```
./webserver 10000
```

[![usage_sample](https://github.com/M3stark/LeptHttp/raw/main/resources/screenshoot/usage_sample.png)](https://github.com/M3stark/LeptHttp/blob/main/resources/screenshoot/usage_sample.png)

- Open the browser, input the the resources.

```
http:// your link : port name / index.html
```



**Highlights：**

- Using Thread Pool, Non-blocking Socket, Epoll and Proactor in this work.
- Using State Machine to resolve the HTTP request.
- Using Webbench to test presure.
