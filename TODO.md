Central de domótica:
    Usuarios: falta agregar a todo el sitio el control de acceso basado en usuarios.
    Grupos: ABM y manejo de grupos de objetos
    Variables:
    Alarma: Funcionalidad y configuración de alarma (va o no va?)
    Cámaras:
    Actualización:
    Descargas: 
    Riego:
    Calefacción:
    Aire:
    Tareas:

Servicios en la nube:



App
    App android para administración remota



gmd dump trace

Using host libthread_db library "/lib/arm-linux-gnueabihf/libthread_db.so.1".
0x7690b67c in __GI___libc_write (fd=<optimized out>, buf=buf@entry=0x1adfbe8, nbytes=nbytes@entry=275)
    at ../sysdeps/unix/sysv/linux/write.c:26
26      ../sysdeps/unix/sysv/linux/write.c: No existe el fichero o el directorio.
(gdb) bt
#0  0x7690b67c in __GI___libc_write (fd=<optimized out>, buf=buf@entry=0x1adfbe8, nbytes=nbytes@entry=275)
    at ../sysdeps/unix/sysv/linux/write.c:26
#1  0x76eb8f80 in CGMComm::FDWrite (datalen=<optimized out>, data=<optimized out>, this=<optimized out>) at gmcomm.cc:146
#2  CGMComm::FDWrite (this=0x1adf288, data=0x1adfbe8, datalen=275) at gmcomm.cc:139
#3  0x0001279c in MessageProc (s=s@entry=0x0, pmsg=pmsg@entry=0x1adf098) at gmd.cc:347
#4  0x00011cd8 in main (argc=<optimized out>, argv=<optimized out>) at gmd.cc:226
