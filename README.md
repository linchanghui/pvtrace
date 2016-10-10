# pvtrace
pvtrace对informix的堆栈路径追踪的定制版
1.外部依赖redis和zlog，informix源码也需要修改，保证能把thread id传出。这部分实现是机密。
1.c部分是修改pvtrace的源码，和instrument.c的源码，对输出的堆栈记录根据线程id进行筛选，这部分是靠groovy脚本实现
