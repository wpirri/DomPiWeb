-- #### Backup creado el 240515234608
-- ####
-- ####CREATE DATABASE DB_DOMPIWEB;
-- ####CREATE USER 'dompi_web'@'%' IDENTIFIED BY 'dompi_web';
-- ####GRANT SELECT, INSERT, UPDATE, DELETE ON DB_DOMPIWEB.* TO 'dompi_web'@'%' WITH GRANT OPTION;
-- ####FLUSH PRIVILEGES;
-- ####
USE DB_DOMPIWEB;
-- ####
DELETE FROM TB_DOM_AUTO;
DELETE FROM TB_DOM_AT;
DELETE FROM TB_DOM_EVENT;
DELETE FROM TB_DOM_CAMARA;
DELETE FROM TB_DOM_ALARM_SALIDA;
DELETE FROM TB_DOM_ALARM_ZONA;
DELETE FROM TB_DOM_ALARM_PARTICION;
DELETE FROM TB_DOM_FLAG;
DELETE FROM TB_DOM_GROUP;
DELETE FROM TB_DOM_ASSIGN;
DELETE FROM TB_DOM_GRUPO_VISUAL;
DELETE FROM TB_DOM_PERIF;
DELETE FROM TB_DOM_USER;
DELETE FROM TB_DOM_CONFIG;
-- ####
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(1,'0000/00/00 00:00:00','CALLEYNUM00000-CP0000','witchblade.com.ar',0,'http','pueyrredon2679.com.ar',0,'http','','','','','','','',0,0,'','','','','','0','0','home1.jpg','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(2,'2023/05/09 18:22:04','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'http','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(3,'2023/05/09 18:22:42','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'http','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg,500,250','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(4,'2023/05/09 18:23:13','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'http','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg,250,125','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(5,'2023/05/09 18:23:41','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'http','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg,150,50','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(6,'2023/05/09 18:24:09','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'http','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg,150,30','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(7,'2023/05/09 22:22:00','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'http','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg,150,35','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(8,'2023/05/09 22:22:23','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'http','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg,150,40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(9,'2023/05/10 17:05:27','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'https','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg,150,40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(10,'2023/09/01 09:17:18','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'https','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(11,'2024/01/20 08:57:19','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'http','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(12,'2024/01/20 08:59:12','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'https','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(13,'2024/01/20 15:19:00','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'http','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(14,'2024/01/20 15:19:25','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'https','',0,'http','872e-a','00464bab872e','872e-b','00464bab872e','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(15,'2024/02/15 18:56:46','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'https','',0,'http','872e','00464bab872e','872e-backup','YE0X0.000120000000FP-10226R!^}ftgUAb7Tdtt:111000Htc_GhrSC','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(16,'2024/02/15 19:08:10','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'https','',0,'http','872e','PQF4FYyLALwrafBVP9Xfb4tKSbN9DbLV','872e-backup','3cLGSCn6gEXNg9RPbdFFAFfHvCJhEjxp','192.168.10.9','','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(17,'2024/03/02 16:27:21','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'https','',0,'http','872e','00464bab872e','','','192.168.10.5','192.168.10.9','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(18,'2024/03/03 16:52:16','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'https','',0,'http','872e','00464bab872e','','','192.168.10.5','192.168.10.9','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_CONFIG(Id,Creacion,System_Key,Cloud_Host_1_Address,Cloud_Host_1_Port,Cloud_Host_1_Proto,Cloud_Host_2_Address,Cloud_Host_2_Port,Cloud_Host_2_Proto,Wifi_AP1,Wifi_AP1_Pass,Wifi_AP2,Wifi_AP2_Pass,Home_Host_1_Address,Home_Host_2_Address,Rqst_Path,Wifi_Report,Gprs_APN_Auto,Gprs_APN,Gprs_DNS1,Gprs_DNS2,Gprs_User,Gprs_Pass,Gprs_Auth,Send_Method,Planta1,Planta2,Planta3,Planta4,Planta5,Flags)
        VALUES(19,'2024/05/13 23:21:08','PUEYRREDON2679-B1686NTU','witchblade.com.ar',0,'https','',0,'http','872e','00464bab872e','','','192.168.10.5','192.168.10.9','/cgi-bin',60,0,'','','','','','0','0','pueyrredon2679.jpg;150;40','','','','',0);
INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Acceso_Fisico,Acceso_Web,Acceso_Clowd,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (0,'nadie','Nadie','','','****','','','','','','','','','','','','','',0,0,0,0,NULL,0,'','',0);
INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Acceso_Fisico,Acceso_Web,Acceso_Clowd,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (1,'admin','Administrador del sistema','','','admin','','','','','','','','','','','','','',0,0,0,0,NULL,0,'','',0);
INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Acceso_Fisico,Acceso_Web,Acceso_Clowd,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (2,'walter','Walter Pirri','','','','','','walter@pirri.com.ar','0f9522bd','walter@pirri.com.ar','walter@pirri.com.ar','','','0069AE8B','','','','Lu,Ma,Mi,Ju,Vi,Sa,Do',0,0,0,0,1,0,'','',0);
INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Acceso_Fisico,Acceso_Web,Acceso_Clowd,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (3,'sandra','Sandra Borgna','','','','','','sandra@pirri.com.ar','katarina','004EA18B','','','','004EA18B','','','','Lu,Ma,Mi,Ju,Vi,Sa,Do',0,0,0,0,1,0,'','',0);
INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Acceso_Fisico,Acceso_Web,Acceso_Clowd,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (4,'francisco','Francisco Pirri','','','','','','','','0273D68B','','0273D68B','','','','','','Lu,Ma,Mi,Ju,Vi,Sa,Do',0,0,0,0,1,0,'','',0);
INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Acceso_Fisico,Acceso_Web,Acceso_Clowd,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (5,'graciela','Hracias Del Guercio','','','','','','','','','','','','','','','','Lu,Ma,Mi,Ju,Vi,Sa,Do',0,0,0,0,1,0,'','',0);
INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Acceso_Fisico,Acceso_Web,Acceso_Clowd,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (6,'elvira','Elvira Mendoza','','','','','','','','006AD68B','','','','006AD68B','','','','Lu,Ma,Mi,Ju,Vi,Sa,Do',0,0,0,0,1,0,'','',0);
INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Acceso_Fisico,Acceso_Web,Acceso_Clowd,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (7,'camila','Camila Pirri','','','','','','camila@pirri.com.ar','Cami2007','camila@pirri.com.ar','camila@pirri.com.ar','','','027EE39D','','','','Lu,Ma,Mi,Ju,Vi,Sa,Do',0,0,0,0,1,0,'','',0);
INSERT INTO TB_DOM_USER(Id,Usuario,Nombre_Completo,Pin_Teclado,Pin_SMS,Pin_WEB,Telefono_Voz,Telefono_SMS,Usuario_Cloud,Clave_Cloud,Amazon_key,Google_Key,Apple_Key,Other_Key,Tarjeta,Acceso_Fisico,Acceso_Web,Acceso_Clowd,Dias_Semana,Hora_Desde,Minuto_Desde,Hora_Hasta,Minuto_Hasta,Estado,Contador_Error,Ultimo_Acceso,Ultimo_Error,Flags) 
        VALUES (8,'eduardo','Eduardo Borgna','','','','','','','','02355E8A','','','','02355E8A','','','','Lu,Ma,Mi,Ju,Vi,Sa,Do',0,0,0,0,1,0,'','',0);
INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Usar_Https,Habilitar_Wiegand,Informacion) 
        VALUES(0,'0000000000000000','Ninguno',0,0,'0.0.0.0',0,0,0,0,'');
INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Usar_Https,Habilitar_Wiegand,Informacion) 
        VALUES(1,'C8C9A34A61BD','Pasillo-I',1,0,'192.168.10.120',1715827005,0,0,0,'FW: Feb 14 2023 19:48:17
SDK: 3.0.3(8427744)
AT: 1.7.3.0(Mar 19 2020 18:15:04)
');
INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Usar_Https,Habilitar_Wiegand,Informacion) 
        VALUES(2,'C8C9A34A61A6','Pasillo-D',1,0,'192.168.10.119',1715826995,0,0,0,'FW: Feb 14 2023 19:48:17
SDK: 3.0.3(8427744)
AT: 1.7.3.0(Mar 19 2020 18:15:04)
');
INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Usar_Https,Habilitar_Wiegand,Informacion) 
        VALUES(3,'ECFABC3B6642','Taller',1,0,'192.168.10.124',1715827031,0,0,0,'FW: Mar  7 2024 20:47:08
SDK: 2.2.1(6ab97e9)
AT: 1.6.2.0(Apr 13 2018 11:10:59)
SSL: NO
');
INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Usar_Https,Habilitar_Wiegand,Informacion) 
        VALUES(4,'C8C9A34A61C9','Quincho',1,0,'192.168.10.152',1715827039,0,0,0,'FW: Mar  7 2024 20:47:08
SDK: 3.0.3(8427744)
AT: 1.7.3.0(Mar 19 2020 18:15:04)
SSL: YES
');
INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Usar_Https,Habilitar_Wiegand,Informacion) 
        VALUES(5,'ECFABC3B6690','Garage-1',1,0,'192.168.10.122',1715827005,0,0,1,'FW: Feb 14 2023 19:48:17
SDK: 2.2.1(6ab97e9)
AT: 1.6.2.0(Apr 13 2018 11:10:59)
');
INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Usar_Https,Habilitar_Wiegand,Informacion) 
        VALUES(6,'c8c9a34a61e6','Garage-2',1,0,'192.168.10.133',1715827000,0,0,0,'FW: Feb 14 2023 19:48:17
SDK: 3.0.3(8427744)
AT: 1.7.3.0(Mar 19 2020 18:15:04)
');
INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Usar_Https,Habilitar_Wiegand,Informacion) 
        VALUES(7,'c8c9a34a61c2','Cocina',1,0,'192.168.10.164',1715827027,0,0,0,'FW: Feb 14 2023 19:48:17
SDK: 3.0.3(8427744)
AT: 1.7.3.0(Mar 19 2020 18:15:04)
');
INSERT INTO TB_DOM_PERIF(Id,MAC,Dispositivo,Tipo,Estado,Direccion_IP,Ultimo_Ok,Actualizar,Usar_Https,Habilitar_Wiegand,Informacion) 
        VALUES(8,'b827eb6dd1a9','Alarma',2,0,'127.0.0.1',1711310021,0,0,0,'');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id,Nombre,Descripcion,Icono) 
        VALUES (0,'Ninguno','','');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id,Nombre,Descripcion,Icono) 
        VALUES (1,'Alarma','','');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id,Nombre,Descripcion,Icono) 
        VALUES (2,'Luces','','');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id,Nombre,Descripcion,Icono) 
        VALUES (3,'Puertas','','');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id,Nombre,Descripcion,Icono) 
        VALUES (4,'Climatizacion','','');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id,Nombre,Descripcion,Icono) 
        VALUES (5,'Camaras','','');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id,Nombre,Descripcion,Icono) 
        VALUES (6,'Riego','','');
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(0,'Ninguno',0,'0',0,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(1,'Tecla 1 Comedor',1,'IO1',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(2,'Tecla 2 Comedor',1,'IO2',1,1,1,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(3,'Tecla 3 Comedor',1,'IO3',1,1,1,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(4,'Tecla 4 Comedor',1,'IO4',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(5,'Tecla 5 Comedor',1,'IO5',1,1,1,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(6,'Tecla Dormitorio Fondo',1,'IO6',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(7,'Luz Dormitorio Camila',1,'OUT1',0,0,0,'','lamp0.png','lamp1.png',2,1,390,180,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(8,'Luz Puerta Entrada',1,'OUT2',0,0,0,'','lamp0.png','lamp1.png',2,1,260,245,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(9,'Luz Living',1,'OUT3',0,0,0,'','lamp0.png','lamp1.png',2,1,350,350,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(10,'Luz Comedor',1,'OUT4',0,1,1,'','lamp0.png','lamp1.png',2,1,510,350,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(11,'Tecla 1 Living',2,'IO1',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(12,'Tecla 2 Living',2,'IO2',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(13,'Tecla 3 Living',2,'IO3',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(14,'Tecla 4 Living',2,'IO4',1,1,1,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(15,'Tecla 5 Living',2,'IO5',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(16,'Tecla Dormitorio Frente',2,'IO6',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(17,'Luz Dormitorio Fondo',2,'OUT3',0,0,0,'','lamp0.png','lamp1.png',2,1,650,180,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(18,'Luz Pasillo',2,'OUT4',0,0,0,'','lamp0.png','lamp1.png',2,1,525,245,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(19,'Tecla Luz Taller',3,'IO1',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(20,'Luz Alero Taller',3,'OUT1',0,0,0,'','lamp0.png','lamp1.png',2,1,1200,175,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(21,'Luz Terraja',3,'OUT2',0,1,1,'','lamp0.png','lamp1.png',2,1,1425,175,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(22,'Luz Deposito',3,'OUT3',0,0,0,'','lamp0.png','lamp1.png',2,1,1450,90,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(23,'Luz Taller',3,'OUT4',0,0,0,'','lamp0.png','lamp1.png',2,1,1250,90,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(24,'Tecla Luz Bano Quincho',4,'IO1',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(25,'Tecla Luz Deposito',4,'IO2',1,1,1,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(26,'Tecla Luz Quincho',4,'IO3',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(27,'Tecla Luces Fondo',4,'IO4',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(28,'Luz Bano Quincho',4,'OUT1',0,0,0,'','lamp0.png','lamp1.png',2,1,1500,175,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(29,'Luz Quincho',4,'OUT2',0,0,0,'','lamp0.png','lamp1.png',2,1,1455,325,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(30,'Timbre Fondo',4,'OUT3',5,0,0,'','campana0.png','campana1.png',0,1,1350,175,0,0,3,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(31,'Led RFID Entrada',5,'IO1',0,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(32,'Boton Timbre',5,'IO2',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(33,'Tecla Luz Garage D',5,'IO3',1,1,1,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(34,'Tecla Luz Garage I',5,'IO4',1,1,1,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(35,'Reflector Palmera',5,'OUT1',0,0,0,'','lamp0.png','lamp1.png',2,1,210,300,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(36,'Luz Garage',5,'OUT2',0,0,0,'','lamp0.png','lamp1.png',2,1,150,100,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(37,'Luz Puerta Calle',5,'OUT3',0,1,1,'','lamp0.png','lamp1.png',2,1,15,250,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(38,'Puerta Calle',5,'OUT4',5,0,0,'','key.png','key.png',3,1,70,250,0,0,3,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(39,'Lector Tarjeta Calle',5,'CARD',6,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(40,'Temperatura Taller',3,'TEMP',6,0,0,'23.3','lamp0.png','lamp1.png',0,1,1200,50,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(41,'Humedad Taller',3,'HUM',6,0,0,'34.6','lamp0.png','lamp1.png',0,1,1300,50,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(42,'Luz Ventana Living',2,'OUT2',0,0,0,'','lamp0.png','lamp1.png',2,1,260,350,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(43,'Enchufe Living',2,'OUT1',0,0,0,'','lamp0.png','lamp1.png',2,1,430,400,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(44,'Luz Pasillo Exterior',6,'OUT1',0,1,1,'','lamp0.png','lamp1.png',2,1,400,65,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(45,'Luz Piedritas',6,'OUT2',0,1,1,'','lamp0.png','lamp1.png',2,1,125,170,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(46,'Tecla 1 Cocina',7,'IO1',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(47,'Tecla 2 Cocina',7,'IO2',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(48,'Tecla 3 Cocina',7,'IO3',1,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(49,'Luz Cocina',7,'OUT1',0,0,0,'','lamp0.png','lamp1.png',2,1,640,350,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(50,'Luz Cuartito',7,'OUT2',0,1,1,'','lamp0.png','lamp1.png',2,1,700,375,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(51,'Extractor Cocina',7,'OUT3',0,0,0,'','vent0.png','vent1.png',4,1,640,300,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(52,'Lamparas Mimbre Quincho',4,'OUT4',0,0,0,'','lamp0.png','lamp1.png',2,1,1500,325,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(53,'Detector Pta. Calle',8,'IO1',4,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(54,'Detector Pileta',8,'IO2',4,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(55,'Detector Palier',8,'IO3',4,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(56,'Detector Pta. Cocina',8,'IO4',4,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(57,'Sirena exterior',8,'OUT1',3,0,0,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(58,'Zona5',8,'IO5',4,1,1,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_ASSIGN(Id,Objeto,Dispositivo,Port,Tipo,Estado,Estado_HW,Perif_Data,Icono_Apagado,Icono_Encendido,Grupo_Visual,Planta,Cord_x,Cord_y,Coeficiente,Analog_Mult_Div,Analog_Mult_Div_Valor,Actualizar,Flags) 
        VALUES(59,'Control Remoto',8,'IO6',4,1,1,'','','',0,0,0,0,0,0,1,0,0);
INSERT INTO TB_DOM_GROUP(Id,Grupo,Listado_Objetos,Estado,Actualizar) 
        VALUES(0,'Ninguno','',0,0);
INSERT INTO TB_DOM_GROUP(Id,Grupo,Listado_Objetos,Estado,Actualizar) 
        VALUES(1,'Luces Nocturnas','50,18,44,45,37,21',1,0);
INSERT INTO TB_DOM_GROUP(Id,Grupo,Listado_Objetos,Estado,Actualizar) 
        VALUES(2,'Luces Exteriores','8,20,35,36',0,0);
INSERT INTO TB_DOM_GROUP(Id,Grupo,Listado_Objetos,Estado,Actualizar) 
        VALUES(3,'Luces Interiores','7,9,10,17,18',0,0);
INSERT INTO TB_DOM_GROUP(Id,Grupo,Listado_Objetos,Estado,Actualizar) 
        VALUES(4,'Luces apagan temprano','44,45,50',0,0);
INSERT INTO TB_DOM_FLAG(Id,Variable,Valor) 
        VALUES(0,'Ninguna',0);
INSERT INTO TB_DOM_ALARM_PARTICION(Id,Nombre,Entrada_Act_Total,Entrada_Act_Parcial,Testigo_Activacion,Estado_Activacion,Estado_Memoria,Estado_Alarma,Delay_Activacion,Delay_Alarma,Tiempo_De_Salida,Tiempo_De_Entrada,Tiempo_De_Alerta,Notificar_SMS_Activacion,Notificar_SMS_Alerta) 
        VALUES(0,'Ninguna',0,0,0,0,0,0,0,0,0,0,0,0,0);
INSERT INTO TB_DOM_ALARM_PARTICION(Id,Nombre,Entrada_Act_Total,Entrada_Act_Parcial,Testigo_Activacion,Estado_Activacion,Estado_Memoria,Estado_Alarma,Delay_Activacion,Delay_Alarma,Tiempo_De_Salida,Tiempo_De_Entrada,Tiempo_De_Alerta,Notificar_SMS_Activacion,Notificar_SMS_Alerta) 
        VALUES(1,'Casa',59,0,0,0,0,0,0,0,0,0,0,0,0);
INSERT INTO TB_DOM_ALARM_ZONA(Id,Particion,Objeto_Zona,Tipo_Zona,Grupo,Activa) 
        VALUES(0,0,0,0,0,0);
INSERT INTO TB_DOM_ALARM_ZONA(Id,Particion,Objeto_Zona,Tipo_Zona,Grupo,Activa) 
        VALUES(1,1,53,0,0,1);
INSERT INTO TB_DOM_ALARM_ZONA(Id,Particion,Objeto_Zona,Tipo_Zona,Grupo,Activa) 
        VALUES(2,1,54,0,0,1);
INSERT INTO TB_DOM_ALARM_ZONA(Id,Particion,Objeto_Zona,Tipo_Zona,Grupo,Activa) 
        VALUES(3,1,55,0,0,1);
INSERT INTO TB_DOM_ALARM_ZONA(Id,Particion,Objeto_Zona,Tipo_Zona,Grupo,Activa) 
        VALUES(4,1,56,0,0,1);
INSERT INTO TB_DOM_ALARM_SALIDA(Id,Particion,Objeto_Salida,Tipo_Salida) 
        VALUES(0,0,0,0);
INSERT INTO TB_DOM_ALARM_SALIDA(Id,Particion,Objeto_Salida,Tipo_Salida) 
        VALUES(1,1,57,0);
INSERT INTO TB_DOM_CAMARA(Id,Nombre,Direccion_IP,Usuario,Clave,Protocolo,Requerimiento,Flags) 
        VALUES(0,'Ninguna','0.0.0.0','','','http','/',0);
INSERT INTO TB_DOM_CAMARA(Id,Nombre,Direccion_IP,Usuario,Clave,Protocolo,Requerimiento,Flags) 
        VALUES(1,'Camara Calle','192.168.10.14:554','admin','AUXGCQ','rtsp','/h264_stream',0);
INSERT INTO TB_DOM_CAMARA(Id,Nombre,Direccion_IP,Usuario,Clave,Protocolo,Requerimiento,Flags) 
        VALUES(2,'Camara Fondo','192.168.10.13','syshome','syshome','http','/tmpfs/auto.jpg',0);
INSERT INTO TB_DOM_CAMARA(Id,Nombre,Direccion_IP,Usuario,Clave,Protocolo,Requerimiento,Flags) 
        VALUES(3,'Camara Frente','192.168.10.12','syshome','syshome','http','/tmpfs/auto.jpg',0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(0,'Ninguno',0,0,0,0,0,0,0,0,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(1,'Luz Comedor On/Off',1,10,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(2,'Luz Living On/Off',11,9,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(3,'Luz Comedor On/Off',12,10,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(4,'Luz Living On/Off',2,9,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(5,'Luz Domitorio Frente On/Off',16,7,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(6,'Luz Domitorio Fondo On/Off',6,17,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(7,'Luz Pasillo On/Off',3,18,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(8,'Luz Puerta Entrada On/Off',4,8,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(9,'Luz Garage On/Off',5,36,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(10,'Luz Taller On/Off',19,23,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(11,'Luz Deposito On/Off',25,22,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(12,'Luz Bano Quincho On/Off',24,28,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(13,'Luz Quincho On/Off',26,29,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(14,'Replicar Timbre',32,30,0,0,0,1,0,4,3,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(15,'Abrir Puerta Calle',39,38,0,0,0,0,1,4,5,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(16,'Luz Taparrollo On/Off',13,42,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(17,'Luces Exteriores On/Off',27,0,2,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(18,'Luz Garage On/Off',34,36,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(19,'Luz Garage On/Off',33,36,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(20,'Luz Cocina On/Off',46,49,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(21,'Luz Cuartito On/Off',47,50,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_EVENT(Id,Evento,Objeto_Origen,Objeto_Destino,Grupo_Destino,Particion_Destino,Variable_Destino,ON_a_OFF,OFF_a_ON,Enviar,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Flags) 
        VALUES(22,'Extractor Cocina On/Off',48,51,0,0,0,1,1,3,0,0,0,0,0);
INSERT INTO TB_DOM_AT(Id,Agenda,Mes,Dia,Hora,Minuto,Dias_Semana,Objeto_Destino,Grupo_Destino,Evento,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Ultimo_Mes,Ultimo_Dia,Ultima_Hora,Ultimo_Minuto,Flags) 
        VALUES(0,'Ninguna',0,0,0,0,'',0,0,0,0,0,0,0,0,0,0,0,0);
INSERT INTO TB_DOM_AT(Id,Agenda,Mes,Dia,Hora,Minuto,Dias_Semana,Objeto_Destino,Grupo_Destino,Evento,Parametro_Evento,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Ultimo_Mes,Ultimo_Dia,Ultima_Hora,Ultimo_Minuto,Flags) 
        VALUES(1,'Apagar algunas luces',0,0,22,0,'Lu,Ma,Mi,Ju,Vi,Sa,Do',0,4,2,0,0,0,0,3,2,22,0,0);
INSERT INTO TB_DOM_AUTO(Id,Objeto,Tipo,Objeto_Sensor,Objeto_Salida,Grupo_Salida,Particion_Salida,Variable_Salida,Parametro_Evento,Min_Sensor,Enviar_Min,Max_Sensor,Enviar_Max,Hora_Inicio,Minuto_Inicio,Hora_Fin,Minuto_Fin,Dias_Semana,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Estado,Habilitado,Icono_Apagado,Icono_Encendido,Icono_Auto,Grupo_Visual,Planta,Cord_x,Cord_y,Actualizar,Flags) 
        VALUES(0,'Ninguno',1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0,0,0,0,1,'','','',0,0,0,0,0,0);
INSERT INTO TB_DOM_AUTO(Id,Objeto,Tipo,Objeto_Sensor,Objeto_Salida,Grupo_Salida,Particion_Salida,Variable_Salida,Parametro_Evento,Min_Sensor,Enviar_Min,Max_Sensor,Enviar_Max,Hora_Inicio,Minuto_Inicio,Hora_Fin,Minuto_Fin,Dias_Semana,Condicion_Variable,Condicion_Igualdad,Condicion_Valor,Estado,Habilitado,Icono_Apagado,Icono_Encendido,Icono_Auto,Grupo_Visual,Planta,Cord_x,Cord_y,Actualizar,Flags) 
        VALUES(1,'Luces Nocturnas',4,0,0,1,0,0,0,0,1,0,2,18,15,7,30,'Lu,Ma,Mi,Ju,Vi,Sa,Do',0,0,0,1,2,'','','',0,0,0,0,0,0);
