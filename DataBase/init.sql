INSERT INTO TB_DOM_PERIF (Id, MAC, Dispositivo) VALUES (0, '0000000000000000', 'Ninguno');
INSERT INTO TB_DOM_GRUPO_VISUAL(Id, Nombre) VALUES (0, 'Ninguno');
INSERT INTO TB_DOM_ASSIGN (Id, Objeto, Dispositivo, Port, E_S, Tipo, Estado, Flags) VALUES(0, 'Ninguno', 0, 0, 0, 0, 0, 0);
INSERT INTO TB_DOM_GROUP (Id, Grupo) VALUES (0, 'Ninguno');
INSERT INTO TB_DOM_EVENT (Id, Evento, Objeto_Origen, Objeto_Destino, Grupo_Destino, Funcion_Destino, Variable_Destino) VALUES (0,'Ninguno',0,0,0,0,0);
INSERT INTO TB_DOM_FLAG (Id, Variable) VALUES (0, 'Ninguna');
INSERT INTO TB_DOM_FUNCTION (Id, Funcion) VALUES (0, 'Ninguna');
INSERT INTO TB_DOM_USER (id, Usuario, Nombre_Completo, Pin_WEB) VALUES (0, 'nadie', 'Nadie', '****');
INSERT INTO TB_DOM_USER (id, Usuario, Nombre_Completo, Pin_WEB) VALUES (1, 'admin', 'Administrador del sistema', 'admin');
INSERT INTO TB_DOM_CONFIG (id, Creacion, System_Key, Cloud_Host_1_Address, Cloud_Host_1_Proto, Cloud_Host_2_Address, Cloud_Host_2_Proto)
  VALUES (1, '2021/10/30 15:58:00', '0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF', 'witchblade.com.ar', 'http', 'pueyrredon2679.com.ar', 'http');
