
program Bitmap;

{ GameBoy Title Maker, 12-Oct-95 }
{  by Jeff Frohwein }

uses Crt;

const
 esc = 27;

 map_width = 48;
 map_height = 8;

var
 error:boolean;
 map:array [1..map_width,1..map_height] of byte;

procedure clear_map;
 var x,y:byte;
 begin
  for y:=1 to map_height do
   for x:=1 to map_width do
    map[x,y]:=0;
 end;

procedure draw_pixel(x,y:byte);
 begin
  if map[x,y]=0 then
   write('.')
  else
   write('*');
 end;

procedure display_map;
 var x,y:byte;
 begin
  for y:=1 to map_height do
   begin
    for x:=1 to map_width do
     draw_pixel(x,y);
    writeln;
   end;
 end;

function hex(b:byte):char;
 begin
  if b<10 then
   hex:=chr(ord('0')+b)
  else
   hex:=chr(ord('a')+b-10);
 end;

function nib(x,y,o:byte):byte;
 var
  i,t,xx,yy:byte;
 begin
  xx:=(((x-1)*4)+1);
  yy:=(((y-1)*2)+1);
  t:=0;
  for i:=0 to 3 do
   begin
    t:=t shl 1;
    t:=t+map[xx+i,yy+o];
   end;
  nib:=t;
 end;

procedure save_map;
 var
  first:boolean;
  h,l,x,y,z:byte;
  s:string[80];
  f:text;

 begin
  writeln('Saving data to "data.asm".');

  assign(f,'data.asm');
  rewrite(f);

  s:=' db ';
  first:=true;
  for z:=0 to 1 do
   for x:=1 to map_width shr 2 do
    for y:=1 to 2 do
     begin
      if length(s)>70 then
       begin
        writeln(f,s);
        s:=' db ';
        first:=true;
       end;
      h:=nib(x,y+(z*2),0);
      l:=nib(x,y+(z*2),1);
      if not(first) then
       s:=concat(s,',');
      first:=false;
      if h>9 then s:=concat(s,'0');
      if (h<>0) or (l>9) then s:=concat(s,hex(h));
      s:=concat(s,hex(l));
      if (h<>0) or (l>9) then s:=concat(s,'h');
     end;
  writeln(f,s);

  close(f);

 end;

function file_exists(fname:string):boolean;
 var f:file;
 begin
  {$I-}
  assign(f,fname);
  reset(f);
  close(f);
  {$I+}
  file_exists:=(IOResult=0) and (fname<>'');
 end;

procedure load_map;
 var
  x,y:byte;
  s:string;
  f:text;

 begin
  if file_exists('title.map') then
   begin

    {Load Configuration}

    Assign(f,'title.map');
    Reset(f);

    y:=1;
    while not(eof(f)) do
     begin
      Readln(f,s);
      for x:=1 to length(s) do
       if (x < map_width+1) and
          (y < map_height+1) then
        map[x,y]:=ord(s[x]);
      y:=y+1;
     end;
    close(f);
   end
  else
   begin
    writeln('File "title.map" not found.');
    writeln;
    write('Press any key to continue.');
    repeat until KeyPressed;
   end;
 end;

function translate_map:boolean;
 var
  error:boolean;
  a,b,c,x,y:byte;
 begin
  error:=false;

  {first pass}

  c:=0;
  for y:=1 to map_height do
   for x:=1 to map_width do
    begin
     case c of
      0:begin
         a:=map[x,y];
         c:=c+1;
        end;
      1:begin
         if map[x,y]<>a then
          begin
           b:=map[x,y];
           c:=c+1;
          end;
        end;
      else
       begin
        if (map[x,y]<>a) and
           (map[x,y]<>b) then
         begin
          write('Only two characters allowed in "title.map".');
          error:=true;
         end;
       end;
     end;
    end;

  { Make sure b is the largest value }

  if a>b then
   begin
    c:=a;
    a:=b;
    b:=c;
   end;

  {second pass}

  for y:=1 to map_height do
   for x:=1 to map_width do
    begin
     if map[x,y]=a then
      map[x,y]:=0
     else
      map[x,y]:=1;
    end;

  translate_map:=error;
 end;

procedure modify_routine;
 var
  x,y:byte;
  ch:char;

 begin
  x:=1;
  y:=1;

  repeat

   GotoXY(x,y);
   ch:=readKey;

   if ch<>chr(esc) then
    begin
     case ch of
      ' ':begin
           map[x,y]:=map[x,y] xor 1;
           draw_pixel(x,y);
          end;
      'i':y:=y-1;
      'j':x:=x-1;
      'k':x:=x+1;
      'm':y:=y+1;
     end;
    end;

  until ch=chr(esc);
 end;

begin

 writeln('** Nintendo Gameboy Title Maker, 9-12-95 **');
 writeln;

 clear_map;

 load_map;

 error:=translate_map;

{ display_map;}

 {modify_routine;}

 if not(error) then save_map;

{ repeat until KeyPressed;}
end.