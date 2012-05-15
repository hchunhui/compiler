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
   30 begin
   31     x:=m; y:=n;
   35     call multiply;
   36 end.
        30  int  0    8
        31  lit  0    7
        32  sto  0    3
        33  lit  0   85
        34  sto  0    4
        35  cal  0    2
        36  opr  0    0
start pl/0
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
end pl/0
