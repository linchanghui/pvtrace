Rm -f *.trc ANA_* FUNCS*

userid informix $INFORMIXDIR/bin/oninit1 -vy
userid informix $INFORMIXDIR/bin/onmode1 -e OFF

echo "1" >> /tmp/IFX_TRACE
chmod 666 /tmp/IFX_TRACE

sleep 0.01

userid informix $INFORMIXDIR/bin/dbaccess testdb select.sql

sleep 0.01

rm -rf /tmp/IFX_TRACE

userid informix $INFORMIXDIR/bin/onmode1 -kuy

#awk -F, '{print $3}' oninit.trc | sort | uniq >> ANA_THREADS
#awk -F, '{print $1}' oninit.trc | sed "s/[<>]//" | sort | uniq -c | sort -n | awk '{ if($1<=4) print $2}' >> ANA_FUNC_ADDR
#for addr in `cat ANA_FUNC_ADDR`; do echo "$addr `addr2line -e /INFORMIX/bin/oninit -f $addr | xargs`" | sed "s/\/vobs\/tristarm//" >> ANA_FUNCS; done;
#cat ANA_THREADS
