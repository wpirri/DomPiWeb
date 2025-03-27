CREATE DATABASE DB_DOMPIWEB;
USE DB_DOMPIWEB;

DROP TABLE IF EXISTS TB_DOM_TOUCH;
DROP TABLE IF EXISTS TB_DOM_AUTO;
DROP TABLE IF EXISTS TB_DOM_AT;
DROP TABLE IF EXISTS TB_DOM_EVENT;
DROP TABLE IF EXISTS TB_DOM_CAMARA;
DROP TABLE IF EXISTS TB_DOM_ALARM_SALIDA;
DROP TABLE IF EXISTS TB_DOM_ALARM_ZONA;
DROP TABLE IF EXISTS TB_DOM_ALARM_PARTICION;
DROP TABLE IF EXISTS TB_DOM_FLAG;
DROP TABLE IF EXISTS TB_DOM_GROUP;
DROP TABLE IF EXISTS TB_DOM_ASSIGN;
DROP TABLE IF EXISTS TB_DOM_GRUPO_VISUAL;
DROP TABLE IF EXISTS TB_DOM_PERIF;
DROP TABLE IF EXISTS TB_DOM_USER;
DROP TABLE IF EXISTS TB_DOM_CONFIG;


CREATE TABLE IF NOT EXISTS TB_DOM_CONFIG (
Id integer primary key,
Creacion varchar(32),
System_Key varchar(256),
Cloud_Host_1_Address varchar(64),
Cloud_Host_1_Port integer DEFAULT 0,
Cloud_Host_1_Proto varchar(8),
Cloud_Host_2_Address varchar(64),
Cloud_Host_2_Port integer DEFAULT 0,
Cloud_Host_2_Proto varchar(8),
Wifi_AP1 varchar(33),
Wifi_AP1_Pass varchar(65),
Wifi_AP2 varchar(33),
Wifi_AP2_Pass varchar(65),
Home_Host_1_Address varchar(64),
Home_Host_2_Address varchar(64),
Rqst_Path varchar(256),
Wifi_Report integer DEFAULT 0,
Gprs_APN_Auto integer DEFAULT 0,
Gprs_APN varchar(33),
Gprs_DNS1 varchar(16),
Gprs_DNS2 varchar(16),
Gprs_User varchar(17),
Gprs_Pass varchar(17),
Gprs_Auth integer DEFAULT 0,		-- 1:PAP 2:CHAP 3:PAP/CHAP
Send_Method integer DEFAULT 0,	-- 1: First Wifi 2: First GPRS 3: Paralell
Planta1 varchar(256),
Planta2 varchar(256),
Planta3 varchar(256),
Planta4 varchar(256),
Planta5 varchar(256),
Flags integer DEFAULT 0,
UNIQUE INDEX idx_config_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_USER (
Id integer primary key,
Usuario varchar(16) NOT NULL,         -- Nombre corto (1 palabra)
Nombre_Completo varchar(64) NOT NULL,        -- Nombre completo
Pin_Teclado varchar(32),
Pin_SMS varchar(32),
Pin_WEB varchar(32),
Telefono_Voz varchar(32),
Telefono_SMS varchar(32),
Usuario_Cloud varchar(256),
Clave_Cloud varchar(256),
Amazon_Key varchar(256),
Google_Key varchar(256),
Apple_Key varchar(256),
Other_Key varchar(256),
Tarjeta varchar(256),
Acceso_Fisico varchar(256),     -- Id de Puertas de acceso separadas por , (comas)
Acceso_Web varchar(256),
Acceso_Clowd varchar(256),
Dias_Semana varchar(128),
Hora_Desde integer DEFAULT 0,
Minuto_Desde integer DEFAULT 0,
Hora_Hasta integer DEFAULT 0,
Minuto_Hasta integer DEFAULT 0,
Estado varchar(32), -- Habilitado, Bloqueado [motivo] 
Contador_Error integer DEFAULT 0,
Ultimo_Acceso varchar(32),
Ultimo_Error varchar(32),
Flags integer DEFAULT 0,
UNIQUE INDEX idx_user_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_PERIF (
Id integer primary key,
MAC varchar(16) NOT NULL,                       -- MAC Address
Dispositivo varchar(128) NOT NULL,
Tipo integer DEFAULT 0,                         -- 0=Ninguno, 1=Wifi 2=RBPi 3=DSC 4=Garnet
Estado integer DEFAULT 0,                       -- 0=Offline
Direccion_IP varchar(16) DEFAULT "0.0.0.0",
Ultimo_Ok integer DEFAULT 0,
Usar_Https integer DEFAULT 0,
Habilitar_Wiegand integer DEFAULT 0,
Update_Firmware integer DEFAULT 0,
Update_WiFi integer DEFAULT 0,
Update_Config integer DEFAULT 0,

Informacion varchar(1024),
UNIQUE INDEX idx_perif_id (Id),
UNIQUE INDEX idx_perif_mac (MAC)
);

CREATE TABLE TB_DOM_GRUPO_VISUAL (
Id integer primary key,
Nombre varchar(32) NOT NULL,
Descripcion varchar(256),
Icono varchar(32),
UNIQUE INDEX idx_grp_vis_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_ASSIGN (
Id integer primary key,
Objeto varchar(128) NOT NULL,               -- Nombre para identificarlo en el sistema
Dispositivo integer NOT NULL,               -- Discpositivo - Id de TB_DOM_PERIF
Port varchar(128) NOT NULL,                 -- Nombre con el que se identifica en el dispositivo
Tipo integer NOT NULL,                      -- 0=Output, 1=Input, 2=Analog, 3=Output Alarma, 4=Input Alarma, 5=Output Pulse/Analog_Mult_Div_Valor=Pulse Param, 6=Periferico
Estado integer DEFAULT 0,                   -- 1 / 0 para digitales 0 a n para analogicos
Estado_HW integer DEFAULT 0,                -- Estado reportado por el HW
Perif_Data varchar(128),
Icono_Apagado varchar(32),
Icono_Encendido varchar(32),
Grupo_Visual integer DEFAULT 0,             -- 0=Ninguno 1=Alarma 2=Iluminación 3=Puertas 4=Climatización 5=Cámaras 6=Riego
Planta integer DEFAULT 0,
Cord_x integer DEFAULT 0,
Cord_y integer DEFAULT 0,
Coeficiente integer DEFAULT 0,              -- 1=Coeficiente Positivo, -1=Coeficiente Negativo  - rc = Coeficiente * ( (Analog_Mult_Div)?Estado/Analog_Mult_Div_Valor:Estado*Analog_Mult_Div_Valor ) 
Analog_Mult_Div integer DEFAULT 0,          -- 0=Multiplicar por valor, 1=Dividir por valor
Analog_Mult_Div_Valor integer DEFAULT 1,    -- Parámetro para coeficiente si Tipo=2, Tiempo si Tipo=5
Actualizar integer DEFAULT 0,                   -- Enviar update de config al HW por este PORT
Flags integer DEFAULT 0,
FOREIGN KEY(Dispositivo) REFERENCES TB_DOM_PERIF(Id),
FOREIGN KEY(Grupo_Visual) REFERENCES TB_DOM_GRUPO_VISUAL(Id),
UNIQUE INDEX idx_assign_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_GROUP (
Id integer primary key,
Grupo varchar(128) NOT NULL,
Listado_Objetos varchar(256),       -- Id de assign separados por , (comas)
Estado integer DEFAULT 0,            -- Define el estado que deben tener los objetos del grupo
Icono_Apagado varchar(32),
Icono_Encendido varchar(32),
Grupo_Visual integer DEFAULT 0,             -- 0=Ninguno 1=Alarma 2=Iluminación 3=Puertas 4=Climatización 5=Cámaras 6=Riego
Planta integer DEFAULT 0,
Cord_x integer DEFAULT 0,
Cord_y integer DEFAULT 0,
Actualizar integer DEFAULT 0,
UNIQUE INDEX idx_group_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_FLAG (
Id integer primary key,
Variable varchar(128) NOT NULL,
Valor integer DEFAULT 0,
UNIQUE INDEX idx_flag_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_ALARM_PARTICION (
Id integer primary key,
Nombre varchar(128) NOT NULL,
Entrada_Act_Total integer NOT NULL,        -- Entrada que arma total o desarma la particion de la alarma
Entrada_Act_Parcial integer NOT NULL,        -- Entrada que arma parcial o desarma la particion de la alarma
Testigo_Activacion integer NOT NULL,        -- Salida que muestra el estado de la particion de la alarma
Estado_Activacion integer DEFAULT 0,        -- 0= Desactivada 1= Activacion Parcial 2= Activacion Total
Estado_Memoria integer DEFAULT 0,
Estado_Alarma integer DEFAULT 0,            -- Se carga con Tiempo_De_Alerta al dispararse la alarma y se decrementa cada segundo
Delay_Activacion integer DEFAULT 0,         -- Se carga con Tiempo_De_Salida al activarse la alarma  y se decrementa cada segundo
Delay_Alarma integer DEFAULT 0,             -- Se carga con Tiempo_De_Entrada al alertarse una zona demorada
Tiempo_De_Salida integer DEFAULT 0,
Tiempo_De_Entrada integer DEFAULT 0,
Tiempo_De_Alerta integer DEFAULT 0,          -- En segundos
Notificar_SMS_Activacion integer DEFAULT 0,
Notificar_SMS_Alerta integer DEFAULT 0,
FOREIGN KEY(Entrada_Act_Total) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Entrada_Act_Parcial) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Testigo_Activacion) REFERENCES TB_DOM_ASSIGN(Id),
UNIQUE INDEX idx_ap_id (Id),
UNIQUE INDEX idx_ap_nombre (Nombre)
);

CREATE TABLE IF NOT EXISTS TB_DOM_ALARM_ZONA (
Id integer primary key,
Particion integer NOT NULL,
Objeto_Zona integer NOT NULL,
Tipo_Zona integer DEFAULT 0,    -- 0= Normal 1= Demora 2= Incendio 3= Panico 4= Emergencia médica
Grupo integer DEFAULT 0,        -- 0= Solo Total 1= Siempre 3= 24Hs
Activa integer DEFAULT 0,
FOREIGN KEY(Particion) REFERENCES TB_DOM_ALARM_PARTICION(Id),
FOREIGN KEY(Objeto_Zona) REFERENCES TB_DOM_ASSIGN(Id),
UNIQUE INDEX idx_az_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_ALARM_SALIDA (
Id integer primary key,
Particion integer NOT NULL,
Objeto_Salida integer NOT NULL,
Tipo_Salida integer DEFAULT 0,                   -- 0= Sirena 1=Buzer 2=Testigo
FOREIGN KEY(Particion) REFERENCES TB_DOM_ALARM_PARTICION(Id),
FOREIGN KEY(Objeto_Salida) REFERENCES TB_DOM_ASSIGN(Id),
UNIQUE INDEX idx_as_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_CAMARA (
Id integer primary key,
Nombre varchar(128) NOT NULL,
Direccion_IP varchar(32) DEFAULT "0.0.0.0",
Usuario varchar(16),
Clave varchar(16),
Protocolo varchar(16) DEFAULT "http",
Requerimiento varchar(256) DEFAULT "/",
Flags integer DEFAULT 0,
UNIQUE INDEX idx_cam_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_EVENT (
Id integer primary key,
Evento varchar(128) NOT NULL,
Objeto_Origen integer DEFAULT 0,
Objeto_Destino integer  DEFAULT 0,      -- Solo uno de los cinco assign, grupo, Funcion, Particion, Variable
Grupo_Destino integer  DEFAULT 0,       -- Solo uno de los cinco assign, grupo, Funcion, Particion, Variable
Particion_Destino integer  DEFAULT 0,   -- Solo uno de los cinco assign, grupo, Funcion, Particion, Variable
Variable_Destino integer  DEFAULT 0,    -- Solo uno de los cinco assign, grupo, Funcion, Particion, Variable
ON_a_OFF integer DEFAULT 0,
OFF_a_ON integer DEFAULT 0,
Enviar integer DEFAULT 0,               -- Evento a enviar 
                                        --      0=Nada 
                                        --      1=On 
                                        --      2=Off 
                                        --      3=Switch 
                                        --      4=Pulso a Objeto o Grupo. Si no Variable = Enviar
Parametro_Evento integer DEFAULT 0,     -- Se pasa si es Variable o Funcion
Condicion_Variable integer DEFAULT 0,             -- Condiciona el evento
Condicion_Igualdad integer DEFAULT 0,             -- 0 ==, 1 >, 2 <
Condicion_Valor integer DEFAULT 0,                -- Valor de condicion
Filtro_Repeticion integer DEFAULT 0,              -- Segundos para ignorar repeticiones
Ultimo_Evento  integer DEFAULT 0,
Flags integer DEFAULT 0,
FOREIGN KEY(Objeto_Origen) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Objeto_Destino) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Grupo_Destino) REFERENCES TB_DOM_GROUP(Id),
FOREIGN KEY(Particion_Destino) REFERENCES TB_DOM_ALARM_PARTICION(Id),
FOREIGN KEY(Variable_Destino) REFERENCES TB_DOM_FLAG(Id),
UNIQUE INDEX idx_event_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_AT (
Id integer primary key,
Agenda varchar(128) NOT NULL,
Mes integer DEFAULT 0,
Dia integer DEFAULT 0,
Hora integer DEFAULT 0,
Minuto integer DEFAULT 0,
Dias_Semana varchar(128),
Objeto_Destino integer  DEFAULT 0,        -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Grupo_Destino integer  DEFAULT 0,         -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Variable_Destino integer  DEFAULT 0,        -- Solo uno de los cuatro assign, grupo, Funcion, Variable
Evento integer DEFAULT 0,               -- Evento a enviar 0=Nada 1=On 2=Off 3=Switch 4=Pulso a Objeto o Grupo. Si no Variable = Enviar
Parametro_Evento integer DEFAULT 0,     -- Se pasa si es Variable o Funcion
Condicion_Variable integer DEFAULT 0,             -- Condiciona el evento
Condicion_Igualdad integer DEFAULT 0,             -- ==, >, <
Condicion_Valor integer DEFAULT 0,                -- Valor de condicion
Ultimo_Mes integer DEFAULT 0,
Ultimo_Dia integer DEFAULT 0,
Ultima_Hora integer DEFAULT 0,
Ultimo_Minuto integer DEFAULT 0,
Flags integer DEFAULT 0,
FOREIGN KEY(Objeto_Destino) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Grupo_Destino) REFERENCES TB_DOM_GROUP(Id),
FOREIGN KEY(Variable_Destino) REFERENCES TB_DOM_FLAG(Id),
UNIQUE INDEX idx_at_id (Id)
);

-- Sistema de riego
-- Calefaccion
-- Aire acondicionado
-- Fotocelula
CREATE TABLE IF NOT EXISTS TB_DOM_AUTO (
Id integer primary key,
Objeto varchar(128) NOT NULL,               -- Nombre para identificarlo en el sistema
Tipo integer default 0,                     -- 0 = Riego 1 = Calefaccion 2 = Aire acondicionado 3 = Fotocelula
Objeto_Sensor integer default 0,             -- Discpositivo - Id de input de TB_DOM_ASSIGN
Objeto_Salida integer default 0,             -- Discpositivo - Id de TB_DOM_ASSIGN
Grupo_Salida integer default 0,              -- Grupo - Id de TB_DOM_GROUP
Particion_Salida integer default 0,          -- Particion - Id de TB_DOM_ALARM_PARTICION
Variable_Salida integer default 0,           -- Variable - Id de TB_DOM_FLAG
Parametro_Evento integer default 0,         -- Se pasa si es Variable o Funcion
Min_Sensor integer DEFAULT 0,
Enviar_Min integer default 0,               -- Accion a enviar al pasar el minimo
Max_Sensor integer DEFAULT 0,
Enviar_Max integer default 0,               -- Accion a enviar al pasar el màximo
Hora_Inicio integer DEFAULT 0,
Minuto_Inicio integer DEFAULT 0,
Hora_Fin integer DEFAULT 0,
Minuto_Fin integer DEFAULT 0,
Dias_Semana varchar(128),                   -- Lu,Ma,Mi,Ju,Vi,Sa,Do
Condicion_Variable integer DEFAULT 0,       -- Condiciona el evento
Condicion_Igualdad integer DEFAULT 0,       -- ==, >, <
Condicion_Valor integer DEFAULT 0,          -- Valor de condicion
Estado integer DEFAULT 0,                   -- 0= Salida apagada 1= Salida encendida
Habilitado integer DEFAULT 1,               -- 0=Apagado 1=Automatico 2=Encendido 
Icono_Apagado varchar(32),
Icono_Encendido varchar(32),
Icono_Auto varchar(32),
Grupo_Visual integer DEFAULT 0,             -- 0=Ninguno 1=Alarma 2=Iluminación 3=Puertas 4=Climatización 5=Cámaras 6=Riego
Planta integer DEFAULT 0,
Cord_x integer DEFAULT 0,
Cord_y integer DEFAULT 0,
Actualizar integer DEFAULT 0,
Flags integer DEFAULT 0,
FOREIGN KEY(Objeto_Salida) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Grupo_Salida) REFERENCES TB_DOM_GROUP(Id),
FOREIGN KEY(Particion_Salida) REFERENCES TB_DOM_ALARM_PARTICION(Id),
FOREIGN KEY(Variable_Salida) REFERENCES TB_DOM_FLAG(Id),
FOREIGN KEY(Objeto_Sensor) REFERENCES TB_DOM_ASSIGN(Id),
FOREIGN KEY(Grupo_Visual) REFERENCES TB_DOM_GRUPO_VISUAL(Id),
UNIQUE INDEX idx_auto_id (Id)
);

CREATE TABLE IF NOT EXISTS TB_DOM_TOUCH (
Dispositivo integer,
Screen integer,
Line integer,
Button integer,
Evento integer DEFAULT 0,
Objeto integer DEFAULT 0,
X integer DEFAULT 0,
Y integer DEFAULT 0,
W integer DEFAULT 0,
H integer DEFAULT 0,
Redondo integer DEFAULT 0,
texto varchar(16),
icono varchar(16),
color_borde integer DEFAULT 0,
color_fondo integer DEFAULT 0,
color_texto integer DEFAULT 0,
orientacion integer DEFAULT 0,
UNIQUE INDEX idx_touch_id (Dispositivo,Screen,Line,Button),
FOREIGN KEY(Dispositivo) REFERENCES TB_DOM_PERIF(Id),
FOREIGN KEY(Objeto) REFERENCES TB_DOM_ASSIGN(Id)
);

