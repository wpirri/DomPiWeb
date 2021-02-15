Programas del sistema de domotica
===============================================================================

Los servers corren dentro de GMonitor


Servers
-------------------------------------------------------------------------------

dompi_server
---------------------------------------
Se encarga de la logica del sistema de domotica, se instala en la Raspberry Pi que funcione como principal.


dompi_manager
---------------------------------------
Se encarga de manejar la I/O de una Raspberry Pi cuando se usa como priferico del sistema de domótica. También se instala en la central cuando estaademás tiene funciones perifericas.

Clientes
-------------------------------------------------------------------------------

infoio.cgi
---------------------------------------
Se encarga de procesar los mensajes XML de notificación de estado de los perifericos y eviarlos a dompi_server.

