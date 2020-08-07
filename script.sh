rmdisk &path->"/home/guillermo/test/disco0.dsk"
mkdisk &size->50 &path->/home/guillermo/test/ \^
&name->disco0.dsk &unit->m
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->10 &unit->m &name->particion1 &type->p &fit->wf
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->10 &unit->m &name->particion2 &type->p &fit->wf
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->10 &unit->m &name->particion3 &type->e &fit->wf
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->19 &unit->m &name->particion4 &type->p &fit->wf
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->19 &unit->m &name->particion5 &type->p &fit->wf
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->3 &unit->k &name->Particion3_1 &type->l
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->3 &unit->k &name->Particion3_2 &type->l
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->3 &unit->k &name->Particion3_3 &type->l
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->3 &unit->k &name->Particion3_4 &type->l
fdisk &path->"/home/guillermo/test/disco0.dsk" &size->3 &unit->k &name->Particion3_5 &type->l
fdisk &path->"/home/guillermo/test/disco0.dsk" &name->particion1 &add->39 &unit->m
mount &path->/home/guillermo/test/disco0.dsk &name->particion1
mount &path->/home/guillermo/test/disco0.dsk &name->particion2
mount &path->/home/guillermo/test/disco1.dsk &name->particion1
mount &path->/home/guillermo/test/disco0.dsk &name->particion3
mount &path->/home/guillermo/test/disco1.dsk &name->particion2
mount &path->/home/guillermo/test/disco2.dsk &name->particion1
unmount &id->vda3
mount
mkfs &id->vda1 &type->full
mkdir &p &id->vda1 &path->/car1
mkdir &p &id->vda1 &path->/car2
mkdir &p &id->vda1 &path->/car3
mkdir &p &id->vda1 &path->/car4
mkdir &p &id->vda1 &path->/car5
mkdir &p &id->vda1 &path->/car6
mkdir &p &id->vda1 &path->/car7
mkdir &p &id->vda1 &path->/car8
mkdir &p &id->vda1 &path->/car9
mkdir &p &id->vda1 &path->/car10
mkdir &p &id->vda1 &path->/car1/car11
mkdir &p &id->vda1 &path->/car1/car12
mkdir &p &id->vda1 &path->/car1/car13
mkdir &p &id->vda1 &path->/car1/car14
mkdir &p &id->vda1 &path->/car1/car15
mkdir &p &id->vda1 &path->/car1/car16
mkdir &p &id->vda1 &path->/car1/car17
mkdir &p &id->vda1 &path->/car1/car18
mkdir &p &id->vda1 &path->/car1/car19/car191
mkdir &p &id->vda1 &path->/car1/car19/car192
mkdir &p &id->vda1 &path->/car1/car19/car193
mkdir &p &id->vda1 &path->/car1/car19/car194
mkdir &p &id->vda1 &path->/car1/car19/car195
mkdir &p &id->vda1 &path->/car1/car110
mkdir &p &id->vda1 &path->/car1
mkfile &p &size->15 &id->vdb1 &path->/car1/archivo1.txt
mkfile &p &size->30 &id->vda1 &path->/car1/car19/car191/archivo1.txt
mkfile &p &size->30 &id->vda1 &path->/car1/car19/car191/archivo2.txt
mkfile &p &size->30 &id->vda1 &path->/car1/car19/car191/archivo3.txt
mkfile &p &size->15 &id->vdb1 &path->/car1/archivo2.txt
mkfile &p &id->vda1 &path->/car2/archivo.txt &p \^
&cont->"hola como estas amigo como como estas amigo repito como estas probando un inodo mas por favor uno mas compa"
mkfile &size->15 &id->vda1 &path->/car2/archivo1.txt
mkfile &size->15 &id->vda1 &path->/archivo.txt
mkfile &size->30 &id->vda1 &path->/archivo.txt
mkfile &size->30 &id->vda1 &path->/archivo2.txt
mkfile &size->30 &id->vda1 &path->/archivo3.txt
mkfile &size->30 &id->vda1 &path->/archivo4.txt
mkfile &size->30 &id->vda1 &path->/archivo5.txt
mkfile &size->30 &id->vda1 &path->/archivo6.txt
mkfile &size->30 &id->vda1 &path->/archivo7.txt
mkfile &size->30 &id->vda1 &path->/archivo8.txt
mkfile &size->30 &id->vda1 &path->/archivo9.txt
mkfile &size->300 &id->vda1 &path->/archivo10.txt
mkfile &p &id->vda1 &path->/car10/prueba.txt &p \^
&cont->"probando archivo de texto"
mkfile &p &id->vda1 &path->/car10/prueba1.txt &p \^
&cont->"segundo archivo"
mkfile &p &id->vda1 &path->/car10/prueba2.txt &p \^
&cont->"tercer archivo"
rep &name->bm_arbdir &path->/home/guillermo/bm1.pdf &id->vda1
rep &name->bm_detdir &path->/home/guillermo/bm2.pdf &id->vda1
rep &name->bm_inode &path->/home/guillermo/bm3.pdf &id->vda1
rep &name->bm_block &path->/home/guillermo/bm4.pdf &id->vda1
rep &name->directorio &path->/home/guillermo/directorio.pdf &id->vda1
rep &name->tree_complete &path->/home/guillermo/completo.pdf &id->vda1
rep &name->tree_direct &path->/home/guillermo/dir.pdf &ruta->/car1/car19/ &id->vda1
rep &name->tree_direct &path->/home/guillermo/dir2.pdf &ruta->/ &id->vda1
rep &name->tree_file &path->/home/guillermo/dir3.pdf &ruta->/car1/car19/car191/ &id->vda1
rep &name->disk &path->/home/guillermo/disk.pdf &id->vda1
rep &name->sb &path->/home/guillermo/sb.pdf &id->vda1
rep &name->ls &path->/home/guillermo/ls1.pdf &ruta->/ &id->vda1
rep &name->ls &path->/home/guillermo/ls1.pdf &ruta->/car1/car19/car191/ &id->vda1
rep &name->bitacora &path->/home/guillermo/bitacora.pdf &id->vda1
loss &id->vda1
rep &name->bm_arbdir &path->/home/guillermo/bm1.pdf &id->vda1
rep &name->bm_detdir &path->/home/guillermo/bm2.pdf &id->vda1
rep &name->bm_inode &path->/home/guillermo/bm3.pdf &id->vda1
rep &name->bm_block &path->/home/guillermo/bm4.pdf &id->vda1
rep &name->directorio &path->/home/guillermo/directorio.pdf &id->vda1
rep &name->tree_complete &path->/home/guillermo/completo.pdf &id->vda1
rep &name->tree_direct &path->/home/guillermo/dir.pdf &ruta->/car1/car19/ &id->vda1
rep &name->tree_direct &path->/home/guillermo/dir2.pdf &ruta->/ &id->vda1
rep &name->tree_file &path->/home/guillermo/dir3.pdf &ruta->/car1/car19/car191/ &id->vda1
rep &name->disk &path->/home/guillermo/disk.pdf &id->vda1
rep &name->sb &path->/home/guillermo/sb.pdf &id->vda1
rep &name->ls &path->/home/guillermo/ls1.pdf &ruta->/ &id->vda1
rep &name->ls &path->/home/guillermo/ls1.pdf &ruta->/car1/car19/car191/ &id->vda1
recovery &id->vda1
rep &name->bm_arbdir &path->/home/guillermo/bm1.pdf &id->vda1
rep &name->bm_detdir &path->/home/guillermo/bm2.pdf &id->vda1
rep &name->bm_inode &path->/home/guillermo/bm3.pdf &id->vda1
rep &name->bm_block &path->/home/guillermo/bm4.pdf &id->vda1
rep &name->directorio &path->/home/guillermo/directorio.pdf &id->vda1
rep &name->tree_complete &path->/home/guillermo/completo.pdf &id->vda1
rep &name->tree_direct &path->/home/guillermo/dir.pdf &ruta->/car1/car19/ &id->vda1
rep &name->tree_direct &path->/home/guillermo/dir2.pdf &ruta->/ &id->vda1
rep &name->tree_file &path->/home/guillermo/dir3.pdf &ruta->/car1/car19/car191/ &id->vda1
rep &name->disk &path->/home/guillermo/disk.pdf &id->vda1
rep &name->sb &path->/home/guillermo/sb.pdf &id->vda1
rep &name->ls &path->/home/guillermo/ls1.pdf &ruta->/ &id->vda1
rep &name->ls &path->/home/guillermo/ls1.pdf &ruta->/car1/car19/car191/ &id->vda1
fdisk &path->"/home/guillermo/test/disco0.dsk" &name->particion2 &delete->full