cp -a /home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Librerias/libreriaRequests/{conexiones.c,requests.c,colores.c,analisis.c,requests.h} /home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Memoria/;

cp -a /home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Librerias/libreriaRequests/{conexiones.c,requests.c,colores.c,analisis.c,requests.h} /home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Kernel/;

cd /home/utnso/workspace/tp-2019-1c-U-TN-Tecno/LFS;

mkdir Debug;

cp -a /home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Librerias/libreriaRequests/{conexiones.c,requests.c,colores.c,analisis.c,requests.h} /home/utnso/workspace/tp-2019-1c-U-TN-Tecno/LFS/Debug;

gcc *.c -o LFS -lpthread -lcommons -lreadline;

cd /home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Kernel/;

gcc *.c -o kernel -lpthread -lcommons -lreadline;

cd /home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Memoria/;

gcc *.c -o memoria -lpthread -lcommons -lreadline;