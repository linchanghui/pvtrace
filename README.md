# pvtrace
根据pvtrace的思想，用groovy实现

## 工具准备
* 用instrument符号编译过的informix
* userid 工具的权限为6755

## 使用流程（informix用户下执行）
* pvtrace_script目录下./exec_trace.sh
* groovy src/generate_function_table.groovy,会在目录下生成xxx_trace.svg文件，可以在浏览器下打开

## groovy脚本支持的参数
*  -c: 函数过滤，被调用最大的次数 （6）
      -h: 跟踪的线程包含的方法 （th_init_initgls）
      -d: 函数过滤，函数最小的调用跨度 （12，至少六进六出）
      -l: 函数过滤，函数最大的层级 （20）
      -e: 可执行文件路径 （/usr/local/bin/informix/bin/oninit1）
      -tl: informix运行生成的oninit.trc文件路径。（oninit.trc）
      -dot: groovy脚本生成的dot文件路径（trace.dot）
      -svg: groovy脚本生成的svg文件路径 （trace.svg）
      -s: 追踪根节点方法函数名