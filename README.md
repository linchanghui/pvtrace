# pvtrace

## 介绍
*  首先修改了mt.c使外部的libinstrument.so可以获取到内部的tid和pid.
*  exec_trace.sh通过设置追踪开关/tmp/IFX_TRACE文件（可以参考instrument.c代码），来决定什么时候开始记录堆栈，避免记录太多
*  groovy脚本分析oninit.trc这个堆栈文件，并生成图。通过-h输入的函数名，决定跟踪哪个线程。所以这个需要使用者事先看过源码，大概知道自己要追踪的模块有什么一定会执行的方法
*  除非把那些过滤参数设置到极限，比如-l 100000 -d 1 -c 100000 才能得到不被过滤的调用图（但是这样得到的结果图太大，太乱，没法看），否则得到svg图片中很多非关键函数都被过滤。
## 工具准备
* 用instrument符号编译过的informix
* userid 工具的权限为6755
* informix 用户需要设置ids的环境，在/home/informix目录下
* 需要touch $INFORMIXDIR/etc/sysadmin/stop 为了避免db_sch_worker对追踪的影响，这个函数被调用后，没有退出。但是调用他的函数已经推出

## 使用流程（informix用户下执行）
* pvtrace_script目录下./exec_trace.sh
* groovy src/generate_function_table.groovy,会在目录下生成xxx_trace.svg文件，可以在浏览器下打开

## groovy脚本支持的参数
*  -c: 函数过滤，被调用最大的次数 （6）
*  -h: 跟踪的线程包含的方法 （th_init_initgls）
    * groovy generate_function_table.groovy -h sqmain
*  -d: 函数过滤，函数最小的调用跨度 （12，至少六进六出）
*  -l: 函数过滤，函数最大的层级 （20）
*  -e: 可执行文件路径 （/usr/local/bin/informix/bin/oninit1）
    * groovy generate_function_table.groovy -e /usr/local/bin/informix/bin/oninit
*  -tl: informix运行生成的oninit.trc文件路径。（oninit.trc）
*  -dot: groovy脚本生成的dot文件路径（trace.dot）
*  -svg: groovy脚本生成的svg文件路径 （trace.svg）
*  -s: 追踪根节点方法函数名 
    * groovy generate_function_table.groovy -s sqmain
