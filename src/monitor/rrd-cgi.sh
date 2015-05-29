#!/usr/local/bin/rrdcgi --goodfor 60 --refresh
<HTML>
<HEAD>
<TITLE>Polygraph Stats</TITLE>
<META HTTP-EQUIV="Refresh" CONTENT="60">
</HEAD>
<BODY>

<RRD::GRAPH xact.6hour.png --title="Transactions -- 6hour"
        --start -6hour
        --imgformat PNG
        --vertical-label "count"
        --width 300 --height 150
        DEF:xact=/usr/local/apache/share/htdocs/polyrrd/test.rrd:xact:AVERAGE
        DEF:err=/usr/local/apache/share/htdocs/polyrrd/test.rrd:err:AVERAGE
        AREA:xact#0000FF:Xactions
        AREA:err#00FF00:Errors
        >

<RRD::GRAPH rt.6hour.png --title="Response Time -- 6hour"
        --start -6hour
        --imgformat PNG
        --vertical-label "seconds"
        --width 300 --height 150
        DEF:rt=/usr/local/apache/share/htdocs/polyrrd/test.rrd:rt:AVERAGE
        AREA:rt#0000FF:Mean_Response_Time
        >

<RRD::GRAPH sock.6hour.png --title="Open Sockets -- 6hour"
        --start -6hour
        --imgformat PNG
        --vertical-label "count"
        --width 300 --height 150
        DEF:sock=/usr/local/apache/share/htdocs/polyrrd/test.rrd:sock:AVERAGE
        AREA:sock#0000FF:Open_Sockets
        >

<RRD::GRAPH dhr.6hour.png --title="Hit Ratio -- 6hour"
        --start -6hour
        --imgformat PNG
        --vertical-label "percent"
        --width 300 --height 150
        DEF:dhr=/usr/local/apache/share/htdocs/polyrrd/test.rrd:dhr:AVERAGE
        AREA:dhr#0000FF:Doc_Hit_Ratio
        >

</center>
</BODY>
</HTML>
