please input source program file name: 
    0 const m=7, n=85;
    1 var x,y,z,q,r;
    1 procedure multiply;
    1     var a,b;
    2     begin
    3         a:=x; b:=y; z:=0;
    9         while b>0 do
   13             begin
   13                 if odd b then z:=z+a;
   20                 a:=2*a; b:=b/2;
   28             end
   28     end;
         2  int  0    5
         3  lod  1    3
         4  sto  0    3
         5  lod  1    4
         6  sto  0    4
         7  lit  0    0
         8  sto  1    5
         9  lod  0    4
        10  lit  0    0
        11  opr  0   12
        12  jpc  0   29
        13  lod  0    4
        14  opr  0    6
        15  jpc  0   20
        16  lod  1    5
        17  lod  0    3
        18  opr  0    2
        19  sto  1    5
        20  lit  0    2
        21  lod  0    3
        22  opr  0    4
        23  sto  0    3
        24  lod  0    4
        25  lit  0    2
        26  opr  0    5
        27  sto  0    4
        28  jmp  0    9
        29  opr  0    0
   30 
   30 procedure divide;
   30     var w;
   31     begin
   32         r:=x; q:=0; w:=y;
   38 	while w<=r do w:=2*w;
   47 	while w>y do
   51 	    begin
   51 	        q:=2*q; w:=w/2;
   59 		if w<=r then
   62 		    begin
   63 		        r:=r-w;
   67 			q:=q+1;
   71 		    end
   71            end
   71 end;
        31  int  0    4
        32  lod  1    3
        33  sto  1    7
        34  lit  0    0
        35  sto  1    6
        36  lod  1    4
        37  sto  0    3
        38  lod  0    3
        39  lod  1    7
        40  opr  0   13
        41  jpc  0   47
        42  lit  0    2
        43  lod  0    3
        44  opr  0    4
        45  sto  0    3
        46  jmp  0   38
        47  lod  0    3
        48  lod  1    4
        49  opr  0   12
        50  jpc  0   72
        51  lit  0    2
        52  lod  1    6
        53  opr  0    4
        54  sto  1    6
        55  lod  0    3
        56  lit  0    2
        57  opr  0    5
        58  sto  0    3
        59  lod  0    3
        60  lod  1    7
        61  opr  0   13
        62  jpc  0   71
        63  lod  1    7
        64  lod  0    3
        65  opr  0    3
        66  sto  1    7
        67  lod  1    6
        68  lit  0    1
        69  opr  0    2
        70  sto  1    6
        71  jmp  0   47
        72  opr  0    0
   73 
   73 procedure gcd;
   73     var f,g;
   74     begin
   75         f:=x;
   77 	g:=y;
   79 	while f<>g do
   83 	    begin
   83 	        if f<g then g:=g-f;
   91 		if g<f then f:=f-g;
   99 	    end
   99     end;
        74  int  0    5
        75  lod  1    3
        76  sto  0    3
        77  lod  1    4
        78  sto  0    4
        79  lod  0    3
        80  lod  0    4
        81  opr  0    9
        82  jpc  0  100
        83  lod  0    3
        84  lod  0    4
        85  opr  0   10
        86  jpc  0   91
        87  lod  0    4
        88  lod  0    3
        89  opr  0    3
        90  sto  0    4
        91  lod  0    4
        92  lod  0    3
        93  opr  0   10
        94  jpc  0   99
        95  lod  0    3
        96  lod  0    4
        97  opr  0    3
        98  sto  0    3
        99  jmp  0   79
       100  opr  0    0
  101 
  101 begin
  102     x:=m; y:=n; call multiply;
  107     x:=25; y:=3; call divide;
  112     x:=34; y:=36; call gcd;
  117 end.
       101  int  0    8
       102  lit  0    7
       103  sto  0    3
       104  lit  0   85
       105  sto  0    4
       106  cal  0    2
       107  lit  0   25
       108  sto  0    3
       109  lit  0    3
       110  sto  0    4
       111  cal  0   31
       112  lit  0   34
       113  sto  0    3
       114  lit  0   36
       115  sto  0    4
       116  cal  0   74
       117  opr  0    0
start PL/0
         7
        85
         7
        85
         0
         7
        14
        42
        28
        21
        35
        56
        10
       112
         5
       147
       224
         2
       448
         1
       595
       896
         0
        25
         3
        25
         0
         3
         6
        12
        24
        48
         0
        24
         1
         1
         2
        12
         4
         6
         8
         3
        34
        36
        34
        36
         2
        32
        30
        28
        26
        24
        22
        20
        18
        16
        14
        12
        10
         8
         6
         4
         2
end PL/0
