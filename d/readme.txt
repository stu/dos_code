D - v20161215

Recompile of my old 'd' directory lister util
Its a little more 'ls' with colours and less dos dir.

Setup your environment and stick d.exe into your path;

set D=-24 -l -d -oN
set DCLR_EXEC=yellow:exe,com,bat,cmd
set DCLR_ARC=brblue:zip,arj,rar,lzh,pak,zoo,arc,gz,tar
set DCLR_TEXT=white:txt,doc,diz,me,ans,nfo,asc
set DCLR_IMAGE=brred:jpg,gif,png,bmp,tif,tga,iff,lbm,pcx
set DCLR_CODE=cyan:c,h,cpp,hpp,pas,cc,asm,lib,obj
set DCLR_BAD=red:bak,$$$,old,tmp
set DCLR_SOUND=green:wav,mp3,mod,s3m,it,xm,mtm,669,mid,ptm,voc,mus,xmi,okt,amf
set DCLR_DIR=brcyan

or make a "d.cfg" file in the same directory that "d.exe" lives in,
and put

D=-24 -l -d -oN
DCLR_EXEC=yellow:exe,com,bat,cmd
DCLR_ARC=brblue:zip,arj,rar,lzh,pak,zoo,arc,gz,tar
DCLR_TEXT=white:txt,doc,diz,me,ans,nfo,asc
DCLR_IMAGE=brred:jpg,gif,png,bmp,tif,tga,iff,lbm,pcx
DCLR_CODE=cyan:c,h,cpp,hpp,pas,cc,asm,lib,obj
DCLR_BAD=red:bak,$$$,old,tmp
DCLR_SOUND=green:wav,mp3,mod,s3m,it,xm,mtm,669,mid,ptm,voc,mus,xmi,okt,amf
DCLR_DIR=brcyan

and it will be read each time (if you run ncache or smartdrive you will
see no slowdown as it will already be in memory).
