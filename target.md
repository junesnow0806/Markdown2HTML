# Memo of OSH

spdk的示例程序可执行程序在/spdk/build/examples/目录下。

写好需要spdk的源代码后，放到spdk文件夹下用文档所给命令编译。

需打开虚拟机工作，打开vagrant步骤：

> 进入目录spdk/scripts/vagrant/ubuntu2004-virtualbox
>
> 用root权限依次执行 vagrant up 和 vagrant ssh

## 可能用到的背景

​		基于NVMe设备，一种有效改善吞吐能力的办法是借助内核旁路机制，通过用户态的驱动程序来与底层的NVMe设备进行直接的交互，从而避免系统调用所带来的overhead开销。为此SPDK应运而生，其不但提供了一个用户态的文件系统(blobfs)，还将磁盘驱动程序放到了用户态空间，使得数据的访问可以在用户态直接进行，这样便有效避免了操作系统的上下文切换以及数据在用户态和内核态之间的拷贝传递。

## SPDK、BlobFS和BlobStore笔记

​		SPDK在存储服务层提供了BlobStore和BlobFS，其中BlobStore是一种持久的电源故障安全快分配器，用于与用户态文件系统BlobFS集成，旨在通过代替传统的文件系统，支持更高级别的存储服务，如MyAOL、RocksDB等。

​		SPDK官方提供的BlobFS的应用场景是为Rocksdb提供存储后端，Rocksdb的I/O经由BlobFS与BlobStore下发到bdev，随后由SPDK提供的用户态驱动写入NVMe盘。整个I/O流程从发起到落盘均在用户态操作，且充分利用了SPDK所提供的异步、无锁化、Zero Copy、轮询等机制，大幅度减少额外的系统开销。

​		I/O channel：在thread.c中实现

​		SPDK Bdev设计主要考虑以下几个维度：一是，抽象出来的通用设备需要数据结构来维护；二是，操作通用设备的函数指针表，提供给后端具体硬件来实现；三是，I/O数据结构用来打通上层协议模块和下层驱动模块。下面我们具体来看一下这些核心的数据结构。

* 通用块设备的数据结构：struct spdk_bdev
* 操作通用设备的函数指针表：struct spdk_bdev_fn_table
* 块设备I/O数据结构：struct spdk_bdev_io

### BlobStore介绍

https://www.sdnlab.com/22880.html

### BlobStore运作理论

spdk官方文档

***抽象：***

BlobStore定义了存储抽象的层次结构

* 逻辑块(Logic Block)
* 页面(Page)
* Cluster
* Blob
* BlobStore

### RocksDB如何通过SPDK写SSD

​		RocksDB在SPDK中已经实现有连接的接口(spdk/rocksdb/env-spdk.c)中通过`append`函数调用BlobFS中的`spdk_file_write`实现对SSD的写入。

https://www.liangzl.com/get-article-detail-222251.html

### 接口要怎么用？

"用户在使用spdk编程的过程中,通过以上接口,就可以简单地操作一个块设备"

`spdk bdev layer`：直接操作块设备的接口

https://mp.weixin.qq.com/s?__biz=MzI3NDA4ODY4MA==&mid=2653336658&idx=1&sn=6c81de4b2897fd78bfdabab9c1c316dd&chksm=f0cb43d5c7bccac3a531867010e43e32abe13c5f8607e61e458f19594402ae5974fd326f7960&scene=21#wechat_redirect

* 如何启动spdk_bdev？

## 具体能做些什么？

1. 创建空的blobfs进行测试：官方文档

   ```
   cd spdk
   HUGEMEM=3048 scripts/setup.sh
   scripts/gen_nvme.sh --json-with-subsystems > /home/vagrant/spdk/rocksdb.json
   test/blobfs/mkfs/mkfs /home/vagrant/spdk/rocksdb.json Nvme0n1
   ```

   结果：

   ```
   Initializing filesystem on bdev Nvme0n1...done
   ```

2. blobfs改造：https://blog.csdn.net/weixin_37097605/article/details/114052086

3. 添加支持blobfs的目录：现有blobfs是根据文件名逐个比对(队列)来查找文件的，能否实现目录查询方式？支持目录查询但保留原有的队列数据结构

   * 目录用什么数据结构？要怎么显示当前目录？
   * 新建一个文件和删除一个文件要怎么修改目录？

4. 关于测试：

   * SPDK有一组脚本，这些脚本将针对各种工作负载运行db_bench，并捕获性能和分析数据，主要脚本是test/blobfs/rocksdb/rocksdc.sh(官方文档)。
   * 编写一个应用程序

### 完成的工作



## 报错及其可能的原因

1. 在vagrant中执行`HUGEMEM=5120 scripts/gen_nvme.sh`时不够分配，执行`mkfs`时会Killed，查看killed信息是"Out of memory"。

## 问题

1. 需要用到哪些文件？需要修改哪些代码？

   thread.c

~~**内加粗外删除**~~

**~~内删除外加粗~~**

_**内加粗外斜体**_

__*内斜体外加粗*__

