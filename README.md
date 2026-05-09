# lighthttp

基于 C++11 实现的轻量级 Web 服务器，采用线程池 + Epoll 实现高并发。

## 技术栈

- Linux
- C++11
- Socket / TCP
- Epoll (ET 模式)
- 线程池

## 功能特性

- ✅ 多线程 Reactor 高并发模型
- ✅ HTTP 请求解析（正则 + 有限状态机）
- ✅ 静态资源服务
- ✅ 动态缓冲区（自动增长）
- ✅ 定时器（基于小根堆，30s 清理超时连接）

## 编译

```bash
mkdir build && cd build
cmake ..
make
```

## 运行

```bash
./lighthttp [-p port] [-t thread_num]
```

默认端口 8080，线程数 4
