SELECT "INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(",Id,",'",Creacion,"','",System_Key,"','",Cloud_Host_1_Address,"',",Cloud_Host_1_Port,",'",Cloud_Host_1_Proto,"','",Cloud_Host_2_Address,"',",Cloud_Host_2_Port,",'",Cloud_Host_2_Proto,"','",Wifi_AP1,"','",Wifi_AP1_Pass,"','",Wifi_AP2,"','",Wifi_AP2_Pass,"','",Home_Host_1_Address,"','",Home_Host_2_Address,"','",Rqst_Path,"',",Wifi_Report,",",Gprs_APN_Auto,",'",Gprs_APN,"','",Gprs_DNS1,"','",Gprs_DNS2,"','",Gprs_User,"','",Gprs_Pass,"','",Gprs_Auth,"','",Send_Method,"','",Planta1,"','",Planta2,"','",Planta3,"','",Planta4,"','",Planta5,"',",Flags,");"
        FROM TB_DOM_CONFIG;
SELECT "INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Permisos,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (",Id,",'",Usuario,"','",Nombre_Completo,"','",Pin_Teclado,"','",Pin_SMS,"','",Pin_WEB,"','",Telefono_Voz,"','",Telefono_SMS,"','",Usuario_Cloud,"','",Clave_Cloud,"','",Tarjeta,"','",Amazon_key,"','",Google_Key,"','",Apple_Key,"','",Other_Key,"','",Permisos,"','",Dias_Semana,"',",Hora_Desde,",",Minuto_Desde,",",Hora_Hasta,",",Minuto_Hasta,",",Estado,",",Contador_Error,",'",Ultimo_Acceso,"','",Ultimo_Error,"',",Flags,");" 
        FROM TB_DOM_USER;
SELECT "INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Flags) 
        VALUES(",Id,",'",MAC,"','",Dispositivo,"',",Tipo,",",Estado,",'",Direccion_IP,"','",Ultimo_Ok,"',",Actualizar,",",Flags,");" 
        FROM TB_DOM_PERIF;
SELECT "INSERT INTO TB_DOM_GRUPO_VISUAL(Id,Nombre,Descripcion,Icono) 
        VALUES (",Id,",'",Nombre,"','",Descripcion,"','",Icono,"');" 
        FROM TB_DOM_GRUPO_VISUAL;
SELECT "INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(",Id,",'",Objeto,"',",Dispositivo,",'",Port,"',",Tipo,",",Estado,",",Estado_HW,",'",Perif_Data,"','",Icono_Apagado,"','",Icono_Encendido,"',",Grupo_Visual,",",Planta,",",Cord_x,",",Cord_y,",",Coeficiente,",",Analog_Mult_Div,",",Analog_Mult_Div_Valor,",",Actualizar,",",Flags,");" 
        FROM TB_DOM_ASSIGN;
SELECT "INSERT INTO TB_DOM_GROUP(Id,Grupo,Listado_Objetos,Estado,Actualizar) 
        VALUES(",Id,",'",Grupo,"','",Listado_Objetos,"',",Estado,",",Actualizar,");" 
        FROM TB_DOM_GROUP;
SELECT "INSERT INTO TB_DOM_FLAG(Id,Variable,Valor) 
        VALUES(",Id,",'",Variable,"',",Valor,");" 
        FROM TB_DOM_FLAG;
SELECT "INSERT INTO TB_DOM_FUNCTION(Id,Funcion) 
        VALUES(",Id,",'",Funcion,"');" 
        FROM TB_DOM_FUNCTION;
SELECT "INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Funcion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(",Id,",'",Evento,"',",Objeto_Origen,",",Objeto_Destino,",",Grupo_Destino,",",Funcion_Destino,",",Variable_Destino,",",ON_a_OFF,",",OFF_a_ON,",",Enviar,",",Parametro_Evento,",",Condicion_Variable,",",Condicion_Igualdad,",",Condicion_Valor,",",Flags,");" 
        FROM TB_DOM_EVENT;
SELECT "INSERT INTO TB_DOM_AT(Id,Agenda,Mes,Dia,Hora,Minuto,Dias_Semana,Objeto_Destino,Grupo_Destino,Funcion_Destino,Variable_Destino,Evento,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Ultimo_Mes,Ultimo_Dia,Ultima_Hora,Ultimo_Minuto,Flags) 
        VALUES(",Id,",'",Agenda,"',",Mes,",",Dia,",",Hora,",",Minuto,",'",Dias_Semana,"',",Objeto_Destino,",",Grupo_Destino,",",Funcion_Destino,",",Variable_Destino,",",Evento,",",Parametro_Evento,",",Condicion_Variable,",",Condicion_Igualdad,",",Condicion_Valor,",",Ultimo_Mes,",",Ultimo_Dia,",",Ultima_Hora,",",Ultimo_Minuto,",",Flags,");" 
        FROM TB_DOM_AT;

SELECT "INSERT INTO TB_DOM_AUTO(Id,Objeto,Tipo,Objeto_Sensor,Objeto_Salida,Grupo_Salida,Funcion_Salida,Variable_Salida,Parametro_Evento,Min_Sensor,Enviar_Min,Max_Sensor,Enviar_Max,Hora_Inicio,Minuto_Inicio,Hora_Fin,Minuto_Fin,Dias_Semana,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Estado,Habilitado,Icono_Apagado,Icono_Encendido,Icono_Auto,Grupo_Visual,Planta,Cord_x,Cord_y,Actualizar,Flags) 
        VALUES(",Id,",'",Objeto,"',",Tipo,",",Objeto_Sensor,",",Objeto_Salida,",",Grupo_Salida,",",Funcion_Salida,",",Variable_Salida,",",Parametro_Evento,",",Min_Sensor,",",Enviar_Min,",",Max_Sensor,",",Enviar_Max,",",Hora_Inicio,",",Minuto_Inicio,",",Hora_Fin,",",Minuto_Fin,",'",Dias_Semana,"',",Condicion_Variable,",",Condicion_Igualdad,",",Condicion_Valor,",",Estado,",",Habilitado,",'",Icono_Apagado,"','",Icono_Encendido,"','",Icono_Auto,"',",Grupo_Visual,",",Planta,",",Cord_x,",",Cord_y,",",Actualizar,",",Flags,");" 
        FROM TB_DOM_AUTO;

-- TB_DOM_ALARM_SALIDA;
-- TB_DOM_ALARM_ZONA;
-- TB_DOM_ALARM_PARTICION;
