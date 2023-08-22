use DB_DOMPIWEB;

INSERT INTO TB_DOM_PERIF (Id, MAC, Dispositivo) VALUES (0, '0000000000000000', 'Ninguno');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id, Nombre) VALUES (0, 'Ninguno');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id, Nombre) VALUES (1, 'Alarma');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id, Nombre) VALUES (2, 'Luces');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id, Nombre) VALUES (3, 'Puertas');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id, Nombre) VALUES (4, 'Climatización');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id, Nombre) VALUES (5, 'Cámaras');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id, Nombre) VALUES (6, 'Riego');
INSERT INTO TB_DOM_ASSIGN (Id, Objeto, Dispositivo, Port, Tipo) VALUES(0,'Ninguno','0',0,0);
INSERT INTO TB_DOM_GROUP (Id, Grupo) VALUES (0, 'Ninguno');
INSERT INTO TB_DOM_FLAG (Id, Variable) VALUES (0, 'Ninguna');
INSERT INTO TB_DOM_FUNCTION (Id, Funcion) VALUES (0, 'Ninguna');
INSERT INTO TB_DOM_EVENT (Id, Evento, Objeto_Origen, Objeto_Destino, Grupo_Destino, Funcion_Destino, Variable_Destino) VALUES (0,'Ninguno',0,0,0,0,0);
INSERT INTO TB_DOM_ALARM_PARTICION (Id, Nombre, Entrada_Act_Total, Entrada_Act_Parcial, Testigo_Activacion) VALUES (0, 'Ninguna', 0, 0, 0);
INSERT INTO TB_DOM_ALARM_ZONA (Id, Particion, Objeto_Zona) VALUES (0, 0, 0);
INSERT INTO TB_DOM_ALARM_SALIDA (Id, Particion, Objeto_Salida) VALUES (0, 0, 0);
INSERT INTO TB_DOM_USER (id, Usuario, Nombre_Completo, Pin_WEB) VALUES (0, 'nadie', 'Nadie', '****');
INSERT INTO TB_DOM_USER (id, Usuario, Nombre_Completo, Pin_WEB) VALUES (1, 'admin', 'Administrador del sistema', 'admin');
INSERT INTO TB_DOM_CONFIG (id, Creacion, System_Key, Cloud_Host_1_Address, Cloud_Host_1_Proto, Cloud_Host_2_Address, Cloud_Host_2_Proto, Planta1)
  VALUES (1, '0000/00/00 00:00:00', 'CALLEYNUM00000-CP0000', 'witchblade.com.ar', 'http', 'pueyrredon2679.com.ar', 'http', 'home1.jpg');
INSERT INTO TB_DOM_AUTO (Id, Objeto, Tipo, Objeto_Salida, Grupo_Salida, Funcion_Salida, Variable_Salida, Objeto_Sensor)
  VALUES(0,'Ninguno',1,0,0,0,0,0);
INSERT INTO TB_DOM_AT (Id, Agenda, Objeto_Destino, Grupo_Destino, Funcion_Destino, Variable_Destino) VALUES (0,'Ninguna', 0, 0, 0, 0);

CREATE USER 'dompi_web'@'%' IDENTIFIED BY 'dompi_web'; 
GRANT SELECT, INSERT, UPDATE, DELETE ON DB_DOMPIWEB.* TO 'dompi_web'@'%' WITH GRANT OPTION;
FLUSH PRIVILEGES;
