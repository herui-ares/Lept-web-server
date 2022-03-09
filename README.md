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

- Open the browser, input the the resources.

```
http:// 120.25.155.134:10000 / index.html
```

![result](https://github.com/herui-ares/Lept-web-server/blob/main/result.png)

**Highlights：**

- Using Thread Pool, Non-blocking Socket, Epoll and Proactor in this work.
- Using State Machine to resolve the HTTP request.
- Using Webbench to test presure.
