import groovy.util.BuilderSupport
import javax.imageio.ImageIO
import groovy.swing.SwingBuilder
import java.awt.BorderLayout
/**
 * 分析TRC文件
 *
 * @author bruce
 */
//-----------------------------------------------------------------------------
//全局配置定义
//-----------------------------------------------------------------------------
String          FUNC_HINT="th_init_initgls" //目标跟踪函数，也就是：跟踪目标线程中一定出现的函数
String          START_FUNC=null //不追踪整个堆栈，而是从这个方法开始追踪
int             FUNC_MAX_CALL_TIMES=6
int             TRACE_MIN_DISTANCE=12
int             TRACE_MAX_LEVEL=20
int             WEIGHT_LEVEL_1=6
int             WEIGHT_LEVEL_2=13

String          EXEC_PATH = "/usr/local/bin/informix/bin/oninit1"
//String          EXEC_PATH = "/home/linchanghui/Documents/oninit"

String          NM_COMMAND="nm -e ${EXEC_PATH} -l -C"

String          PATH_TRACE_LOG="oninit.trc"
String          PATH_OF_DOT="trace.dot"
String          PATH_OF_SVG="trace.svg"

//-----------------------------------------------------------------------------
//全局变量定义
//-----------------------------------------------------------------------------
Map<String, Map<String, String>> functions=[:]
List<String> raw_lines=[]
List<Map<String,?>> log_lines=[]
List<Map<String,?>> log_filtered=[]

//-----------------------------------------------------------------------------
//输入参数处理部分
//-----------------------------------------------------------------------------
def cli
cli = new CliBuilder().with {
    usage = 'program [options] <arguments>'
    header = 'Options:'
    footer = '-' * width
    c(args: 1, argName: 'FUNC_MAX_CALL_TIMES', "FUNC_MAX_CALL_TIMES (XXXXXXXXX)")
    h(args: 1, argName: 'FUNC_HINT', "FUNC_HINT (XXXXXXXXX)")
    d(args: 1, argName: 'TRACE_MIN_DISTANCE', "TRACE_MIN_DISTANCE (XXXXXXXXX)")
    l(args: 1, argName: 'TRACE_MAX_LEVEL', "TRACE_MAX_LEVEL (XXXXXXXXX)")
    e(args: 1, argName: 'EXEC_PATH', "EXEC_PATH (XXXXXXXXX)")
    tl(args: 1, argName: 'PATH_TRACE_LOG', "PATH_TRACE_LOG (XXXXXXXXX)")
    dot(args: 1, argName: 'PATH_OF_DOT', "PATH_OF_DOT (XXXXXXXXX)")
    svg(args: 1, argName: 'PATH_OF_SVG', "PATH_OF_SVG (XXXXXXXXX)")
    s(args: 1, argName: 'START_FUNC', "START_FUNC (XXXXXXXXX)")

    def options = parse(args)
    options || System.exit(1)

    parse(args) ?: System.exit(1)
}
try{
    if(cli.c) FUNC_MAX_CALL_TIMES = cli.c.toInteger()
    if(cli.h) FUNC_HINT = cli.h
    if(cli.d) TRACE_MIN_DISTANCE = cli.d.toInteger()
    if(cli.l) TRACE_MAX_LEVEL = cli.l.toInteger()
    if(cli.e) {
        EXEC_PATH = cli.e
        NM_COMMAND="nm -e ${EXEC_PATH} -l -C"
    }
    if(cli.tl) PATH_TRACE_LOG = cli.tl
    if(cli.dot) PATH_OF_DOT = cli.dot
    if(cli.svg) FUNC_MAX_CALL_TIMES = cli.svg
    if(cli.s) START_FUNC = cli.s

}catch (Exception e) {
    exitWithMessage(e.printStackTrace())
}



//-----------------------------------------------------------------------------
//执行shell命令所需的参数
//-----------------------------------------------------------------------------
def proc
def sout = new StringBuilder(), serr = new StringBuilder()
void exitWithMessage(String message) {
    System.err.println(message)
    System.exit(1)
}

proc = "userid root chown informix:informix ${PATH_TRACE_LOG}".execute()
proc.waitForProcessOutput(sout, serr)
if("$sout".length() != 0 || "${serr}".length() != 0) {
    exitWithMessage("out> $sout err> $serr")
}

//-----------------------------------------------------------------------------
//获取全局函数地址、名称、文件映射表
//-----------------------------------------------------------------------------
String command = NM_COMMAND

String[] lines = command.execute().text.split("\n")

for(int i=0; i<lines.size();i++)
{
    String previous = ""
    //通过正则将参数列表删除，否则可能出现参数位置不对
    //    List<String> inputs = [
    //            //[INPUT]                                     [edgeOutput]
    //            "csmQueryBlockInit(db2UCinterface*)",         //csmQueryBlockInit
    //            "closeHandle(void**)",                        //closeHandle
    //            "CUdbgVar::GetReportInfo(int*, int*)",        //CUdbgVar::GetReportInfo
    //            "getIntProperty(int)",                        //getIntProperty
    //            "getIntProperty()",                           //getIntProperty
    //            "CUXParser::BuildSubTree(CUXNode*)",          //CUXParser::BuildSubTree
    //    ]
    String tmp=lines[i]
    //通过迭代完成全部替换
    while(tmp != previous)
    {
        previous=tmp
        tmp=tmp.replaceAll(/\([\W\w.]*\)/, "")
    }

    String[] fields=tmp.split(" |\t")

    //00000000019fee9c T _ZN8CUdbgExpD1Ev	/vobs/tristarm/spldebug/udbgexcept.C:79
    //4列: 地址 类型 名称 归属

    if(fields.size() == 4 )
    {
        String addr=fields[0].replaceFirst(/^0+/,"")
        String name=fields[2]

        //把 spldebug/udbgexcept.C:79 进行切分
        String[] temps=(fields[3] - "/vobs/tristarm/").split("/")
        String module= temps[0]
        String dir= temps[0..-2].join("/").replaceAll("/./", "/")
        String file=temps[-1].split(":")[0]
        String line=temps[-1].split(":")[1]

        functions[addr]=[name: name, module: module, dir:dir, file: file, line: line]
    }
}

//-----------------------------------------------------------------------------
//抽取目标函数线程日志
//-----------------------------------------------------------------------------
//找到FUNC_HINT的地址
String func_hint_addr=""
functions.each{String addr, Map<String, String> func->
    if(func["name"]==FUNC_HINT)
    {
        func_hint_addr=addr
    }
}

//println func_hint_addr

//找到包含FUNC_HINT调用的线程
String func_hint_thread=""
new File(PATH_TRACE_LOG).eachLine {String line->
    if(line?.contains(func_hint_addr))
    {
        func_hint_thread=line.split(",")[2]
    }
}

//println func_hint_thread

new File(PATH_TRACE_LOG).eachLine {String line->
    if(line?.endsWith(func_hint_thread))
    {
        raw_lines.add(line)
    }
}
//转换成MAP
raw_lines.each{String line->
    //<,61a75a,1000050
    String[] fields=line.split(",")

    Map<String, String> log=[
            //time: fields[0],
            //pid: fields[1],
            direct: fields[0],
            weight: 0,
            accessCount: 0
            //thread: fields[4]
    ]

    if(functions[fields[1]] != null) {
        log.putAll(functions[fields[1]])
    }else {
        println "error data :"+fields[1]
    }

    //println log

    log_lines.add(log)
}
//移除调用次数超过FUNC_MAX_CALL_TIMES的调用记录(仅统计调用，不统计退出)
Map <String, Integer> call_statistics=[:]
Map <String, Integer> tmp_for_access_count = [:];
log_lines.each{ Map<String, String> line->
    if(line["direct"]==">")
    {
        if(call_statistics[line["name"]]==null)
        {
            call_statistics[line["name"]]=0
        }
        call_statistics[line["name"]]++


    }
}
//-----------------------------------------------------------------------------
//根据方法的被调用次数，做过滤，调用次数太多的不记录
//-----------------------------------------------------------------------------
log_lines.each { Map<String, String> line ->
    if(FUNC_MAX_CALL_TIMES != -1 && call_statistics[line["name"]]<=FUNC_MAX_CALL_TIMES){
        log_filtered.add(line)
    }
}

//-----------------------------------------------------------------------------
//对调用日志进行匹配检查和补偿(自动完成匹配)
//-----------------------------------------------------------------------------
for(int i=0;i<log_filtered.size(); i++){
    if(log_filtered[i].match==null && log_filtered[i].direct == ">"){
        for(int j=i+1;j<log_filtered.size();j++){
            if(log_filtered[j].name == log_filtered[i].name
                    && log_filtered[j].direct == "<"
                    && log_filtered[j].match==null
            )
            {
                log_filtered[j].match=true
                log_filtered[i].match=true

                if(j==i+1)
                {
                    log_filtered[i].close=true
                }

                break;
            }
        }
    }
}

//构建补偿日志
List<String, String> patched_log=[]
for(int i=0;i<log_filtered.size();i++){
    if(log_filtered[i].match!=true && log_filtered[i].direct == ">"){

        log_filtered[i].match=true

        Map<String, String> patch =[:]
        patch.putAll(log_filtered[i])
        patch["direct"]="<"
        patch["match"]=true
        patch["fixed"]=true

        patched_log.add(patch)

        if(i==log_filtered.size()-1){
            log_filtered[i].close=true
        }
    }
}

//调用堆栈是反的，加到正常日志末尾
log_filtered.addAll(patched_log.reverse())

int line_total =0
int line_match =0
int line_close =0


log_filtered.each{ Map<String, String> line ->

    line_total++
    if(line["match"]==true) line_match++
    if(line["close"]==true) line_close++
    //println line
}



//-----------------------------------------------------------------------------
//转换函数调用日志： level, distance, kids_num, matched, brothers, kids
//pair对应的匹配记录的下标
//distance 进入函数和离开函数之间执行了多少步
//-----------------------------------------------------------------------------
int level = 0
for(int i=0;i<log_filtered.size();i++){
    int jump = 0
    if(log_filtered[i].direct == "<"){
        level--
    }
    if(log_filtered[i].direct == "<" || log_filtered[i].match!=true) continue
    for(int j=i+1;j<log_filtered.size();j++){
        //处理特殊情况，自己调用了自己，所以在这时候的匹配要跳过
        if(log_filtered[i].name == log_filtered[j].name && log_filtered[j].direct == ">") jump++;
        if(log_filtered[i].name == log_filtered[j].name && log_filtered[j].direct == "<"){
            if(jump > 0) {
                jump--
                continue
            }
            log_filtered[i].level = level
            log_filtered[j].level = level

            log_filtered[i].pair = j
            log_filtered[j].pair = i

            log_filtered[i].distance = j-i
            log_filtered[j].distance = j-i

            //寻找最近的匹配，一定要break; 否则同一个函数调用多次，会引起关系错乱
            break;
        }
    }

    if(log_filtered[i].direct == ">"){
        level++
    }
}
//-----------------------------------------------------------------------------
//生成调用说明TXT
//-----------------------------------------------------------------------------
for(int i=0;i<log_filtered.size();i++){
    if(log_filtered[i].distance >= TRACE_MIN_DISTANCE && log_filtered[i].level <= TRACE_MAX_LEVEL) {

        println "  " * log_filtered[i].level + "${log_filtered[i].direct} ${log_filtered[i].name} ${log_filtered[i].module}/${log_filtered[i].file}"
    }
}

//-----------------------------------------------------------------------------
//根据这个函数的跨度DISTANCE和层级去过过滤，顺便再做一个为多次访问的节点改名的临时数组
//-----------------------------------------------------------------------------
for(int i=0;i<log_filtered.size();i++) {
    Map item = log_filtered[i]
    if (item.distance < TRACE_MIN_DISTANCE || item.level > TRACE_MAX_LEVEL) {
        item["delete"] = true
    }
    //为进入，且判断是否正常删除
    if(item["direct"]==">" && (item["delete"] == null || !item["delete"])) {
        //上面的call_statistics为了后面过滤使用，下面的tmp_for_access_count为了被多次访问的节点改名
        if(tmp_for_access_count[item["name"]] == null) {
            tmp_for_access_count[item["name"]] = 0
        }
        tmp_for_access_count[item["name"]]++

        item["accessCount"] = tmp_for_access_count[item["name"]]
    }
}

//-----------------------------------------------------------------------------
//下面开始根据字节点计算权重,如果权重大于标准，则对import_level字段赋值
//-----------------------------------------------------------------------------
for(int i=0;i<log_filtered.size();i++) {
    Map item = log_filtered[i]

    for (int start = i; start <= item.pair; start++) {
        Map son_item = log_filtered[start]
        if (son_item["direct"] == ">" && (son_item["delete"] == null || !son_item["delete"]) && son_item.level == (item.level + 1))
            item.weight = item.weight + 1
    }
    String filename
    if(item.weight != null && item.weight >= WEIGHT_LEVEL_1 && item.weight < WEIGHT_LEVEL_2) {
        item.color = "red"
        item.url = "file://"+System.getProperty("user.dir");
    }else if(item.weight != null && item.weight >= WEIGHT_LEVEL_2) {
        item.color = "blue"
        item.url = "file://"+System.getProperty("user.dir");
    }
}

//-----------------------------------------------------------------------------
//生成DOT文件
//-----------------------------------------------------------------------------
Stack <Map> stack=[]
List<String> edgeOutput=[]
Set<String> nodeOutput = []

int start = 0
List<String> output = []
int j=1;
boolean peekFlag = false
String peekValue
String oldToFunctionName
for(int i=0;i<log_filtered.size();i++){

    //todo 通过加标志位，在这里去做拦截，找到关键函数名且为>则加1,为<则减1，大于零则记录
    if(START_FUNC != null && START_FUNC == log_filtered[i]["name"] && log_filtered[i]["direct"]==">") {
        start++
    }
    if(START_FUNC != null && START_FUNC == log_filtered[i]["name"] && log_filtered[i]["direct"]=="<") {
        start--
    }

    if(START_FUNC != null && start<1) continue
    if (log_filtered[i]["delete"] != null && log_filtered[i]["delete"]) continue
    if (log_filtered[i].direct == ">") {
        if (stack.empty() == false) {
            String fromFunctionName
            if (peekFlag) {
                if ((stack.peek())["name"] == oldToFunctionName) {
                    /*
                   名字相同代表有子节点
                   a -> b1
                   b1 -> c
                   */
                    fromFunctionName = peekValue
                } else {
                    /*
                    名字不相同代表没有子节点
                    a -> b1
                    c -> d
                    */
                    fromFunctionName = (stack.peek())["name"]
                }
                peekFlag = false
            } else {
                fromFunctionName = (stack.peek())["name"]
            }

            String toFunctionName
            if (log_filtered[i].accessCount == 1) {
                toFunctionName = log_filtered[i].name
            } else {
                //一旦toFunctionName是需要特殊处理的,就对peekFlag标志位赋值
                //并保留下一个fromFunctionName可能的值peekValue，oldToFunctionName是为了处理万一这个toFunctionName没有再调用别的函数，则不需要修改下一个fromFunctionName
                toFunctionName = (log_filtered[i].name + "_" + log_filtered[i].accessCount)
                peekValue = toFunctionName
                oldToFunctionName = log_filtered[i].name
                peekFlag = true
            }

            edgeOutput << """    ${fromFunctionName} -> ${toFunctionName} [label="${j++}"];"""
            //这里去把所有的节点记录下来
            nodeOutput << """    ${toFunctionName} [
            ${log_filtered[i].color == null ? "" : """fontcolor="${log_filtered[i].color}","""}
            label="${log_filtered[i].name}"
            tooltip=""];"""

            nodeOutput << """    ${fromFunctionName} [
            ${stack.peek().color == null ? "" : """fontcolor="${stack.peek().color}","""}
            label="${stack.peek().name}"
            tooltip=""];"""

        }

        stack.push(log_filtered[i])
    } else {
        stack.pop()
    }
}
output.add(0,"digraph trace{")
output.add(1,"""
    ranksep=2;
    rankdir=LR;
    node [shape=plaintext, fontsize=20, fontname="Microsoft Yahei"];

""")

output.addAll(nodeOutput)
output.addAll(edgeOutput)
output.add("}")

file=new File(PATH_OF_DOT)
content =""

output.each{String line->
    content = content+line + "\n"

}
file.write(content)

command = "dot ${PATH_OF_DOT} -Tsvg -o ${new Date().format( 'yyyyMMddHHmmSS' )}_${PATH_OF_SVG}"

proc = command.execute()
//proc.consumeProcessOutput(sout, serr)
proc.waitForProcessOutput(sout, serr)
if("$sout".length() != 0 || "${serr}".length() != 0) {
    println "out> $sout err> $serr"
    exitWithMessage("out> $sout err> $serr")
}

//

